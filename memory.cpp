#include <fstream>
#include <sstream>
#include <ctype.h>
#include "mem.h"
#include "parse.h"

using memory::parser;
using memory::var;

int
main(int argc, char **argv) {
    try {
        bool e = false;

        while (argv[1] && argv[1][0] == '-') {
            if (argv[1][1] == 'e') {
                std::istringstream is(argv[2]);
                parser(is).parse();
                e = true;
                argv+=2; argc-=2;
            }
        }

        for (int i = 1; i < argc; i++) {
            std::ifstream is(argv[i]);
            parser(is).parse();
            e = true;
        }
        if (!e) {
                parser(std::cin).parse();
        }
    }
    catch (parser::parse_error) {
        std::cerr << "could not parse file" << std::endl;
    }
    catch (var::type_error const& te) {
        std::cerr << "type error: " << te << std::endl;
    }
}
