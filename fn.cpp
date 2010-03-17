#include <vector>
#include <fstream>
#include "parse.h"
#include "var.h"
#include "fn.h"
#include "mem.h"

using std::vector;

namespace memory {
    std::string ofs = "";
    std::string ors = "\n";

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

    var
    rangefn(vector<var> const& args) {
        if (args.size() != 1) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        return args[0].getmemory().getrange();
    }

    var
    offsetfn(vector<var> const& args) {
        if (args.size() != 2) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        mem::memory const& m = args[0].getmemory();
        number n = args[1].getnumber();

        return offset(m, n);
    }

    var
    joinfn(vector<var> const& args) {
        if (args.size() != 2) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        return join(args[0].getmemory(), args[1].getmemory());
    }

    var
    add(var const& v1, var const& v2) {
        if (v1.is(var::tnumber) && v2.is(var::tnumber))
            return v1.getnumber() + v2.getnumber();
        if (v1.is(var::tmemory) && v2.is(var::tmemory))
            return join(v1.getmemory(), v2.getmemory());
        throw new var::type_error(v1.type(), v2.type(), "cannot add");
    }

    var
    mul(var const& v1, var const& v2) {
        return v1.getnumber() * v2.getnumber(); //XXX
    }

    var
    index(var const& v1, var const& v2) {
        if (v1.is(var::tmemory)) {
            if (v2.is(var::trange))
                return crop(v1.getmemory(), v2.getrange());
            if (v2.is(var::tnumber))
                return v1.getmemory()[v2.getnumber()];
        }
        throw new var::type_error(v1.type(), v2.type(), "cannot index");
    }

    var
    mkrange(var const& v1, var const& v2) {
        return mem::range(v1.getnumber(), v2.getnumber());
    }
}
