#ifndef FN_H
#define FN_H

#include <vector>
#include "var.h"

namespace memory {

var::var readfn(std::vector<var::var> const& args);
var::var printfn(std::vector<var::var> const& args);
var::var crc16fn(std::vector<var::var> const& args);
var::var writefn(std::vector<var::var> const& args);
var::var rangefn(std::vector<var::var> const& args);

}
#endif
