#include "parse.h"
#include "var.h"
#include "fn.h"

using std::vector;

namespace memory {

parser::parser(std::istream& os) : lex_(os) {
    syms["read"] = readfn;
    syms["print"] = printfn;
    syms["write"] = writefn;
    syms["crc16"] = crc16fn;
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
    if (lex_[0].type() != t) parse_error("unexpected");
}

void
parser::consume() {
    lex_.consume();
}

void
parser::match(int t) {
    expect(t);
    consume();
}

void
parser::eatuntil(int stop) {
    int t;
    do {
        t = lex_[0].type();
        if (debug) std::cout << "eating: " << lex_[0] << std::endl;
        consume();
    }
    while (stop != t && stop != lexer::eoftok);
}

void
parser::parsefile() {
    int errcnt = 0;
    while (lex_[0].type() != lexer::eoftok) {
        try {
            parsestmt();
        }
        catch (parse_error const& pe) {
           std::cout << pe << std::endl;
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
    return (*v.getfunction())(args);
}

void
parser::parsestmt() {
    trace t1("stmt", lex_);
    var n = parseexpr();
    if (n.type() != var::tnull) {
        std::cout << n << std::endl;
    }
    if (lex_[0].type() == lexer::eoftok) {
        return;
    }
    match(';');
    if (debug) printsymtab(std::cout);
}

var
parser::parseexpr() {
    trace t1("expr", lex_);
    if (lex_[0].type() == lexer::name) {
         trace t3("name", lex_);
         std::string s = lex_[0].str();
         consume();
         var& v = syms[s];
         if (lex_[0].type() == '=') {
             trace t2("assign", lex_);
             consume();
             v = parseexpr();
         }
         else if (lex_[0].type() == '(') {
             trace t4("fn", lex_);
             vector<var> args;
             if (lex_[1].type() == ')') {
                 consume();
                 consume();
             }
             else {
                 trace t5("args", lex_);
                 do {
                     consume();
                     var e = parseexpr();
                     args.push_back(e);
                 } while (lex_[0].type() == ',');
                 match(')');
             }
             return callfn(v, args);
         }
         else if (lex_[0].type() == '[') {
             var r = parseexpr();
             return crop(v.getmemory(), r.getrange());
         }

         return v;
    }
    else if (lex_[0].type() == '[') {
        mem::range r = parserange();
        return r;
    }
    else if (lex_[0].type() == lexer::num) {
         trace t3("num", lex_);
         number n = readnumber(lex_[0].str());
         consume();
         return n;
    }
    else if (lex_[0].type() == lexer::str) {
         trace t3("str", lex_);
         std::string s = lex_[0].str();
         consume();
         return s;
    }
    else {
         trace t4("unexpected", lex_);
         parseerror("unexpected");
    }
    return var();
}

mem::range
parser::parserange() {
    trace t1("range", lex_);
    match('[');
    mem::addr min = parsenumber();
    match(lexer::dotdot);
    mem::addr max = parsenumber();
    match(']');
    return mem::range(min, max);
}

number
parser::parsenumber() {
    expect(lexer::num);
    number n = readnumber(lex_[0].str());
    consume();
    return n;
}

void
parser::parseerror(std::string s) {
    throw parse_error("parse error: " + s);
}

} // namespace
