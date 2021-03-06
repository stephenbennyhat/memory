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

    vector<pv>
    reify(vector<xfn> const& args, pe e) {
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
        pv const& operator()(pe e) { return v_; };
    };

    class binding {
        string s_;
    public:
        binding(string const& s) : s_(s) {}
        pv& operator()(pe e) { return (*e)[s_]; }
    };

    class primopcall {
        primopfn op_;
        xfn a_;
    public:
        primopcall(primopfn op, xfn a) : op_(op), a_(a) {}
        pv operator()(pe e) {
            return pv(new var(op_(a_(e))));
        }
    };

    class binopcall {
        binopfn op_;
        xfn a1_;
        xfn a2_;
    public:
        binopcall(binopfn op, xfn a1, xfn a2) : op_(op), a1_(a1), a2_(a2) {}
        pv operator()(pe e) { 
            return pv(new var(op_(a1_(e), a2_(e))));
        }
    };

    class compstmt {
        vector<xfn> stmts_;
    public:
        compstmt(vector<xfn> const& stmts): stmts_(stmts) {}
        pv operator()(pe e) {
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
        ifexpr(xfn e, xfn s1, xfn s2 = constantly(var())) : e_(e), s1_(s1), s2_(s2) {}
        pv operator()(pe e) {
            return *e_(e) ? s1_(e) : s2_(e);
        }
    };

    class whileexpr {
        xfn e_;
        xfn s_;
    public:
        whileexpr(xfn e, xfn s) : e_(e), s_(s) {}
        pv operator()(pe e) {
            pv v;
            while (*e_(e)) {
               v = s_(e);
            }
            return v;
        }
    };

   class fncall {
        xfn fn_;
        vector<xfn> args_;
    public:
        fncall(xfn fn, vector<xfn> const& args) : fn_(fn), args_(args) {}
        pv operator()(pe e) {
            return pv(new var(fn_(e)->getfunction()(reify(args_, e), e)));
        }
    };

    class closure {
        xfn fn_;
        vector<string> args_;
        pe e_;
    public:
        closure(xfn fn, vector<string>const& args) : fn_(fn), args_(args) {}
        pv operator()(pe const& e) {
            e_ = e;
            return pv(new var(*this));
        }
        var operator()(vector<pv> const& v, pe const& e) {
            pe ee(new env);
            ee->setprev(e_); // extend the captured environment with the params.
            for (size_t i = 0; i < args_.size(); i++) {
                (*ee)[args_.at(i)] = v.at(i);
            }
            return *fn_(ee);
        }
    };

    class assign {
        lfn lhs_;
        xfn rhs_;
    public:
        assign(lfn lhs, xfn rhs) : lhs_(lhs), rhs_(rhs) {}
        pv operator()(pe e) {
            pv& l = lhs_(e);
            *l = *rhs_(e);
            return l;
        }
    };

    parser::parser() {
        syms_ = pe(new env);

        (*syms_)["read"] = pv(new var(readfn));
        (*syms_)["print"] = pv(new var(printfn));
        (*syms_)["write"] = pv(new var(writefn));
        (*syms_)["crc16"] = pv(new var(crc16fn));
        (*syms_)["range"] = pv(new var(rangefn));
        (*syms_)["offset"] = pv(new var(offsetfn));
        (*syms_)["join"] = pv(new var(joinfn));

        (*syms_)["ofs"] = pv(new var(string("")));
        (*syms_)["ors"] = pv(new var(string("\n")));

        binops_[lexer::eqtok] = binop(5, eqop);
        binops_[lexer::netok] = binop(5, neop);
        binops_['<'] = binop(10, ltop);
        binops_['>'] = binop(10, gtop);
        binops_['+'] = binop(20, add);
        binops_['-'] = binop(20, sub);
        binops_['*'] = binop(40, mul);
        binops_[lexer::dotdot] = binop(50, mkrange);

        primops_['!'] = primop(notop);
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
        for (binoptab::const_iterator o = binops_.begin(); o != binops_.end(); o++) {
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
            pv v(parsestmt()(syms_));
            (*syms_)["_"] = v;
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
            while (toks_[0].type() != '}')
               ss.push_back(parsestmt());
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
        vector<string> args;
        match('(');
        for (int index = 0;; index++) {
            if (toks_[0].type() == ')')
                break;
            if (toks_[0].type() != lexer::name)
                parseerror("invalid argument list");
            string s = toks_[0].str();
            args.push_back(s);
            toks_.consume();
            if (toks_[0].type() == ',') {
                toks_.consume();
                continue;
            }
        }
        toks_.consume();
        return closure(parseexpr(), args);
    }

    // this mechanism pinched from the llvm tutorial
    // http://llvm.org/docs/tutorial/LangImpl2.html
    xfn
    parser::parsebinoprhs(int exprprec, xfn lhs) {
        trace t1("binop", toks_);
        for (;;) {
            trace t1("binoploop", toks_);
            int type = toks_[0].type();
            if (binops_.count(type) == 0 || binops_[type].first < exprprec)
                return lhs;
            toks_.consume();
            xfn rhs = parseprimaryexpr();
            int ntype = toks_[0].type();
            if (binops_.count(ntype) == 0 || binops_[type].first < binops_[ntype].first)
                rhs = parsebinoprhs(binops_[type].first+1, rhs);
            binopfn fn(binops_[type].second);
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

        primoptab::const_iterator it = primops_.find(toks_[0].type());
        if (it != primops_.end()) {
            toks_.consume();
            return primopcall(it->second, parseprimaryexpr());
        }

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
        string s = toks_[0].str();
        toks_.consume();
        if (toks_[0].type() == '(')
            return fncall(binding(s), parsearglist());
        if (toks_[0].type() == '[')
            return parseindexexpr(binding(s));
        if (toks_[0].type() == '=') {
            trace t2("assign", toks_);
            toks_.consume();
            return assign(binding(s), parseexpr());
        }
        return binding(s);
    }

    vector<xfn>
    parser::parsearglist() {
        toks_.consume();
        vector<xfn> args;
        if (toks_[0].type() != ')') {
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
    parser::parseparenexpr() {
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
