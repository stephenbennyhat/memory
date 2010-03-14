#ifndef TRACE_H
#define TRACE_H

#include <stdexcept>
#include "lex.h"

namespace tracer {

    struct trace {
        static bool const debug = false;

        trace(std::string s, memory::tokstream& ts) : s_(s), ts_(ts) {
            if (debug)
                std::cout << "enter: " << s_ << " " << ts_ << std::endl;
        }
        ~trace() {
            if (debug)
                std::cout << "exit: " << s_ << " " << ts_
                          << (std::uncaught_exception() ? " (exception)" : "")
                          << std::endl;
        }
        std::string s_;
        memory::tokstream& ts_;
    };

} // namespace

#endif
