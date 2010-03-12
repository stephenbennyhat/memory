#ifndef FN_H
#define FN_H

#include <vector>
#include "var.h"

var readfn(std::vector<var> const& args);
var printfn(std::vector<var> const& args);
var crc16fn(std::vector<var> const& args);
var writefn(std::vector<var> const& args);

#endif
