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
            if (i->count(s) != 0) {
                return (*i)[s];
            }
        }
        return insert(s, var());
    }

    var& symtab::insert(std::string const& s, var const& val) {
        return syms.front()[s] = val;
    }

    //void
    //symtab::print(std::ostream& os) const {
        //symtab::const_iterator end = syms.end();
        //os << "nsyms: " << syms.size() << std::endl;
        //for (symtab::const_iterator i = syms.begin(); i != end; ++i) {
            //os << " syms[" << i->first << "] = " << i->second << std::endl;
        //}
    //}
}
