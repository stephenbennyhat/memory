#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"

using namespace mem;
using std::string;

typedef unsigned long number;

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
            throw std::logic_error("bad symtype access");
        }
    }
};

struct parser {
    enum type {
        nonchar = 65536,
        read,
        write,
        crc,
        str,
        eof,
        number,
        dotdot,
        name,
    };

    struct parse_error : public std::exception {};

    parser(memory& m) : m_(m) {}

    void parse(std::istream& is) {
        while (lex(is) != eof) {
             if (t_ == crc) {
             }
             else if (t_ == name) {
                 std::string vname = s_;
                 expect(type('='), is, "equals");
                 var v = parseexpr(is);
                 symtab[vname] = v;
             }
             else {
                 parseexpr(is);
             }
        }
    }
private:
    var parseexpr(std::istream& is) {
        if (lex(is) == read) {
             expect(str, is, "filename");
             std::string filename = s_;
             memory m;
             readmoto(filename, m);
             return var(m);
        }
        else if (t_ == crc) {
             range r = parserange(is);
             expect(type(';'), is, "semi");
        }
        return var();
    }
    range parserange(std::istream& is) {
        expect(type('['), is, "[");
        addr min = parseaddr(is);
        expect(dotdot, is, "dotdot");
        addr max = parseaddr(is);
        expect(type(']'), is, "]");
        return range(min, max);
    }
    addr parseaddr(std::istream& is) {
        expect(number, is, "number");
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
        std::cerr << "parse error: " << s << std::endl;
        throw parse_error();
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
           if (ch == '\n' || ch == '\r' || std::isspace(ch)) continue;
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
               if (s_ == "crc") return crc;
               char *ep;
               a_ = std::strtoul(s_.c_str(), &ep, 0);
               std::cout << "check number: s=" << s_ << std::endl;
               if (*ep == 0) {
                   return number;
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
    memory& m_;

    std::map<std::string, var> symtab;
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
    catch (parser::parse_error) {
        std::cerr << "could not parse file" << std::endl;
    }
//    writemoto(std::cout, mem);
}
