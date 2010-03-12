#ifndef VAR_H
#define VAR_H

#include "mem.h"
#include "lex.h"

class var {
public:
    enum vartype {
       tnull,
       tmemory,
       trange,
       tnumber,
       tstring,
    };

    struct type_error : public std::exception {
        std::string s_;
        vartype exp_;
        vartype was_;

        type_error(std::string s, vartype exp, vartype was)
                 : s_(s), exp_(exp), was_(was) {}

        ~type_error() throw() {}

        friend std::ostream& operator<<(std::ostream& os, type_error const& te) {
            os << "type error: " << typestr(te.was_) << " != " << typestr(te.exp_) ;
            if (!te.s_.empty())
                os << ": " << te.s_;
            return os;
        }
    };

    var() : t_(tnull) {} // for containers
    var(mem::memory m) : t_(tmemory), m_(m) {}
    var(mem::range r) : t_(trange), r_(r) {}
    var(number r) : t_(tnumber), n_(r) {}
    var(std::string s) : t_(tstring), s_(s) {}

    vartype type() const { return t_; }

    mem::memory& getmemory() {
        check(tmemory);
        return m_;
    }
    mem::memory const& getmemory() const {
        check(tmemory);
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

    void check(vartype t) const {
        if (t != t_) {
            throw type_error("type error", t, t_);
        }
    }
    static std::string typestr(vartype t);

private:
    vartype t_;

    // in a better world i'd hold these by pointer to derived type.
    mem::memory m_;
    mem::range r_;
    number n_;
    std::string s_;

public:
    void print(std::ostream& os) const;
};

inline
std::ostream& operator<<(std::ostream& os, var const& v) {
    v.print(os);
    return os;
}

#endif
