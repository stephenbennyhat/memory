#include <iostream>
#include <string>
#include "var.h"

using std::string;

namespace memory {
    void var::print(std::ostream& os) const {
        switch (t_) {
        default:
           break;
        case tfunction:
           os << typestr(t_);
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
        default: 
        case tnull:   return "(null)"; break;
        case tmemory: return "memory"; break;
        case trange:  return "range"; break;
        case tnumber: return "number"; break;
        case tstring: return "string"; break;
        case tfunction: return "function"; break;
        }
    }

    var& symtab::lookup(std::string const &s) {
        for (symstype::iterator i = syms.begin(); i != syms.end(); i++) {
            scope::iterator ci = (*i).find(s);
            if (ci != (*i).end())
                return ci->second;
        }
        return insert(s, var());
    }

    var& symtab::insert(std::string const& s, var const& val) {
        return syms.front()[s] = val;
    }

    void
    symtab::print(std::ostream& os) const {
        int level = 0;
        for (symstype::const_iterator bi = syms.begin(); bi != syms.end(); ++bi) {
            scope const& sc = *bi;
            string s = ""; for (int i = 0; i < level; i++) s += " ";
            os << s << "scope: " << ++level << std::endl;
            for (scope::const_iterator si = sc.begin(); si != sc.end(); ++si) {
                os << s << " " << si->first << "=" << si->second << std::endl;
            }
        }
    }
}
