#ifndef VAR_H
#define VAR_H

#include <vector>
#include "port.h"
#include <list>
#include "mem.h"
#include "lex.h"

namespace memory {
    class var;

    typedef port::function<var (std::vector<var> const&)> fn;
    typedef port::function<var ()> xfn;

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

            virtual vartype type() const = 0;
            virtual mem::memory& getmemory() {
               throw type_error(tmemory,  type(), "cannot convert");
            }
            virtual mem::range& getrange() {
               throw type_error(trange,   type(), "cannot convert");
            }
            virtual number& getnumber() { 
               throw type_error(tnumber,  type(), "cannot convert");
            }
            virtual std::string& getstring()   {
               throw type_error(tnumber,  type(), "cannot convert");
            }
            virtual fn& getfunction() {
               throw type_error(tfunction, type(), "cannot convert");
            }
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
        void print(std::ostream& os) const;
    private:
        typedef std::map<std::string, var::var> scope;
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
