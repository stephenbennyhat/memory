#include <iostream>
#include <iomanip>
#include <vector>
#include "mem.h"

using namespace mem;

namespace {

void
writeline(std::ostream& os, memory const& mem, addr a, int len, int addrlen)
{
    byte chk;
    int count = (len + 1 + addrlen);

    os << std::setfill('0') << std::uppercase;
    os << std::setw(1) << 'S' << std::hex << (addrlen - 1);
    os << std::setw(2) << std::hex << count;
    os << std::setw(2 * addrlen) << a;
    
    chk = count;
    chk += byte(a >> 24);
    chk += byte(a >> 16);
    chk += byte(a >>  8);
    chk += byte(a >>  0);

    os << std::setw(2);
    for (int i = 0; i < len; i++) {
        byte b = mem[a + i];

        chk += b;
        os << std::setw(2) << unsigned(b);
    }
    os << std::setw(2) << (unsigned(~chk) & 0xFF);
    os << std::endl;
}

}

void
mem::writemoto(std::ostream& os, memory const& mem, int addrlen, int maxline)
{
    std::vector<byte> v;
    addr pa = mem.min();
    addr a = pa;

    memory::const_iterator end = mem.end();
    for (memory::const_iterator i = mem.begin(); i != end; i++) {
        if (a + 1 != i->first || v.size() >= size_t(maxline)) {
            if (!v.empty())
                writeline(os, mem, pa, v.size(), addrlen);

            pa = i->first;
            v = std::vector<byte>();
        }
        a = i->first;
        v.push_back(i->second);
    }
    if (!v.empty())
        writeline(os, mem, pa, v.size(), addrlen);
}
