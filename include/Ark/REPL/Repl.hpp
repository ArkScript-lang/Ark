/**
 * @file Repl.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript REPL - Read Eval Print Loop
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef ARK_REPL_REPL_HPP
#define ARK_REPL_REPL_HPP

#include <iostream>

#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/REPL/ConsoleStyle.hpp>

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
        Repl(uint16_t options, const std::string& lib_dir);

        /**
         * @brief Start the REPL
         *
         */
        int run();

    private:
        std::string m_lib_dir;
        uint16_t m_options;
        Replxx m_repl;
        unsigned m_lines;
        int m_old_ip;

        inline void print_repl_header();
        int count_open_parentheses(const std::string& line);
        int count_open_braces(const std::string& line);
        void trim_whitespace(std::string& line);
        void cui_setup();
    };
}

#endif
