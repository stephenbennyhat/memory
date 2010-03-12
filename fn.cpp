#include <vector>
#include <fstream>
#include "parse.h"
#include "var.h"
#include "fn.h"
#include "mem.h"

using std::vector;

var
readfn(vector<var> const& args)
{
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
writefn(vector<var> const& args)
{
    size_t n = args.size();

    if (n >= 2) {
        std::cout << "error: bad arguments" << std::endl;
        return var();
    }

    args[0].check(var::tmemory);
    if (n > 0) args[1].check(var::tstring);

    var const& m = args[0];

    std::ostream *p = &std::cout;
    std::ofstream os;

    if (n > 1) {
        os.open(args[1].getstring().c_str());
        p = &os;
    }

    mem::writemoto(*p, m.getmemory()); 
    return var();
}

var
printfn(vector<var> const& args)
{
    std::string ofs = ", ";
    std::string ors = "\n";

    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i];
        if (i + 1 != args.size())
            std::cout << ofs;
    }
    std::cout << ors;
    return var();
}

var
crc16fn(vector<var> const& args)
{
    if (args.size() != 1) {
        std::cout << "error: bad arguments" << std::endl;
        return var();
    }

    var const& v = args[0];
    v.check(var::tmemory);

    number n = mem::crc16(v.getmemory());
    return n;
}
