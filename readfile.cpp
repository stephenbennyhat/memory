#include <fstream>
#include <iostream>
#include <ios>
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

void
chksum(byte chk, addr a, int lineno, std::istream& is, std::string const& filename)
{
    chk += (a >> 24);
    chk += (a >> 16);
    chk += (a >>  8);
    chk += (a >>  0);
    chk += readhex(is, 2);
    if (chk != 0xFF) {
        std::cerr << filename << ":" << lineno << ": "
                  << "chk err: " 
                  << std::hex << std::showbase << int(chk)
                  << std::endl;
    }
}

} // namespace

namespace mem {

void
readmoto(std::string const& filename, memory& mem) {
    std::ifstream is(filename.c_str());
    if (is.fail()) {
        std::cerr << "couldn't open: \"" << filename << "\"" << std::endl;
        return;
    }
    readmoto(is, mem, filename);
}

void
readmoto(std::istream& is, memory& mem, std::string const& filename) {
    std::string line;
    int lineno = 1;
    unsigned rcnt = 0;
    do {
        char ch;
        is >> ch;
        if (ch != 'S') {
            if (is)
                std::cerr << filename << ":" << lineno << ": "
                          << "not an srecord:\"" << line << "\"" << std::endl;
        }
        else {
            is >> ch;

            if (ch >= '1' && ch <= '3') {
                int addrlen = 1 + ch - '0';
                int count = readhex(is, 2);
                byte chk = (byte) count;
                count -= addrlen + 1; // just data bytes.
                addr a = readhex(is, addrlen * 2);
                if (0) std::cerr << std::hex << "addr=" << a
                                 << std::dec << " count=" << count
                                 << std::endl;
                for (int i = 0; i < count; i++) {
                    byte b = (byte) readhex(is, 2);
                    mem[a+i] = b;
                    chk += b;
                }
                chksum(chk, a, lineno, is, filename);
                rcnt++;
            }
            else if (ch == '5') {
                int addrlen = 2;
                int count = readhex(is, 2);
                byte chk = (byte) count;
                count -= addrlen + 1; // just data bytes.
                addr a = readhex(is, addrlen * 2);
                if (rcnt != a) {
                    std::cerr << filename << ":" << lineno << ": "
                              << "record count: " << std::hex << int(rcnt) << "!=" << a
                              << std::endl;
                }
                chksum(chk, a, lineno, is, filename);
            }
            else if (ch >= '7' && ch <= '9') {
                int addrlen = 11 + '0' - ch;
                int count = readhex(is, 2);
                byte chk = (byte) count;
                addr a = readhex(is, addrlen * 2);
                mem.setexecaddr(a);
                chksum(chk, a, lineno, is, filename);
            }
            else if (ch == '0') {
                int addrlen = 2;
                int count = readhex(is, 2);
                byte chk = (byte) count;
                count -= addrlen + 1; // just data bytes.
                addr a = readhex(is, addrlen * 2);
                std::string s;
                for (int i = 0; i < count; i++) {
                    char ch = (char) readhex(is, 2);
                    chk += ch;
                    s.push_back(ch);
                }
                mem.setdesc(s);
                chksum(chk, a, lineno, is, filename);
            }
            else {
                std::cerr << filename << ":" << lineno << ": "
                          << "ignored: S" << ch
                          << std::endl;
            }
        }
        std::string line;
        getline(is, line);
        lineno++;
    }
    while (is);
}

} // namespace
