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

    inline
    std::ostream& operator<<(std::ostream& os, coord const& c) {
        return os << c.line << ':' << c.chr;
    }

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
        token() : type_(0), desc_("null") {}
    private:
        int type_;
        std::string s_;
        coord pos_;
        std::string desc_;
    };

    std::ostream& operator<<(std::ostream& os, token const& tok);

    struct lexer_error : public std::exception {
        lexer_error(std::string const& s) : s_(s) {}
        ~lexer_error() throw() {}
        virtual char const *what() const throw() { return s_.c_str(); }
    private:
        std::string s_;
    };

    class lexer_base {
    public:
        virtual token next() = 0;
        virtual ~lexer_base() {}
    };

    class tokstream {
    public:
        tokstream(lexer_base& lb) : lb_(&lb) {
            toks_.push_back(token());
        }
        tokstream() {}
        tokstream operator=(tokstream const& other) {
            lb_ = other.lb_;
            toks_ = other.toks_;
            return *this;
        }
        virtual ~tokstream();
        token operator[](int i);
        void consume();
        void consumeuntil(int t);
        void print(std::ostream& os) const;

        static int const eof = -1;
        static int const null = 0;
    private:
        void load() { toks_.push_back(lb_->next()); }
        lexer_base *lb_;
        std::deque<token> toks_;
    };

    inline
    std::ostream& operator<<(std::ostream& os, tokstream& ts)
    {
        ts.print(os);
        return os;
    }

    struct lexer : public lexer_base {
        enum toktype {
            str = 65536,
            num,
            dotdot,
            name,
            fn,
            iftok,
            elsetok,
            whiletok,
        };
        explicit lexer(std::istream& is) : is_(is) {}
        virtual ~lexer() {}
        virtual token next();
    private:
        std::istream& is_;
        coord pos_;
        int getchar();
    };

    bool validnumber(std::string const& s);
    number readnumber(std::string const &s);

}
#endif
