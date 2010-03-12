#include <iostream>
#include "var.h"

void var::print(std::ostream& os) const {
//    os << "<" << typestr(this->t_) << " ";
    switch (t_) {
    default:
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
//os << ">";
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
