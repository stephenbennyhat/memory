#include "parse.h"
#include "var.h"
#include "fn.h"

using std::vector;

namespace memory {

parser::parser(std::istream& is) : lex_(is), toks_(lex_) {
    syms["read"] = var(readfn);
    syms["print"] = var(printfn);
    syms["write"] = var(writefn);
    syms["crc16"] = var(crc16fn);
    syms["range"] = var(rangefn);
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
parser::expect(int t) {
    if (toks_[0].type() != t) parse_error("unexpected", toks_[0]);
}

void
parser::match(int t) {
    expect(t);
    toks_.consume();
}

void
parser::eatuntil(int stop) {
    int t;
    do {
        t = toks_[0].type();
        if (debug) std::cout << "eating: " << toks_[0] << std::endl;
        toks_.consume();
    }
    while (stop != t && stop != lexer::eoftok);
}

void
parser::parsefile() {
    int errcnt = 0;
    while (toks_[0].type() != lexer::eoftok) {
        try {
            parsestmt();
        }
        catch (std::exception const& e) {
           std::cout << e.what() << std::endl;
           if (errcnt++ > 3) throw;
           std::cout << "continuing..." << std::endl;
           eatuntil(';');
        }
    }
}

var
callfn(var v, vector<var> const& args, bool debug = false) {
    if (debug) {
        std::cout << "calling function args=[" << args.size() << ": ";
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i] << (i + 1 == args.size() ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }
    return v.getfunction()(args);
}

void
parser::parsestmt() {
    trace t1("stmt", toks_);
    syms["_"] = parseexpr();
    if (toks_[0].type() == lexer::eoftok) {
        return;
    }
    match(';');
    if (debug) printsymtab(std::cout);
}

var
parser::parseexpr() {
    trace t1("expr", toks_);
    if (toks_[0].type() == lexer::name) {
         trace t3("name", toks_);
         std::string s = toks_[0].str();
         toks_.consume();
         var& v = syms[s];
         if (toks_[0].type() == '=') {
             trace t2("assign", toks_);
             toks_.consume();
             v = parseexpr();
         }
         else if (toks_[0].type() == '(') {
             trace t4("fn", toks_);
             vector<var> args;
             if (toks_[1].type() == ')') {
                 toks_.consume();
                 toks_.consume();
             }
             else {
                 trace t5("args", toks_);
                 do {
                     toks_.consume();
                     var e = parseexpr();
                     args.push_back(e);
                 } while (toks_[0].type() == ',');
                 match(')');
             }
             return callfn(v, args);
         }
         else if (toks_[0].type() == '[') {
             var r = parseexpr();
             return crop(v.getmemory(), r.getrange());
         }

         return v;
    }
    else if (toks_[0].type() == '[') {
        mem::range r = parserange();
        return r;
    }
    else if (toks_[0].type() == lexer::num) {
         trace t3("num", toks_);
         number n = readnumber(toks_[0].str());
         toks_.consume();
         return n;
    }
    else if (toks_[0].type() == lexer::str) {
         trace t3("str", toks_);
         std::string s = toks_[0].str();
         toks_.consume();
         return s;
    }
    else {
         trace t4("unexpected", toks_);
         parseerror("unexpected");
    }
    return var();
}

mem::range
parser::parserange() {
    trace t1("range", toks_);
    match('[');
    mem::addr min = parseexpr().getnumber();
    match(lexer::dotdot);
    mem::addr max = parseexpr().getnumber();
    match(']');
    return mem::range(min, max);
}

void
parser::parseerror(std::string s) {
    throw parse_error("parse error: " + s);
}

} // namespace
