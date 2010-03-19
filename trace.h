#ifndef TRACE_H
#define TRACE_H

#include <stdexcept>
#include "lex.h"

namespace tracer {
    struct trace {
        static bool debug;
        static std::string prefix;

        trace(std::string s, memory::tokstream& ts) : s_(s), ts_(ts) {
            if (debug)
                std::cout << prefix << "enter: " << s_ << " " << ts_ << std::endl;
            prefix.push_back(' ');
        }
        ~trace() {
            prefix = prefix.substr(0, prefix.length() - 1);
            if (debug)
                std::cout << prefix << "exit: " << s_ << " " << ts_
                          << (std::uncaught_exception() ? " (exception)" : "")
                          << std::endl;
        }
        std::string s_;
        memory::tokstream& ts_;
    };
}


#endif
