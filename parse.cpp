#include <sstream>
#include "parse.h"
#include "var.h"
#include "fn.h"
#include "trace.h"

using std::vector;
using std::pair;
using tracer::trace;

namespace memory {
    vector<var>
    reify(vector<xfn> const &args) {
       vector<var> vv(args.size());
       std::transform(args.begin(), args.end(),
                      vv.begin(),
                      port::mem_fn(&xfn::operator())); 
       return vv;
    }

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
        opfn op_;
        xfn a1_;
        xfn a2_;
    public:
        binopcall(opfn op, xfn a1, xfn a2) : op_(op), a1_(a1), a2_(a2) {}
        var::var operator()() { 
            return op_(a1_(), a2_());
        }
    };

    class fncall {
        xfn fn_;
        vector<xfn> args_;
    public:
        fncall(xfn fn, vector<xfn> const& args = vector<xfn>()) : fn_(fn), args_(args) {}
        var operator()() {
            return fn_().getfunction()(reify(args_));
        }
    };

    class compstmt {
        vector<xfn> stmts_;
    public:
        compstmt(vector<xfn> const& stmts): stmts_(stmts_) {}
        var operator()() {
            var v;
            for (vector<xfn>::const_iterator i = stmts_.begin(); i != stmts_.end(); i++)
                v = (*i)();
            return v;
        }
    };

    class ifexpr {
        xfn e_;
        xfn s1_;
        xfn s2_;
    public:
        ifexpr(xfn e, xfn s1, xfn s2 = constantly(0)) : e_(e), s1_(s1), s2_(s2) {}
        var operator()() {
            bool b = e_();
            std::cout << "ifexpr: "<< b << std::endl;
            return b ? s1_() : s2_();
        }
    };

    class delay {
        xfn fn_;
    public:
        delay(xfn fn) : fn_(fn) {}
        // its an xfn.
        var operator()() {
            return var(*this);
        }
        // and a var
        var operator()(vector<var> const& args) {
             return fn_();
        }
    };

    class assign {
        binder lhs_;
        xfn rhs_;
    public:
        assign(binder lhs, xfn rhs) : lhs_(lhs), rhs_(rhs) {}
        var operator()() {
            var& v = lhs_();
            return v = rhs_();
        }
    };

    parser::parser() {
        syms["read"] = var(readfn);
        syms["print"] = var(printfn);
        syms["write"] = var(writefn);
        syms["crc16"] = var(crc16fn);
        syms["range"] = var(rangefn);
        syms["offset"] = var(offsetfn);
        syms["join"] = var(joinfn);

        ops['+'] = op(20, add);
        ops['*'] = op(40, mul);
        ops[lexer::dotdot] = op (50, mkrange);
    }

    void parser::parse(std::istream& is) {
        lexer lex(is);
        toks_ = tokstream(lex);
        parsefile();
    }
    void parser::parse(std::string const& s) {
        std::istringstream is(s);
        parse(is);
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
        if (toks_[0].type() == lexer::iftok)
             return parseifexpr();
        if (toks_[0].type() == '{') {
            toks_.consume();
            vector<xfn> ss;
            while (toks_[0].type() != '}') {
               ss.push_back(parsestmt());
            }
            toks_.consume();
            return compstmt(ss);
        }
        xfn e = parseexpr();
        match(';');
        return e;
    }

    //   expr
    // | fn (args*) expr
    xfn
    parser::parseexpr() {
        trace t1("expr", toks_);
        vector<xfn> args;
        bool b = false;
        if (toks_[0].type() == lexer::fn) {
            toks_.consume();
            args = parsearglist();
            b = true;
        }
        xfn v = parseprimaryexpr();
        xfn e = parsebinoprhs(0, v);
        if (b) {
            // not sure what to do with the args here.
            return delay(e);
        }
    
        return e;
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
    parser::parseifexpr() {
        trace t1("ifexpr", toks_);
        toks_.consume();
        xfn e;
        if (toks_[0].type() == '(') {
            toks_.consume();
            e = parseexpr();
            match(')');
        }
        xfn s = parsestmt();

        return ifexpr(e, s);
    }

    xfn
    parser::parsenameexpr() {
        trace t1("nameexpr", toks_);
        var &v = syms[toks_[0].str()];
        toks_.consume();
        if (toks_[0].type() == '(') {
            return fncall(constantly(v), parsearglist());
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

    vector<xfn>
    parser::parsearglist() {
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
        return args;
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
