/**
 * @file Repl.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript REPL - Read Eval Print Loop
 * @version 1.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_REPL_REPL_HPP
#define ARK_REPL_REPL_HPP

#include <iostream>
#include <filesystem>
#include <optional>

#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/State.hpp>

#include <replxx.hxx>

namespace Ark
{
    class Repl
    {
    public:
        /**
         * @brief Construct a new Repl object
         *
         * @param lib_env search path for the std library
         */
        explicit Repl(const std::vector<std::filesystem::path>& lib_env);

        /**
         * @brief Start the REPL
         *
         */
        int run();

    private:
        replxx::Replxx m_repl;
        int m_old_ip;
        std::vector<std::filesystem::path> m_lib_env;
        unsigned m_line_count;
        bool m_running;

        void cuiSetup();

        std::optional<std::string> getLine();
        std::optional<std::string> getCodeBlock();
    };
}

#endif
