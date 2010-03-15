#include "parse.h"
#include "var.h"
#include "fn.h"
#include "trace.h"

using std::vector;
using std::pair;
using tracer::trace;

namespace memory {

var add(var const& v1, var const& v2) {
    return v1.getnumber() + v2.getnumber(); //XXX
}

var mul(var const& v1, var const& v2) {
    return v1.getnumber() * v2.getnumber(); //XXX
}

var mkrange(var const& v1, var const& v2) {
    return mem::range(v1.getnumber(), v2.getnumber());
}

var index(var const& v1, var const& v2) {
    return crop(v1.getmemory(), v2.getrange());
}

parser::parser(std::istream& is) : lex_(is), toks_(lex_) {
    syms["read"] = var(readfn);
    syms["print"] = var(printfn);
    syms["write"] = var(writefn);
    syms["crc16"] = var(crc16fn);
    syms["range"] = var(rangefn);

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
        std::cout << o->first << "=[" << o->second.first << "," << o->second.second << "]" << std::endl;
    }
}

void
parser::match(int t) {
    if (toks_[0].type() != t) parse_error("unexpected", toks_[0]);
    toks_.consume();
}

var
parser::parsefile() {
    var v;
    while (toks_[0].type() != tokstream::eof) {
        v = parsestmt();
    }
    return v;
}

var
parser::parsestmt() {
    trace t1("stmt", toks_);
    var v = parseexpr();
    if (toks_[0].type() == tokstream::eof) {
        return var();
    }
    match(';');
    if (0) printsymtab(std::cout);
    return syms["_"] = v;
}

var
parser::parseexpr() {
    trace t1("expr", toks_);
    var v = parseprimaryexpr();

    return parsebinoprhs(0, v);
}

/*
 * this mechanism pinched from the llvm tutorial
 * http://llvm.org/docs/tutorial/LangImpl2.html
 */
var
parser::parsebinoprhs(int exprprec, var lhs) {
    trace t1("binop", toks_);
    for (;;) {
        trace t1("binoploop", toks_);
        int type = toks_[0].type();
        if (ops.count(type) == 0 || ops[type].first < exprprec)
            return lhs;
        toks_.consume();
        var rhs = parseprimaryexpr();
        int ntype = toks_[0].type();
        if (ops.count(ntype) == 0 || ops[type].first < ops[ntype].first)
            rhs = parsebinoprhs(ops[type].first+1, rhs);
        opfn fn(ops[type].second);
        lhs = fn(lhs, rhs);
    }
    return lhs;
}

var
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
    return var();
}

var
parser::parsenameexpr() {
    trace t1("nameexpr", toks_);
    var& v = syms[toks_[0].str()];
    toks_.consume();
    if (toks_[0].type() == '(') {
        toks_.consume();
        vector<var> args;
        if (toks_[0].type() != ')')
        {
           for (;;) {
               var e = parseexpr();
               args.push_back(e);
               if (toks_[0].type() == ')')
                   break;
               if (toks_[0].type() != ',')
                   parseerror("no , in arglist");
               toks_.consume();
           }
        }
        match(')');
        return v.getfunction()(args);
    }
    if (toks_[0].type() == '[') {
        return parseindexexpr(v);
    }
    if (toks_[0].type() == '=') {
        toks_.consume();
        v = parseexpr();
    }
    return v;
}

var
parser::parseparenexpr()
{
    toks_.consume();
    var v = parseexpr();
    if (toks_[0].type() != ')')
        parse_error("unterminated ( expr");
    toks_.consume();
    return v;
}

var
parser::parseindexexpr(var const& v) {
    trace t1("range", toks_);
    match('[');
    var r = parseexpr();
    match(']');
    return index(v, r);
}

var
parser::parsestringexpr() {
    toks_.consume();
    return toks_[-1].str();
}

var
parser::parsenumberexpr() {
    toks_.consume();
    return readnumber(toks_[-1].str());
}

void
parser::parseerror(std::string s) {
    throw parse_error("parse error: " + s);
}

} // namespace
