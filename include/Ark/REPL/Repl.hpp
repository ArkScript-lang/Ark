#ifndef ark_repl
#define ark_repl

#include <iostream>
#include <sstream>

#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/State.hpp>

namespace Ark
{
    class Repl
    {
    public:
        Repl(uint16_t options, std::string lib_dir) : 
            m_options(options), m_lib_dir(lib_dir) {};
        void run();

    private:
        std::string m_lib_dir;
        uint16_t m_options;

        inline void print_repl_header();
        int count_open_parentheses(std::string& line);
        int count_open_braces(std::string& line);
    };
}

#endif