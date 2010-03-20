#include <iostream>
#include <string>
#include "var.h"

namespace memory {

    using std::string;
    using mem::memory;
    using mem::range;

    void var::check(vartype t) const {
        if (t != type())
            throw type_error(t, type(), "incompatible");
    }

    var::var() {
        struct nullimpl : public impl {
            virtual vartype type() const { return tnull; }
        };
        impl_ = pi(static_cast<impl*>(new nullimpl));
    }

    var::var(memory const& m) {
        struct memimpl : impl {
            virtual vartype type() const { return tmemory; }
            virtual memory& getmemory() { return m_; }
            memory m_;
            memimpl(memory const& m) : m_(m) {}
        };
        impl_ = pi(static_cast<impl*>(new memimpl(m)));
    }

    var::var(range const& r) {
        struct rangeimpl : impl {
            virtual vartype type() const { return trange; }
            virtual range& getrange() { return r_; }
            range r_;
            rangeimpl(range const& r) : r_(r) {}
        };
        impl_ = pi(static_cast<impl*>(new rangeimpl(r)));
    }

    var::var(number const& n) {
        struct numimpl : impl {
            virtual vartype type() const { return tnumber; }
            virtual number& getnumber() { return n_; }
            number n_;
            numimpl(number const& n) : n_(n) {}
        };
        impl_ = pi(static_cast<impl*>(new numimpl(n)));
    }

    var::var(string const& s) {
        struct strimpl : impl {
            virtual vartype type() const { return tstring; }
            virtual string& getstring() { return s_; }
            string s_;
            strimpl(string const& s) : s_(s) {}
        };
        impl_ = pi(static_cast<impl*>(new strimpl(s)));
    }

    var::var(fn const& f) {
        struct fnimpl : impl {
            virtual vartype type() const { return tfunction; }
            virtual fn& getfunction() { return f_; }
            fn f_;
            fnimpl(fn const& f) : f_(f) {}
        };
        impl *p = new fnimpl(f);
        impl_ = pi(p);
    }

    var::operator bool() const {
        if (type() == tnull) return false;
        if (type() == tnumber) return getnumber() != 0;
        return true;
    }

    void var::print(std::ostream& os) const {
        switch (type()) {
        default:
           os << typestr(type());
           break;
        case tmemory:
           os << getmemory();
           break;
        case trange:
           os << getrange();
           break;
        case tnumber:
           os << std::hex << std::showbase << getnumber();
           break;
        case tstring:
           os << getstring();
           break;
        }
    }

    std::string var::typestr(vartype t) {
        switch (t) {
        default:      return "unknown"; break;
        case tnull:   return "(null)"; break;
        case tmemory: return "memory"; break;
        case trange:  return "range"; break;
        case tnumber: return "number"; break;
        case tstring: return "string"; break;
        case tfunction: return "function"; break;
        }
    }

    symtab::symbol& symtab::lookup(std::string const &s) {
        for (symstype::iterator i = syms.begin(); i != syms.end(); i++) {
            scope::iterator ci = (*i).find(s);
            if (ci != (*i).end())
                return ci->second;
        }
        return insert(s, var());
    }

    symtab::symbol& symtab::insert(std::string const& s, var const& val, bool dyn) {
        return syms.front()[s] = symbol(val, s, dyn);
    }

    void
    symtab::print(std::ostream& os) const {
        int level = 0;
        for (symstype::const_iterator bi = syms.begin(); bi != syms.end(); ++bi) {
            scope const& sc = *bi;
            string s = ""; for (int i = 0; i < level; i++) s += " ";
            os << s << "scope: " << ++level << std::endl;
            for (scope::const_iterator si = sc.begin(); si != sc.end(); ++si) {
                os << s << " " << si->first << "=" << si->second.v_
                   << " (" << (si->second.dyn_ ? "dyn" : "lex") << ")"
                   << std::endl;
            }
        }
    }
}


