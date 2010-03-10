#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"

struct parser {
    enum type {
        nonchar = 65536,
        read,
        crc,
        string,
        eof,
        number,
        dotdot,
        name,
    };

    enum symtype {
       mem,
       range,
       num,
    };

    struct symbol {
        symtype t;

        mem::memory   m;
        mem::range    r;
        unsigned long n;
    };

    struct symtab {
        std::map<std::string, symbol> tab;
    };



    struct parse_error : public std::exception {};

    parser(mem::memory& m) : m_(m) {}

    void parse(std::istream& is) {
        while (lex(is) != eof) {
             if (t_ == read) {
                 expect(string, is, "filename");
                 std::string filename = s_;
                 expect(type(';'), is, "semi");
                 readmoto(filename, m_);
             }
             else if (t_ == crc) {
                 mem::range r = parserange(is);
                 expect(type(';'), is, "semi");
             }
             else if (t_ == name) {
                 
             }
             else {
                 error("?");
             }
        }
    }
    mem::range parserange(std::istream& is) {
        mem::addr min = parseaddr(is);
        expect(dotdot, is, "dotdot");
        mem::addr max = parseaddr(is);
        return mem::range(min, max);
    }
    mem::addr parseaddr(std::istream& is) {
        expect(number, is, "number");
        return a_;
    }
    
private:
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
               return string;
           }

           if (ch == ';') return type(';');
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
    mem::addr a_;
    type t_;
    mem::memory& m_;
};

int
main(int argc, char **argv)
{
    mem::memory mem;
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
