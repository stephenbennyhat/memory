#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "memory.h"
#include "readfile.h"
#include "crc.h"


void
chkiter()
{
    memory m1;

    m1.add(1, 4);
    m1.add(3, 5);

    std::cout << m1;

    memory::iterator end = m1.end();
    for (memory::iterator i = m1.begin(); i != end; i++) {
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

    std::cout << "nullchk=" << std::hex << crc16(memory()) << std::endl;

    memory om = offset(cm, 1);
    std::cout << om << std::endl;

    chkiter();
}
