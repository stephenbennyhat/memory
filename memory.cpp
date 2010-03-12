#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"
#include "parse.h"

int
main(int argc, char **argv)
{
    try {
        if (argc == 1) {
            parser p(std::cin);
            p.parse();
        }
        else for (int i = 1; i < argc; i++) {
            std::ifstream is(argv[i]);
            parser p(is);
            p.parse();
        }
    }
    catch (parser::parse_error) {
        std::cerr << "could not parse file" << std::endl;
    }
    catch (parser::type_error) {
        std::cerr << "type error" << std::endl;
    }
}
