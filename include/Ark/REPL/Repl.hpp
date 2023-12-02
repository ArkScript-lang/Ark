/**
 * @file Repl.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript REPL - Read Eval Print Loop
 * @version 0.2
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef ARK_REPL_REPL_HPP
#define ARK_REPL_REPL_HPP

#include <iostream>
#include <filesystem>

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
         * @param libenv search path for the std library
         */
        explicit Repl(const std::vector<std::filesystem::path>& libenv);

        /**
         * @brief Start the REPL
         *
         */
        int run();

    private:
        Replxx m_repl;
        unsigned m_lines;
        int m_old_ip;
        std::vector<std::filesystem::path> m_libenv;

        static inline void print_repl_header();
        static int count_open_parentheses(const std::string& line);
        static int count_open_braces(const std::string& line);
        static void trim_whitespace(std::string& line);
        void cui_setup();
    };
}

#endif
