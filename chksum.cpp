#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>

#include "memory.h"
#include "readfile.h"
#include "writefile.h"
#include "crc.h"


void
chkiter()
{
    memory m1;

    m1.insert(1, 4);
    m1.insert(3, 5);
    m1.insert(4, 6);

    std::cout << m1;

    memory::const_iterator end = m1.end();
    for (memory::const_iterator i = m1.begin(); i != end; i++) {
        std::cout << (*i).first << " " << int((*i).second) << std::endl;
    }
}

int
main(int argc, char **argv)
{
    std::ifstream in(argv[1]);
    memory mem;

    readmoto(in, mem);
    std::cout << mem << std::endl;

    memory nm = fill(mem, range(0, 0x100000), 0xFF);
    std::cout << nm << std::endl;

    memory cm = crop(mem, range(0xE0000, 0x100000));
    std::cout << cm << std::endl;

    bool b = mem == cm;
    std::cout << "same: " << b << std::endl;

    memory om = offset(cm, 1);
    std::cout << om << std::endl;
    assert(om.min() == cm.min() + 1);
    assert(om.max() == cm.max() + 1);

    std::cout << "nullchk=" << std::hex << crc16(memory()) << std::endl;

    chkiter();

    writemoto(std::cout, cm);
}
