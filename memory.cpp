#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"

using namespace mem;
using std::string;
using std::istream;

typedef unsigned long number;

struct parse_error : public std::exception {
    parse_error(std::string s) {}
};
struct type_error : public std::exception {
    type_error(std::string s) {}
};

class var {
public:
    enum vartype {
       tnull,
       tmem,
       trange,
       tnumber,
       tstring,
    };

    var() : t_(tnull) {} // for containers
    var(memory m) : t_(tmem), m_(m) {}
    var(range r) : t_(trange), r_(r) {}
    var(number r) : t_(tnumber), n_(r) {}
    var(string s) : t_(tstring), s_(s) {}

    vartype type() const { return t_; }

    memory& getmemory() {
        check(tmem);
        return m_;
    }
    range& getrange() {
        check(trange);
        return r_;
    }
    number& getnumber() {
        check(tnumber);
        return n_;
    }
    string& getstring() {
        check(tstring);
        return s_;
    }
private:
    vartype  t_;

    memory   m_;
    range    r_;
    number   n_;
    string   s_;

    void check(vartype t) {
        if (t != t_) {
            throw type_error("type error");
        }
    }

public:
    void print(std::ostream& os) const {
       os << "blah";
    }
};

std::ostream& operator<<(std::ostream& os, var const& v) {
    v.print(os);
    return os;
}

struct parser {
    enum type {
        nonchar = 65536,
        read,
        write,
        crc16,
        str,
        eof,
        num,
        dotdot,
        name,
        print,
    };

    parser(memory& m) : m_(m) {}

    void parse(std::istream& is) {
        line_ = 1;
        while (lex(is) != eof) {
            parsestmt(is);
        }
    }
private:
    void parsestmt(istream& is) {
        trace("stmt");
        if (t_ == name) {
            std::string vname = s_;
            expect(type('='), is, "equals");
            var v = parseexpr(is);
            symtab[vname] = v;
        }
        else if (t_ == print) {
            std::cout << parseexpr(is) << std::endl;
        }
        else {
            parseexpr(is);
        }
        expect(type(';'), is, "semi");
    }
    var parseexpr(std::istream& is) {
        trace("expr");
        if (lex(is) == read) {
             expect(str, is, "filename");
             std::string filename = s_;
             memory m;
             readmoto(filename, m); 
             return m;
        }
        else if (t_ == crc16) {
             var v = parseexpr(is);
             mem::memory& m = v.getmemory();
             return mem::crc16(m);
        }
        else if (t_ == name) {
             return symtab[s_];
        }
        else if (t_ == num) {
             return a_;
        }
        error("?");
        return var();
    }
    range parserange(std::istream& is) {
        trace("range");
        expect(type('['), is, "[");
        addr min = parseaddr(is);
        expect(dotdot, is, "dotdot");
        addr max = parseaddr(is);
        expect(type(']'), is, "]");
        return range(min, max);
    }
    addr parseaddr(std::istream& is) {
        trace("addr");
        expect(num, is, "number");
        return a_;
    }
    void
    expect(type et, std::istream& is, std::string s) {
        if (lex(is) != et) {
            error("expected:" + s);
        }
    }
    void
    error(std::string s) {
        throw parse_error("parse error: " + s);
    }
    void
    typeerror(std::string s) {
        throw type_error("type error: " + s);
    }
    
    type
    lex(std::istream& is) {
        t_ = xlex(is);
        std::cout << "tok: " << t_ << " " << s_ << std::endl;
        return t_;
    }
    type
    xlex(std::istream& is) {
        s_ = "";
        is >> std::noskipws;
        for (;;) {
           char ch;
           is >> ch;
            
           if (!is) break;
           if (ch == '\n' || ch == '\r') {
               line_++;
               continue;
           }
           if (std::isspace(ch))
               continue;
           if (ch == '"') {
               is >> ch;
               while (is && ch != '"') {
                  s_.push_back(ch);
                  is >> ch;
               }
               if (!is) return eof;
               return str;
           }

           switch (ch) {
           case ';':
           case '=':
           case '[': case ']':
               return type(ch);
           }
           if (ch == '.') {
               is >> ch;
               if (ch == '.') return dotdot;
               is.putback(ch);
               return type('.');
           }

           if (std::isalnum(ch)) {
               do {
                   s_.push_back(ch);
                   is >> ch;
               } while (isalnum(ch));
               is.putback(ch);
               
               if (s_ == "read") return read;
               if (s_ == "crc") return crc16;
               if (s_ == "print") return print;
               char *ep;
               a_ = std::strtoul(s_.c_str(), &ep, 0);
               if (*ep == 0) {
                   return num;
               }
               return name;
               error("don't understand \"" + s_ + "\"");
           }
           error("unexpected");
        }
        return eof;
    }
    std::string s_;
    addr a_;
    type t_;
    int line_;
    memory& m_;

    std::map<std::string, var> symtab;

    struct trace {
        trace(string s) : s_(s) {
            std::cout << "enter: " << s_ << std::endl;
        }
        ~trace() {
            std::cout << "exit: " << s_ << std::endl;
        }
        string s_;
    };
};

int
main(int argc, char **argv)
{
    memory mem;
    parser p(mem);

    try {
        if (argc == 1) {
            p.parse(std::cin);
        }
        else for (int i = 1; i < argc; i++) {
            std::ifstream is(argv[i]);
            p.parse(is);
        }
    }
    catch (parse_error) {
        std::cerr << "could not parse file" << std::endl;
    }
    catch (type_error) {
        std::cerr << "type error" << std::endl;
    }
}
