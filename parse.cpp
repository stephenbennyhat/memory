#include "parse.h"
#include "var.h"
#include "fn.h"
#include "trace.h"

using std::vector;
using std::pair;
using tracer::trace;

namespace memory {

    class constantly
    {
        var t_;
    public:
        constantly(var t) : t_(t) {}
        var const& operator()() {
            return t_;
        };
    };

    class binder
    {
        var& t_;
    public:
        binder(var& t) : t_(t) {}
        var& operator()() {
            return t_;
        };
    };

    class binopcall {
        parser::opfn op_;
        xfn a1_;
        xfn a2_;
    public:
        binopcall(parser::opfn op, xfn a1, xfn a2) : op_(op), a1_(a1), a2_(a2) {}
        var::var operator()() { 
            return op_(a1_(), a2_());
        }
    };

    class fncall {
        typedef vector<xfn> xfnv;
        xfn fn_;
        xfnv args_;
    public:
        fncall(xfn fn, xfnv args) : fn_(fn), args_(args) {}
        var operator()() {
            vector<var> vv;
            for (xfnv::const_iterator i = args_.begin(); i != args_.end(); i++) {
                vv.push_back((*i)());
            }
            return (fn_().getfunction())(vv);
        }
    };

    struct assign {
        binder lhs_;
        xfn rhs_;
        assign(binder lhs, xfn rhs) : lhs_(lhs), rhs_(rhs) {}
        var operator()() {
            var& v = lhs_();
            return v = rhs_();
        }
    };

    parser::parser(std::istream& is) : lex_(is), toks_(lex_) {
        syms["read"] = var(readfn);
        syms["print"] = var(printfn);
        syms["write"] = var(writefn);
        syms["crc16"] = var(crc16fn);
        syms["range"] = var(rangefn);
        syms["offset"] = var(offsetfn);

        ops['+'] = op(20, add);
        ops['*'] = op(40, mul);
        ops[lexer::dotdot] = op (50, mkrange);
    }

    void
    parser::printsymtab(std::ostream& os) const {
       symtab::const_iterator end = syms.end();
       os << "nsyms: " << syms.size() << std::endl;
       for (symtab::const_iterator i = syms.begin(); i != end; ++i) {
           os << " syms[" << i->first << "] = " << i->second << std::endl;
       }
    }

    void
    parser::printops(std::ostream& os) const {
        for (optab::const_iterator o = ops.begin(); o != ops.end(); o++) {
            std::cout << o->first
                      << "=[" << o->second.first << "," << o->second.second << "]"
                      << std::endl;
        }
    }

    void
    parser::match(int t) {
        if (toks_[0].type() != t) parse_error("unexpected", toks_[0]);
        toks_.consume();
    }

    void
    parser::parsefile() {
        while (toks_[0].type() != tokstream::eof) {
            xfn v = parsestmt();
            syms["_"] = v();
        }
    }

    xfn
    parser::parsestmt() {
        trace t1("stmt", toks_);
        xfn v = parseexpr();
        match(';');
        return v;
    }

    xfn
    parser::parseexpr() {
        trace t1("expr", toks_);
        xfn v = parseprimaryexpr();

        return parsebinoprhs(0, v);
    }

    // this mechanism pinched from the llvm tutorial
    // http://llvm.org/docs/tutorial/LangImpl2.html
    xfn
    parser::parsebinoprhs(int exprprec, xfn lhs) {
        trace t1("binop", toks_);
        for (;;) {
            trace t1("binoploop", toks_);
            int type = toks_[0].type();
            if (ops.count(type) == 0 || ops[type].first < exprprec)
                return lhs;
            toks_.consume();
            xfn rhs = parseprimaryexpr();
            int ntype = toks_[0].type();
            if (ops.count(ntype) == 0 || ops[type].first < ops[ntype].first)
                rhs = parsebinoprhs(ops[type].first+1, rhs);
            opfn fn(ops[type].second);
            lhs = binopcall(fn, lhs, rhs);
        }
        return lhs;
    }

    xfn
    parser::parseprimaryexpr() {
        trace t1("prim", toks_);
        if (toks_[0].type() == lexer::num)
             return parsenumberexpr();
        if (toks_[0].type() == lexer::str)
             return parsestringexpr();
        if (toks_[0].type() == lexer::name)
             return parsenameexpr();
        if (toks_[0].type() == '(') 
            return parseparenexpr();
        parseerror("invalid primary expression");
        return constantly(0);
    }

    xfn
    parser::parsenameexpr() {
        trace t1("nameexpr", toks_);
        var &v = syms[toks_[0].str()];
        toks_.consume();
        if (toks_[0].type() == '(') {
            toks_.consume();
            vector<xfn> args;
            if (toks_[0].type() != ')')
            {
               for (;;) {
                   xfn e = parseexpr();
                   args.push_back(e);
                   if (toks_[0].type() == ')')
                       break;
                   if (toks_[0].type() != ',')
                       parseerror("no , in arglist");
                   toks_.consume();
               }
            }
            match(')');
            return fncall(constantly(v), args);
        }
        if (toks_[0].type() == '[') {
            return parseindexexpr(constantly(v));
        }
        if (toks_[0].type() == '=') {
            toks_.consume();
            return assign(binder(v), parseexpr());
        }
        return constantly(v);
    }

    xfn
    parser::parseparenexpr()
    {
        toks_.consume();
        xfn v = parseexpr();
        if (toks_[0].type() != ')')
            parse_error("unterminated ( expr");
        toks_.consume();
        return v;
    }

    xfn
    parser::parseindexexpr(xfn v) {
        trace t1("range", toks_);
        match('[');
        xfn r = parseexpr();
        match(']');
        return binopcall(index, v, r);
    }

    xfn
    parser::parsestringexpr() {
        toks_.consume();
        return constantly(toks_[-1].str());
    }

    xfn
    parser::parsenumberexpr() {
        toks_.consume();
        return constantly(readnumber(toks_[-1].str()));
    }

    void
    parser::parseerror(std::string s) {
        throw parse_error("parse error: " + s);
    }
}
