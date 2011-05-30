#ifndef FN_H
#define FN_H

#include <vector>
#include "var.h"

namespace memory {
    // functions
    var readfn(std::vector<pv> const& args);
    var printfn(std::vector<pv> const& args);
    var crc16fn(std::vector<pv> const& args);
    var writefn(std::vector<pv> const& args);
    var rangefn(std::vector<pv> const& args);
    var offsetfn(std::vector<pv> const& args);
    var joinfn(std::vector<pv> const& args);

    // binops
    var eqop(pv const& v1, pv const& v2);
    var neop(pv const& v1, pv const& v2);
    var gtop(pv const& v1, pv const& v2);
    var ltop(pv const& v1, pv const& v2);
    var add(pv const& v1, pv const& v2);
    var sub(pv const& v1, pv const& v2);
    var mul(pv const& v1, pv const& v2);
    var index(pv const& v1, pv const& v2);
    var mkrange(pv const& v1, pv const& v2);

    // primops
    var notop(pv const& v);
}
#endif

