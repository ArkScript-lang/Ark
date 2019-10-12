#ifndef ark_repl
#define ark_repl

#include <iostream>
#include <sstream>

#include <Ark/Constants.hpp>

namespace Ark {
    class Repl {
    public:
        Repl(uint16_t options) : m_options(options) {};
        void run();

    private:
        uint16_t m_options;

        void print_repl_header();
        int count_open_parentheses(std::string line);
    };
}

#endif