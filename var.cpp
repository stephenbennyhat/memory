#include <iostream>
#include "var.h"

void var::print(std::ostream& os) const {
    switch (t_) {
    case tnull:
    default:
       os << "(null)";
       break;
    case tmem:
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
