#include <sstream>
#include "parse.h"
#include "var.h"
#include "fn.h"
#include "trace.h"

namespace memory {
    using std::vector;
    using std::pair;
    using std::string;
    using tracer::trace;

    struct env {
        vector<pv> v_;
    };

    vector<pv>
    reify(vector<xfn> const &args, env e) {
       vector<pv> vv(args.size());
       for (size_t i = 0; i < args.size(); i++) {
          vv[i] = args[i](e);
       }
       return vv;
    }

    class constantly {
        pv v_;
    public:
        constantly(var const& t) : v_(new var(t)) {}
        pv const& operator()(env e) {
            return v_;
        };
    };

    class global {
        pv v_;
    public:
        global(symbol const& s) : v_(s.v_) {}
        pv operator()(env e) { return v_; }
    };

    class local {
        int i_;
    public:
        local(symbol const& s) : i_(s.index_) {}
        pv operator()(env e) {
            return e.v_[i_];
        }
    };

    xfn binder(symbol const& s) {
       if (s.global())
               return global(s);
       return local(s);
    }

    class binopcall {
        opfn op_;
        xfn a1_;
        xfn a2_;
    public:
        binopcall(opfn op, xfn a1, xfn a2) : op_(op), a1_(a1), a2_(a2) {}
        pv operator()(env e) { 
            return pv(new var(op_(a1_(e), a2_(e))));
        }
    };

    class fncall {
        xfn fn_;
        vector<xfn> args_;
    public:
        fncall(xfn fn, vector<xfn> const& args = vector<xfn>()) : fn_(fn), args_(args) {}
        pv operator()(env e) {
            return pv(new var(fn_(e)->getfunction()(reify(args_, e))));
        }
    };

    class compstmt {
        vector<xfn> stmts_;
    public:
        compstmt(vector<xfn> const& stmts): stmts_(stmts) {}
        pv operator()(env e) {
            pv v;
            for (vector<xfn>::const_iterator i = stmts_.begin(); i != stmts_.end(); i++)
                v = (*i)(e);
            return v;
        }
    };

    class ifexpr {
        xfn e_;
        xfn s1_;
        xfn s2_;
    public:
        ifexpr(xfn e, xfn s1, xfn s2 = constantly(0)) : e_(e), s1_(s1), s2_(s2) {}
        pv operator()(env e) {
            bool b = *e_(e);
            return b ? s1_(e) : s2_(e);
        }
    };

    class whileexpr {
        xfn e_;
        xfn s_;
    public:
        whileexpr(xfn e, xfn s) : e_(e), s_(s) {}
        pv operator()(env e) {
            pv v;
            while (*e_(e)) {
               v = s_(e);
            }
            return v;
        }
    };

    class closure {
        xfn fn_;
        env e_;
    public:
        closure(xfn fn) : fn_(fn) {}
        pv operator()(env e) {
            e_ = e; //XXX
            return pv(new var(*this));
        }
        var operator()(vector<pv> const& args) {
            e_.v_ = args; //XXX
            return *fn_(e_);
        }
    };

    class assign {
        xfn lhs_;
        xfn rhs_;
    public:
        assign(xfn lhs, xfn rhs) : lhs_(lhs), rhs_(rhs) {}
        pv operator()(env e) {
            pv l = lhs_(e);
            *l = *rhs_(e);
            return l;
        }
    };

    parser::parser() {
        syms_.insert("read", pv(new var(readfn)));
        syms_.insert("print", pv(new var(printfn)));
        syms_.insert("write", pv(new var(writefn)));
        syms_.insert("crc16", pv(new var(crc16fn)));
        syms_.insert("range", pv(new var(rangefn)));
        syms_.insert("offset", pv(new var(offsetfn)));
        syms_.insert("join", pv(new var(joinfn)));

        ops_[lexer::eqtok] = op(5, eqop);
        ops_[lexer::netok] = op(5, neop);
        ops_['<'] = op(10, ltop);
        ops_['>'] = op(10, gtop);
        ops_['+'] = op(20, add);
        ops_['-'] = op(20, sub);
        ops_['*'] = op(40, mul);
        ops_[lexer::dotdot] = op (50, mkrange);
    }

