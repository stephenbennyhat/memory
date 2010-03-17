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
            virtual char const *what() const throw () { return s_.c_str(); }
        private:
            std::string s_;
            token t_;
        };

        parser(std::istream& os);
        void parse() { parsefile(); }

        typedef std::tr1::function<var (var const&, var const&)> opfn;
    private:
        static bool const interactive = 0;


        lexer lex_;
        tokstream toks_;

        symtab syms;

        typedef std::pair<int, opfn> op; // prec, fn
        typedef std::map<int, op> optab;
        optab ops;

        void printops(std::ostream& os) const;

        void match(int t);

        void checktype(var::vartype t1, var::vartype t2) const;

        void parsefile();
        xfn parsestmt();
        xfn parseexpr();
        xfn parseprimaryexpr();
        xfn parseparenexpr();
        xfn parseindexexpr(xfn);
        xfn parsenameexpr();
        xfn parsestringexpr();
        xfn parsenumberexpr();
        xfn parsebinoprhs(int, xfn);
   
        void parseerror(std::string s);
    };
}
#endif
