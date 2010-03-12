#include "parse.h"
#include "var.h"
#include "fn.h"

using std::vector;

parser::parser(std::istream& os) : lex_(os) {
    syms["read"] = readfn;
    syms["print"] = printfn;
    syms["write"] = printfn;
    syms["crc16"] = crc16fn;
}

void parser::printsymtab(std::ostream& os) const {
   symtab::const_iterator end = syms.end();

   os << "nsyms: " << syms.size() << std::endl;
   for (symtab::const_iterator i = syms.begin(); i != end; ++i) {
       os << " syms[" << i->first << "] = " << i->second << std::endl;
   }
}

void parser::expect(int t) {
    if (lex_[0].type() != t) parse_error("unexpected");
}

void parser::consume() {
    lex_.consume();
}

void parser::match(int t) {
    expect(t);
    consume();
}

void parser::parsefile() {
    while (lex_[0].type() != lexer::eoftok) {
        parsestmt();
    }
}

var
callfn(var v, vector<var> const& args)
{
    std::cout << "calling function args=[" << args.size() << ": ";
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i] << (i + 1 == args.size() ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    return (*v.getfunction())(args);
}

void parser::parsestmt() {
    trace t1("stmt", lex_);
    var n = parseexpr();
    if (n.type() != var::tnull) {
        std::cout << n << std::endl;
    }
    if (lex_[0].type() == lexer::eoftok) {
        return;
    }
    match(';');
    printsymtab(std::cout);
}

var parser::parseexpr() {
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
            std::cout << "installing " << s << "=" << v << std::endl;
         }
         else if (lex_[0].type() == '(') {
            trace t4("fn", lex_);
            consume();
            vector<var> args;

            if (lex_[0].type() == ')') {
               consume();
            }
            else {
                trace t5("args", lex_);
                for (;;) {
                    var e = parseexpr();
                    args.push_back(e);
                    std::cout << " added arg: " << e << std::endl;
                    if (lex_[0].type() == ',') {
                        consume();
                        continue;
                    }
                    else
                        break;
                }
                match(')');
            }

            return callfn(v, args);
         }
         return v;
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

mem::range parser::parserange() {
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
