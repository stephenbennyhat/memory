#ifndef PARSE_H
#define PARSE_H

#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"
#include "lex.h"
#include "var.h"

namespace memory {

class parser {
public:
    struct parse_error : public std::exception {
        parse_error(std::string s) : s_(s) {}
        ~parse_error() throw () {}
        std::string s_;

        friend std::ostream& operator<<(std::ostream& os, parse_error const& pe) {
            return os << pe.s_;
        }
    };

    parser(std::istream& os);

    void parse() {
        parsefile();
    }
private:
    enum { debug = 0 };

    lexer lex_;

    typedef std::map<std::string, var::var> symtab;
    symtab syms;

    void printsymtab(std::ostream& os) const;

    void expect(int t);
    void eatuntil(int t);
    void consume();
    void match(int t);

    void checktype(var::var::vartype t1, var::var::vartype t2) const;

    void parsefile();
    void parsestmt();
    var::var parseexpr();
    mem::range parserange();
    number parsenumber();

    void parseerror(std::string s);


    struct trace {
        trace(std::string s, tokstream& ts) : s_(s), ts_(ts) {
            if (debug) std::cout << "enter: " << s_ << " " << ts_ << std::endl;
        }
        ~trace() {
            if (debug) std::cout << "exit: " << s_ << " " << ts_ << std::endl;
        }
        std::string s_;
        tokstream& ts_;
    };
};

} // namespace
#endif
