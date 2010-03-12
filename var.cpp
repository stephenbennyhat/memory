#include <iostream>
#include "var.h"

void var::print(std::ostream& os) const {
    switch (t_) {
    case tnull:
    default:
       os << "(null)";
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
    }
}
