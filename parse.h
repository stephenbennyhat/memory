#ifndef PARSE_H
#define PARSE_H

#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "port.h"
#include "mem.h"
#include "lex.h"
#include "var.h"

namespace memory {

    typedef port::function<var (pv const&, pv const&)> binopfn;
    typedef port::function<var (pv const&)> primopfn;

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

        parser();

        void parse(std::istream& os);
        void parse(std::string const& s);
    private:
        typedef std::pair<int, binopfn> binop; // prec, fn
        typedef std::map<int, binop> binoptab;

        typedef primopfn primop;
        typedef std::map<int, primop> primoptab;

        binoptab binops_;
        primoptab primops_;

        tokstream toks_;

        pe syms_;

        void printops(std::ostream& os) const;

        void match(int t);

        void checktype(vartype t1, vartype t2) const;
        void parseerror(std::string s);

        void parsefile();
        xfn parsestmt();
        xfn parseexpr();
        xfn parseprimaryexpr();
        xfn parseparenexpr();
        xfn parseindexexpr(xfn);
        xfn parsenameexpr();
        xfn parseifexpr();
        xfn parsewhileexpr();
        xfn parsestringexpr();
        xfn parsenumberexpr();
        xfn parsebinoprhs(int, xfn);
        xfn parsefn();
        std::vector<xfn> parsearglist();
    };
}
#endif
