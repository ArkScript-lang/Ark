/**
 * @file Compiler.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript compiler is in charge of transforming the AST into bytecode
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ARK_COMPILER_COMPILER_HPP
#define ARK_COMPILER_COMPILER_HPP

#include <vector>
#include <iostream>
#include <string>
#include <cinttypes>
#include <optional>
#include <functional>

#include <Ark/Compiler/Parser.hpp>
#include <Ark/Compiler/Node.hpp>
#include <Ark/Compiler/CValue.hpp>
#include <Ark/Compiler/Optimizer.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Config.hpp>
#include <Ark/Compiler/makeNodeBasedError.hpp>

namespace Ark
{
    class State;

    /**
     * @brief The ArkScript bytecode compiler
     * 
     */
    class ARK_API_EXPORT Compiler
    {
    public:
        /**
         * @brief Construct a new Compiler object
         * 
         * @param debug the debug level
         * @param lib_dir the path to the standard library
         * @param options the compilers options
         */
        Compiler(unsigned debug, const std::string& lib_dir, uint16_t options=DefaultFeatures);

        /**
         * @brief Feed the differents variables with information taken from the given source code file
         * 
         * @param code the code of the file
         * @param filename the name of the file
         */
        void feed(const std::string& code, const std::string& filename=ARK_NO_NAME_FILE);

        /**
         * @brief Start the compilation
         * 
         */
        void compile();

        /**
         * @brief Save generated bytecode to a file
         * 
         * @param file the name of the file where the bytecode will be saved
         */
        void saveTo(const std::string& file);

        /**
         * @brief Return the constructed bytecode object
         * 
         * @return const bytecode_t& 
         */
        const bytecode_t& bytecode() noexcept;

        friend class Ark::State;

    private:
        Parser m_parser;
        Optimizer m_optimizer;
        uint16_t m_options;
        // tables: symbols, values, plugins and codes
        std::vector<internal::Node> m_symbols;
        std::vector<std::string> m_defined_symbols;
        std::vector<std::string> m_plugins;
        std::vector<internal::CValue> m_values;
        std::vector<std::vector<internal::Inst_t>> m_code_pages;
            // we need a temp code pages for some compilations passes
        std::vector<std::vector<internal::Inst_t>> m_temp_pages;

        bytecode_t m_bytecode;
        unsigned m_debug;

        // helper functions to get a temp or finalized code page
        inline std::vector<internal::Inst_t>& page(int i) noexcept;
        // checking if a symbol is an operator or a builtin
        // because they are implemented the same way

        inline std::size_t countArkObjects(const std::vector<internal::Node>& lst) noexcept;

        /// Checking if a symbol is an operator
        inline std::optional<std::size_t> isOperator(const std::string& name) noexcept;
        /// Checking if a symbol is a builtin
        inline std::optional<std::size_t> isBuiltin(const std::string& name) noexcept;
        /// Checking if a symbol may be coming from a plugin
        inline bool mayBeFromPlugin(const std::string& name) noexcept;

        /// Throw a nice error message
        inline void throwCompilerError(const std::string& message, const internal::Node& node)
        {
            throw CompilationError(internal::makeNodeBasedErrorCtx(message, node));
        }

        /**
         * @brief Compile a single node recursively
         * 
         * @param x the internal::Node to compile
         * @param p the current page number we're on
         */
        void _compile(const internal::Node& x, int p);

        // register a symbol/value/plugin in its own table
        std::size_t addSymbol(const internal::Node& sym) noexcept;
        std::size_t addValue(const internal::Node& x) noexcept;
        std::size_t addValue(std::size_t page_id) noexcept;

        void addDefinedSymbol(const std::string& sym);
        void checkForUndefinedSymbol();

        // push a number on stack (need 2 bytes)
        void pushNumber(uint16_t n, std::vector<internal::Inst_t>* page=nullptr) noexcept;
    };

    #include "Compiler.inl"
}

#endif
