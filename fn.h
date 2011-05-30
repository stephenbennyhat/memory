#ifndef FN_H
#define FN_H

#include <vector>
#include "var.h"

namespace memory {
    // functions
    var readfn(std::vector<pv> const& args, pe const& e);
    var printfn(std::vector<pv> const& args, pe const& e);
    var crc16fn(std::vector<pv> const& args, pe const& e);
    var writefn(std::vector<pv> const& args, pe const& e);
    var rangefn(std::vector<pv> const& args, pe const& e);
    var offsetfn(std::vector<pv> const& args, pe const& e);
    var joinfn(std::vector<pv> const& args, pe const& e);

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

