#ifndef FN_H
#define FN_H

#include <vector>
#include "var.h"

namespace memory {
    // functions
    var::var readfn(std::vector<var::var> const& args);
    var::var printfn(std::vector<var::var> const& args);
    var::var crc16fn(std::vector<var::var> const& args);
    var::var writefn(std::vector<var::var> const& args);
    var::var rangefn(std::vector<var::var> const& args);
    var::var offsetfn(std::vector<var::var> const& args);
    var::var joinfn(std::vector<var::var> const& args);

    // binops
    var::var add(var::var const& v1, var::var const& v2);
    var::var sub(var::var const& v1, var::var const& v2);
    var::var mul(var::var const& v1, var::var const& v2);
    var::var index(var::var const& v1, var::var const& v2);
    var::var mkrange(var::var const& v1, var::var const& v2);
}
#endif

