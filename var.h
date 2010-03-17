#ifndef VAR_H
#define VAR_H

#include <vector>
#include <list>
#include <tr1/functional>
#include "mem.h"
#include "lex.h"

namespace memory {
    class var;

    typedef std::tr1::function<var (std::vector<var> const&)> fn;
    typedef std::tr1::function<var ()> xfn;

    class var {
    public:
        enum vartype {
           tnull,
           tmemory,
           trange,
           tnumber,
           tstring,
           tfunction,
        };

        struct type_error : public std::exception {
            type_error(vartype const& exp, vartype const& was, std::string const& s = "") {
                s_ = "type error: " + typestr(was) + "/" + typestr(exp);
                if (!s_.empty())
                    s_ += ": " +  s;
            }
            virtual char const *what() const throw() { return s_.c_str(); }
            ~type_error() throw() {}
        private:
            std::string s_;
        };

        var() : t_(tnull) {} // for containers

        var(mem::memory m) : t_(tmemory), m_(m) {}
        var(mem::range r) : t_(trange), r_(r) {}
        var(number r) : t_(tnumber), n_(r) {}
        var(std::string s) : t_(tstring), s_(s) {}
        var(fn f) : t_(tfunction), f_(f) {}

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
        fn &getfunction() {
            check(tfunction);
            return f_;
        }
        fn const& getfunction() const {
            check(tfunction);
            return f_;
        }
        bool is(vartype t) const { return t == t_; }
        void check(vartype t) const {
            if (t != t_) {
                throw type_error(t, t_, "incompatible");
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
        fn f_;
    public:
        void print(std::ostream& os) const;
    };

    inline
    std::ostream& operator<<(std::ostream& os, var const& v) {
        v.print(os);
        return os;
    }

    class symtab {
    public:
        var& lookup(std::string const &name);
        var& insert(std::string const& name, var const& val);
        symtab() { push(); }
        void push() {
            syms.push_front(scope());
        }
        void pop() {
            syms.pop_front();
        }
        var& operator[](std::string const& s) {
            return lookup(s);
        }
    private:
        typedef std::map<std::string, var::var> scope;
        typedef std::list<scope> symstype;
        symstype syms;
    };
}
#endif
