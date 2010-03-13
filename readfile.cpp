#include <fstream>
#include <iostream>
#include <string>
#include "mem.h"

using namespace mem;

namespace {

addr
readhex(std::istream& is, int width) {
   int v = 0;
   for (int i = 0; i < width; i++) {
       char ch;
       int digit =  0;
       is >> ch;
       if (!is) {
           throw "Parse Error";
       }
       else if (ch >= 'A' && ch <= 'F') {
           digit = ch - 'A' + 10;
       }
       else if (ch >= 'a' && ch <= 'f') {
           digit = ch - 'a' + 10;
       }
       else if (ch >= '0' && ch <= '9') {
           digit = ch - '0';
       }
       else {
           throw "Unexpected digit";
       }
       v = v << 4 | digit;
   }
   return v;
}

}

namespace mem {

void
readmoto(std::string filename, memory& mem) {
    std::ifstream is(filename.c_str());
    if (is.fail()) {
        std::cerr << "couldn't open: \"" << filename << "\"" << std::endl;
        return;
    }
    readmoto(is, mem);
}

void
readmoto(std::istream& is, memory& mem) {
    std::string line;
    unsigned rcnt = 0;
    do {
        char ch;
        is >> ch;
        if (ch != 'S') {
            if (is)
                std::cerr << "not an srecord:\"" << line << "\"" << std::endl;
        }
        else {
            is >> ch;

            if (ch >= '1' && ch <= '3') {
                int addrlen = 1 + ch - '0';
                int count = readhex(is, 2);
                byte chk = (byte) count;
                count -= addrlen + 1; // just data bytes.
                addr a = readhex(is, addrlen * 2);
                chk += (a >> 24) + (a >> 16) + (a >> 8) + (a >> 0);
                if (0) std::cerr << std::hex << "addr=" << a
                                 << std::dec << " count=" << count
                                 << std::endl;
                for (int i = 0; i < count; i++) {
                    byte b = (byte) readhex(is, 2);
                    mem[a++] = b;
                    chk += b;
                }
                byte chk2 = readhex(is, 2);
                chk += chk2;
                if (chk != 0xFF) {
                    std::cerr << "chk err: " << std::hex << int(chk) << std::endl;
                }
                rcnt++;
            }
            else if (ch == '5') {
                int addrlen = 2;
                int count = readhex(is, 2);
                byte chk = (byte) count;
                count -= addrlen + 1; // just data bytes.
                addr a = readhex(is, addrlen * 2);
                if (rcnt != a) {
                    std::cerr << "record count: " << std::hex << int(rcnt) 
                              << "!=" << a << std::endl;
                }
            }
            else {
                std::cerr << "ignored: S" << ch << std::endl;
            }
        }
        std::string line;
        getline(is, line);
    }
    while (is);
}

} // namespace
