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
    readfn(vector<pv> const& args) {
        if (args.size() != 1) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        pv v = args[0];
        v->check(tstring);
        std::string filename = v->getstring();
        mem::memory m;
        readmoto(filename, m); 
        return m;
    }

    var
    writefn(vector<pv> const& args) {
        size_t n = args.size();
        if (n == 1) {
            mem::writemoto(std::cout, args[0]->getmemory()); 
        }
        else if (n == 2) {
            std::ofstream os(args[1]->getstring().c_str());
            mem::writemoto(os, args[0]->getmemory()); 
        }
        else {
            std::cout << "error: bad arguments" << std::endl;
        }
        return var();
    }

    var
    printfn(vector<pv> const& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << *args[i];
            if (i + 1 != args.size())
                std::cout << ofs;
        }
        std::cout << ors;
        return var();
    }

    var
    crc16fn(vector<pv> const& args) {
        if (args.size() != 1) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        pv v = args[0];
        return mem::crc16(v->getmemory());
    }

    var
    rangefn(vector<pv> const& args) {
        if (args.size() != 1) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        return args[0]->getmemory().getrange();
    }

    var
    offsetfn(vector<pv> const& args) {
        if (args.size() != 2) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        mem::memory const& m = args[0]->getmemory();
        number n = args[1]->getnumber();

        return offset(m, n);
    }

    var
    joinfn(vector<pv> const& args) {
        if (args.size() != 2) {
            std::cout << "error: bad arguments" << std::endl;
            return var();
        }
        return join(args[0]->getmemory(), args[1]->getmemory());
    }

    var
    add(pv const& v1, pv const& v2) {
        if (v1->is(tnumber) && v2->is(tnumber))
            return v1->getnumber() + v2->getnumber();
        if (v1->is(tmemory) && v2->is(tmemory))
            return join(v1->getmemory(), v2->getmemory());
        throw type_error(v1->type(), v2->type(), "cannot add");
    }

    var
    sub(pv const& v1, pv const& v2) {
        if (v1->is(tnumber) && v2->is(tnumber))
            return v1->getnumber() - v2->getnumber();
        throw type_error(v1->type(), v2->type(), "cannot sub");
    }

    var
    mul(pv const& v1, pv const& v2) {
        return v1->getnumber() * v2->getnumber(); //XXX
    }

    var
    eqop(pv const& v1, pv const& v2) {
        if (v1->type() != v2->type()) return false;
        if (v1->is(tnumber))
           return v1->getnumber() == v2->getnumber();
        throw type_error(v1->type(), v2->type(), "cannot cmp");
    }

    var
    neop(pv const& v1, pv const& v2) {
        return !eqop(v1, v2);
    }

    var
    gtop(pv const& v1, pv const& v2) {
        if (v1->is(tnumber) && v2->is(tnumber))
           return v1->getnumber() > v2->getnumber();
        throw type_error(v1->type(), v2->type(), "cannot cmp");
    }

    var
    ltop(pv const& v1, pv const& v2) {
        if (v1->is(tnumber) && v2->is(tnumber))
           return v1->getnumber() < v2->getnumber();
        throw type_error(v1->type(), v2->type(), "cannot cmp");
    }

    var
    index(pv const& v1, pv const& v2) {
        if (v1->is(tmemory)) {
            if (v2->is(trange))
                return crop(v1->getmemory(), v2->getrange());
            if (v2->is(tnumber))
                return v1->getmemory()[v2->getnumber()];
        }
        throw type_error(v1->type(), v2->type(), "cannot index");
    }

    var
    mkrange(pv const& v1, pv const& v2) {
        return mem::range(v1->getnumber(), v2->getnumber());
    }

    var
    notop(pv const& v)
    {
        if (v->is(tnull))
            return var(1);
        if (v->is(tnumber))
            return !v->getnumber();
        return var();
    }
}
