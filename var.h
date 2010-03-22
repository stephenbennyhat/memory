#ifndef VAR_H
#define VAR_H

#include <vector>
#include "port.h"
#include <list>
#include "mem.h"
#include "lex.h"

namespace memory {
    class var;
    typedef struct port::shared_ptr<var> pv;
    typedef port::function<var (std::vector<pv> const&)> fn;
    typedef port::function<pv ()> xfn;
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

        var();
        var(mem::memory const& m);
        var(mem::range const& r);
        var(number const& r);
        var(std::string const& s);
        var(fn const& f);

        vartype type() const { return impl_->type();}
        operator bool() const;

        mem::memory const& getmemory() const { return impl_->getmemory(); }
        mem::range const& getrange() const { return impl_->getrange(); }
        number const& getnumber() const { return impl_->getnumber(); }
        std::string const& getstring() const { return impl_->getstring(); }
        fn const& getfunction() const { return impl_->getfunction(); }

        bool is(vartype t) const { return type() == t; }
        void check(vartype t) const;
        static std::string typestr(vartype t);
    private:
        struct impl {
            void check(vartype t) const {
                if (t != type())
                    throw var::type_error(t, type(), "type check failed");
            };
            template <class T>
               T noconvert(vartype t) const {
               throw type_error(t,  type(), "cannot convert");
            }
            virtual vartype type() const = 0;
            virtual mem::memory& getmemory() { return noconvert<mem::memory&>(tmemory); }
            virtual mem::range& getrange() { return noconvert<mem::range&>(trange); }
            virtual number& getnumber() { return noconvert<number&>(tnumber); }
            virtual std::string& getstring() { return noconvert<std::string&>(tstring); }
            virtual fn& getfunction() { return noconvert<fn&>(tfunction); }
        };
        typedef port::shared_ptr<impl> pi;
        pi impl_;
    public:
        void print(std::ostream& os) const;
    };

    inline
    std::ostream& operator<<(std::ostream& os, var const& v) {
        v.print(os);
        return os;
    }

    struct symbol {
        pv v_;
        std::string name_;
        symbol(pv v, std::string name) : v_(v), name_(name) {}
        symbol() : name_("(none)") {}
    };

    class symtab {
    public:
        symbol& lookup(std::string const& name);
        symbol& insert(std::string const& name, pv v);
        symtab() { push(); }
        void push() { syms.push_front(scope()); }
        void pop() { syms.pop_front(); }
        symbol& operator[](std::string const& s) { return lookup(s); }
        void print(std::ostream& os) const;
    private:
        typedef std::map<std::string, symbol> scope;
        typedef std::list<scope> symstype;
        symstype syms;
    };
    inline
    std::ostream& operator<<(std::ostream& os, symtab const& v) {
        v.print(os);
        return os;
    }

}
#endif
