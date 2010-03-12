#include <iostream>
#include "var.h"

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
       os << getnumber();
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

} // namespace
