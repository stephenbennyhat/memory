#include <fstream>
#include <sstream>
#include <ctype.h>
#include "mem.h"
#include "parse.h"

using memory::parser;
using memory::var;
using std::string;

int
main(int argc, char **argv) {
    try {
        bool e = false;
        bool r = false;
        bool w = false;
        bool pr = false;
        bool tty = false;
        parser p;

        while (argv[1] && argv[1][0] == '-') {
            switch (argv[1][1]) {
            case 'e':
                if (argv[2]) {
                    p.parse(string(argv[2]) + ";");
                    e = true;
                }
                argv+=2; argc-=2;
                break;
            case 'r':
                if (argv[2]) {
                    string cmd = string("read(\"") + argv[2] + "\")";
                    if (r) {
                        cmd = "join(_, " + cmd + ")";
                    }
                    r = true;
                    p.parse(cmd + ";");
                    argv+=2; argc-=2;
                }
                break;
            case 'w':
                if (argv[2]) {
                    string cmd = string("write(_, \"") + argv[2] + "\")";
                    w = true;
                    p.parse(cmd + ";");
                    argv+=2; argc-=2;
                }
                break;
            case 'p':
                p.parse("print(_);");
                argv++; argc--;
                pr = true;
                break;
            case 't':
                tty = true;
                argv++; argc--;
                break;
            default:
                break;
            }
        }

        for (int i = 1; i < argc; i++) {
            std::ifstream is(argv[i]);
            p.parse(is);
            e = true;
        }
        if ((!e && !w && !pr) || tty) {
            p.parse(std::cin);
        }
    }
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}
