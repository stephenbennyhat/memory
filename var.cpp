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

    string typestr(vartype t) {
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

    pv& env::operator[](string s) {
        if (0) std::cerr << "looking up " << s << std::endl;
        env *p = this;
        do {
           scope::iterator i = p->v_.find(s);
           if (i != p->v_.end()) {
               if (0) std::cerr << "found " << s << std::endl;
               return i->second;
           }
           p = p->prev_.get();
        } while (p);
        if (0) std::cerr << "not found " << s << std::endl;
        return v_[s] = pv(new var());
    }

};
