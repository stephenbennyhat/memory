#include <iostream>
#include <iomanip>
#include <vector>
#include "memory.h"
#include "writefile.h"

void
writeline(std::ostream& os, memory const& mem, addr a, int len, int addrlen)
{
    os << std::hex << std::setw(2);
    os << 'S' << (addrlen - 1) << (len + 1 + addrlen);
    os << std::endl;
}

void
writemoto(std::ostream& os, memory const& mem, int addrlen, int maxline)
{
    std::vector<byte> v;
    addr a;
    addr pa = -1;

    os << mem;

    memory::const_iterator end = mem.end();
    for (memory::const_iterator i = mem.begin(); i != end; i++) {
        if (a + 1 != i->first || v.size() == maxline) {
            a = i->first;
            if (!v.empty()) {
                writeline(os, mem, pa, v.size(), addrlen);
            }
            pa = i->first;
            v = std::vector<byte>();
        }
    }
    if (!v.empty())
        writeline(os, mem, pa, v.size(), addrlen);
}
