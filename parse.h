#ifndef PARSE_H
#define PARSE_H

#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"
#include "lex.h"
#include "var.h"

class parser {
public:
    struct parse_error : public std::exception {
        parse_error(std::string s) {}
    };

    struct type_error : public std::exception {
        type_error(std::string s) {}
    };

    parser(std::istream& os) : lex_(os) {}

    void parse() {
        parsefile();
    }
private:
    enum { debug = 1 };
    lexer lex_;

    void expect(int t) {
        if (lex_[0].type() != t) parse_error("unexpected");
    }
    void consume() {
        lex_.consume();
    }
    void match(int t) {
        expect(t);
        consume();
    }
    void parsefile() {
        while (lex_[0].type() != lexer::eoftok) {
            parsestmt();
        }
    }
    void parsestmt() {
        trace t1("stmt", lex_);
        if (lex_[0].type() == lexer::name) {
            trace t2("assign", lex_);
            std::string vname = lex_[0].str();
            consume();
            match('=');
            symtab[vname] = parseexpr();
        }
        else if (lex_[0].type() == lexer::write) {
            trace t3("write", lex_);
            consume();
            var v = parseexpr();
            writemoto(std::cout, v.getmemory());
        }
        else if (lex_[0].type() == lexer::print) {
            trace t3("print", lex_);
            consume();
            std::cout << parseexpr() << std::endl;
        }
        else {
            parseexpr();
        }
        match(';');
    }
    var parseexpr() {
        trace t1("expr", lex_);
        if (lex_[0].type() == lexer::read) {
             trace t2("read", lex_);
             consume();
             expect(lexer::str);
             std::string filename = lex_[0].str();
             consume();
             mem::memory m;
             readmoto(filename, m); 
             std::cout << "returning " << m << std::endl;
             return m;
        }
        else if (lex_[0].type() == lexer::crc16) {
             trace t2("crc", lex_);
             consume();
             var v = parseexpr();
             mem::memory& m = v.getmemory();
             return mem::crc16(m);
        }
        else if (lex_[0].type() == lexer::name) {
             trace t3("name", lex_);
             consume();
             std::string s = lex_[0].str();
             consume();
             return symtab[s];
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
    mem::range parserange() {
        trace t1("range", lex_);
        match('[');
        mem::addr min = parsenumber();
        match(lexer::dotdot);
        mem::addr max = parsenumber();
        match(']');
        return mem::range(min, max);
    }
    number
    parsenumber() {
        expect(lexer::num);
        number n = readnumber(lex_[0].str());
        consume();
        return n;
    }
    void
    parseerror(std::string s) {
        throw parse_error("parse error: " + s);
    }
    void
    typeerror(std::string s) {
        throw type_error("type error: " + s);
    }

    std::map<std::string, var> symtab;

    struct trace {
        trace(std::string s, tokstream& ts) : s_(s), ts_(ts) {
            if (debug) std::cout << "enter: " << s_ << " " << ts_ << std::endl;
        }
        ~trace() {
            if (debug) std::cout << "exit: " << s_ << " " << ts_ << std::endl;
        }
        std::string s_;
        tokstream& ts_;
    };
};
#endif
