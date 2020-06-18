#ifndef ark_repl
#define ark_repl

#include <iostream>
#include <sstream>

#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/REPL/CGUI.hpp>
#include <Ark/REPL/replxx/util.hpp>

namespace Ark
{
    class Repl
    {
    public:
        /**
         * @brief Construct a new Repl object
         * 
         * @param options the REPL options
         * @param lib_dir the path to the standard library
         */
        Repl(uint16_t options, std::string lib_dir);

        /**
         * @brief Start the REPL
         * 
         */
        void run();

    private:
        std::string m_lib_dir;
        uint16_t m_options;
        Replxx m_repl;
        unsigned m_scope = 0;
        unsigned m_lines = 1;
        int m_old_ip = 0;

        inline void print_repl_header();
        int count_open_parentheses(const std::string& line);
        int count_open_braces(const std::string& line);
        void trim_whitespace(std::string& line);
        void scope_update(const std::string& line);
        void cgui_setup();
    };
}

#endif