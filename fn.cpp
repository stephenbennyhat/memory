#include <vector>
#include <fstream>
#include "parse.h"
#include "var.h"
#include "fn.h"
#include "mem.h"

namespace memory {

std::string ofs = "";
std::string ors = "\n";

using std::vector;

var
readfn(vector<var> const& args) {
    if (args.size() != 1) {
        std::cout << "error: bad arguments" << std::endl;
        return var();
    }
    var const& v = args[0];
    v.check(var::tstring);
    std::string filename = v.getstring();
    mem::memory m;
    readmoto(filename, m); 
    return m;
}

var
writefn(vector<var> const& args) {
    size_t n = args.size();
    if (n == 1) {
        mem::writemoto(std::cout, args[0].getmemory()); 
    }
    else if (n == 2) {
        std::ofstream os(args[1].getstring().c_str());
        mem::writemoto(os, args[0].getmemory()); 
    }
    else {
        std::cout << "error: bad arguments" << std::endl;
    }
    return var();
}

var
printfn(vector<var> const& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i];
        if (i + 1 != args.size())
            std::cout << ofs;
    }
    std::cout << ors;
    return var();
}

var
crc16fn(vector<var> const& args) {
    if (args.size() != 1) {
        std::cout << "error: bad arguments" << std::endl;
        return var();
    }
    var const& v = args[0];
    number n = mem::crc16(v.getmemory());
    return n;
}

} // namespace