    void parser::parse(std::istream& is) {
        lexer lex(is);
        toks_ = tokstream(lex);
        parsefile();
    }

    void parser::parse(string const& s) {
        std::istringstream is(s);
        parse(is);
    }

    void
    parser::printops(std::ostream& os) const {
        for (optab::const_iterator o = ops_.begin(); o != ops_.end(); o++) {
            os  << o->first
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
            env e;
            pv v(parsestmt()(e));
            syms_.insert("_", v);
        }
    }

    xfn
    parser::parsestmt() {
        trace t1("stmt", toks_);
        if (toks_[0].type() == lexer::iftok)
             return parseifexpr();
        if (toks_[0].type() == lexer::whiletok)
             return parsewhileexpr();
        if (toks_[0].type() == '{') {
            trace t1("cmptstmt", toks_);
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

    xfn
    parser::parseexpr() {
        trace t1("expr", toks_);
        vector<xfn> args;
        if (toks_[0].type() == '{')
            return parsestmt();
        if (toks_[0].type() == lexer::fn)
            return parsefn();
        xfn v = parseprimaryexpr();
        return parsebinoprhs(0, v);
    }

    xfn
    parser::parsefn() {
        trace t1("fn", toks_);
        toks_.consume();
        syms_.push();
        match('(');
        for (int index = 0;; index++) {
            if (toks_[0].type() == ')')
                break;
            if (toks_[0].type() != lexer::name)
                parseerror("invalid argument list");
            string s = toks_[0].str();
            syms_.insert(s, pv(), index);
            toks_.consume();
            if (toks_[0].type() == ',') {
                toks_.consume();
                continue;
            }
        }
        if (0) std::cout << syms_ << std::endl;
        toks_.consume();
        xfn e = parseexpr();
        syms_.pop();
        return closure(e);
    }

    // this mechanism pinched from the llvm tutorial
    // http://llvm.org/docs/tutorial/LangImpl2.html
    xfn
    parser::parsebinoprhs(int exprprec, xfn lhs) {
        trace t1("binop", toks_);
        for (;;) {
            trace t1("binoploop", toks_);
            int type = toks_[0].type();
            if (ops_.count(type) == 0 || ops_[type].first < exprprec)
                return lhs;
            toks_.consume();
            xfn rhs = parseprimaryexpr();
            int ntype = toks_[0].type();
            if (ops_.count(ntype) == 0 || ops_[type].first < ops_[ntype].first)
                rhs = parsebinoprhs(ops_[type].first+1, rhs);
            opfn fn(ops_[type].second);
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
        match('(');
        xfn e = parseexpr();
        match(')');
        xfn s = parsestmt();
        if (toks_[0].type() != lexer::elsetok)
           return ifexpr(e, s);
        toks_.consume();
        return ifexpr(e, s, parsestmt());
    }

    xfn
    parser::parsewhileexpr() {
        trace t1("whileexpr", toks_);
        toks_.consume();
        match('(');
        xfn e = parseexpr();
        match(')');
        xfn s = parsestmt();
        return whileexpr(e, s);
    }

    xfn
    parser::parsenameexpr() {
        trace t1("nameexpr", toks_);
        symbol &v = syms_[toks_[0].str()];
        toks_.consume();
        if (toks_[0].type() == '(') {
            return fncall(binder(v), parsearglist());
        }
        if (toks_[0].type() == '[') {
            return parseindexexpr(binder(v));
        }
        if (toks_[0].type() == '=') {
            trace t2("assign", toks_);
            toks_.consume();
            return assign(binder(v), parseexpr());
        }
        return binder(v);
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
    parser::parseerror(string s) {
        throw parse_error("parse error: " + s);
    }
}
