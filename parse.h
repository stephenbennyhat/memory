#ifndef PARSE_H
#define PARSE_H

#include <stdexcept>
#include <map>
#include <tr1/functional>
#include <assert.h>
#include <ctype.h>
#include "mem.h"
#include "lex.h"
#include "var.h"

namespace memory {

class parser {
public:
    struct parse_error : public std::exception {
        parse_error(std::string const& s) : s_(s) {}
        parse_error(std::string const& s, token const& t) : s_(s), t_(t) {}
        ~parse_error() throw () {}
        std::string s_;
        token t_;

        virtual char const *what() throw () {
            return s_.c_str();
        }
    };

    parser(std::istream& os);

    void parse() {
        parsefile();
    }
private:
    static bool const interactive = 0;

    lexer lex_;
    tokstream toks_;

    typedef std::map<std::string, var::var> symtab;
    symtab syms;

    typedef std::tr1::function<var (var const&, var const&)> opfn;
    typedef std::pair<int, opfn> op; // prec, fn
    typedef std::map<int, op> optab;
    optab ops;

    void printsymtab(std::ostream& os) const;
    void printops(std::ostream& os) const;

    void expect(int t);
    void eatuntil(int t);
    void match(int t);

    void checktype(var::vartype t1, var::vartype t2) const;

    var::var parsefile();
    var::var parsestmt();
    var::var parseexpr();
    var::var parseprimaryexpr();
    var::var parseparenexpr();
    var::var parsenameexpr();
    var::var parserangeexpr();
    var::var parsestringexpr();
    var::var parsenumberexpr();
    var::var parsebinoprhs(int, var);

    void parseerror(std::string s);
};

} // namespace
#endif
