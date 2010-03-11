#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>
#include <assert.h>
#include <ctype.h>
#include "mem.h"

int
main(int argc, char **argv)
{
    parser p();

    try {
        if (argc == 1) {
            p.parse(std::cin);
        }
        else for (int i = 1; i < argc; i++) {
            std::ifstream is(argv[i]);
            p.parse(is);
        }
    }
    catch (parse_error) {
        std::cerr << "could not parse file" << std::endl;
    }
    catch (type_error) {
        std::cerr << "type error" << std::endl;
    }
}
