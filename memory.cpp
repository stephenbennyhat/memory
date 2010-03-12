#include <fstream>
#include <sstream>
#include <ctype.h>
#include "mem.h"
#include "parse.h"

int
main(int argc, char **argv)
{
    try {
        bool e = false;
        if (argv[1][0] == '-') {
            if (argv[1][1] == 'e') {
                std::istringstream is(argv[2]);
                parser(is).parse();
                e = true;
                argv+=2; argc-=2;
            }
        }

        if (argc == 1 && !e) {
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
    catch (var::type_error const& te) {
        std::cerr << "type error: " << te << std::endl;
    }
}
