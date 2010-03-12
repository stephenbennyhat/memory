#ifndef LEX_H
#define LEX_H

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <deque>
#include <assert.h>
#include <ctype.h>
#include "mem.h"

namespace memory {

typedef unsigned long number;

struct coord {
    int line;
    int chr;
    coord() : line(1), chr(0) {}
};

struct token {
    int type() const { return type_; }
    std::string str() const { return s_; }
    coord pos() const { return pos_; }
    std::string desc() const { return desc_; }

    token(int type, std::string s, coord pos, std::string desc = "")
        : type_(type), s_(s), pos_(pos), desc_(desc) {}
    token(int type, std::string s, coord pos, char ch)
        : type_(type), s_(s), pos_(pos), desc_("ch ") { 
        desc_.push_back('\'');
        desc_.push_back(ch);
        desc_.push_back('\'');
    }
    token() : desc_("null") {}
private:
    int type_;
    std::string s_;
    coord pos_;
    std::string desc_;
};

std::ostream& operator<<(std::ostream& os, token const& tok);

class tokstream {
public:
    tokstream(std::istream& is) : is_(is) {
        token t;
        toks.push_back(t);
    }
    virtual ~tokstream();
    token operator[](int);
    void consume();
    void print(std::ostream& os) const;
protected:
    bool eof() { return !is_; }
    void putback() { is_.putback(ch_); }
    int getchar();
    coord pos() { return pos_; }
private:
    enum { debug = 0 } ;
    void load() {
        token t = fetchnext();
        toks.push_back(t);
        if (debug) std::cout << t << std::endl;
    }

    virtual token fetchnext() = 0;

    std::istream& is_;
    coord pos_;
    std::deque<token> toks;
    char ch_;
};

inline
std::ostream& operator<<(std::ostream& os, tokstream& ts)
{
    ts.print(os);
    return os;
}

struct lexer : public tokstream {
    struct lexer_error : public std::exception {};

    enum toktype {
        eoftok = 65536,
        str,
        num,
        dotdot,
        name,
        null = 0,
    };
    explicit lexer(std::istream& is) : tokstream(is) {}
    virtual ~lexer() {}
private:
    virtual token fetchnext();
};

bool validnumber(std::string s);
number readnumber(std::string s);

}
#endif
