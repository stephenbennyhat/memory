#ifndef VAR_H
#define VAR_H

#include "mem.h"
#include "lex.h"

struct type_error : public std::exception {
            type_error(std::string s) {}
};

class var {
public:
    enum vartype {
       tnull,
       tmem,
       trange,
       tnumber,
       tstring,
    };

    var() : t_(tnull) {} // for containers
    var(mem::memory m) : t_(tmem), m_(m) {}
    var(mem::range r) : t_(trange), r_(r) {}
    var(number r) : t_(tnumber), n_(r) {}
    var(std::string s) : t_(tstring), s_(s) {}

    vartype type() const { return t_; }

    mem::memory& getmemory() {
        check(tmem);
        return m_;
    }
    mem::memory const& getmemory() const {
        check(tmem);
        return m_;
    }
    mem::range& getrange() {
        check(trange);
        return r_;
    }
    mem::range const& getrange() const {
        check(trange);
        return r_;
    }
    number& getnumber() {
        check(tnumber);
        return n_;
    }
    number const& getnumber() const {
        check(tnumber);
        return n_;
    }
    std::string& getstring() {
        check(tstring);
        return s_;
    }
    std::string const& getstring() const {
        check(tstring);
        return s_;
    }
private:
    vartype t_;

    mem::memory m_;
    mem::range r_;
    number n_;
    std::string s_;

    void check(vartype t) const {
        if (t != t_) {
            throw type_error("type error");
        }
    }

public:
    void print(std::ostream& os) const;
};

inline
std::ostream& operator<<(std::ostream& os, var const& v) {
    v.print(os);
    return os;
}

#endif
