/**
 * @file Compiler.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript compiler is in charge of transforming the AST into bytecode
 * @version 3.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_COMPILER_COMPILER_HPP
#define ARK_COMPILER_COMPILER_HPP

#include <vector>
#include <string>
#include <cinttypes>
#include <optional>

#include <Ark/Platform.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/Word.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/ValTableElem.hpp>

namespace Ark
{
    class State;
    class Welder;

    /**
     * @brief The ArkScript bytecode compiler
     *
     */
    class ARK_API Compiler
    {
    public:
        /**
         * @brief Construct a new Compiler object
         *
         * @param debug the debug level
         */
        explicit Compiler(unsigned debug);

        /**
         * @brief Start the compilation
         *
         * @param ast
         */
        void process(const internal::Node& ast);

        /**
         * @brief Return the constructed bytecode object
         *
         * @return const bytecode_t&
         */
        [[nodiscard]] const bytecode_t& bytecode() const noexcept;

        friend class State;
        friend class Welder;

    private:
        struct Page
        {
            std::size_t index;
            bool is_temp;
        };

        // tables: symbols, values, plugins and codes
        std::vector<internal::Node> m_symbols;
        std::vector<std::string> m_plugins;
        std::vector<internal::ValTableElem> m_values;
        std::vector<std::vector<internal::Word>> m_code_pages;
        std::vector<std::vector<internal::Word>> m_temp_pages;  ///< we need temporary code pages for some compilations passes

        bytecode_t m_bytecode;
        unsigned m_debug;  ///< the debug level of the compiler

        /**
         * @brief Push the file headers (magic, version used, timestamp)
         *
         */
        void pushFileHeader() noexcept;

        /**
         * @brief Push the symbols and values tables
         *
         */
        void pushSymAndValTables();

        /**
         * @brief helper functions to get a temp or finalized code page
         *
         * @param page page descriptor
         * @return std::vector<internal::Word>&
         */
        std::vector<internal::Word>& page(const Page page) noexcept
        {
            if (!page.is_temp)
                return m_code_pages[page.index];
            return m_temp_pages[page.index];
        }

        /**
         * @brief helper functions to get a temp or finalized code page
         *
         * @param page page descriptor
         * @return std::vector<internal::Word>*
         */
        std::vector<internal::Word>* page_ptr(const Page page) noexcept
        {
            if (!page.is_temp)
                return &m_code_pages[page.index];
            return &m_temp_pages[page.index];
        }

        /**
         * @brief Checking if a symbol is an operator
         *
         * @param name symbol name
         * @return std::optional<uint8_t> operator instruction
         */
        static std::optional<uint8_t> getOperator(const std::string& name) noexcept;

        /**
         * @brief Checking if a symbol is a builtin
         *
         * @param name symbol name
         * @return std::optional<uint16_t> builtin number
         */
        static std::optional<uint16_t> getBuiltin(const std::string& name) noexcept;

        /**
         * @brief Checking if a symbol is a list instruction
         *
         * @param name
         * @return std::optional<internal::Instruction> list instruction
         */
        static std::optional<internal::Instruction> getListInstruction(const std::string& name) noexcept;

        /**
         * Checks if a node is a list and has a keyboard as its first node, indicating if it's producing a value on the stack or not
         * @param node node to check
         * @return true if the node produces an output on the stack (fun, if, begin)
         * @return false otherwise (let, mut, set, while, import, del)
         */
        static bool nodeProducesOutput(const internal::Node& node);

        /**
         * @brief Check if a given instruction is unary (takes only one argument)
         *
         * @param inst
         * @return true the instruction is unary
         * @return false
         */
        static bool isUnaryInst(internal::Instruction inst) noexcept;

        /**
         * @brief Checking if a symbol may be coming from a plugin
         *
         * @param name symbol name
         * @return true the symbol may be from a plugin, loaded at runtime
         * @return false
         */
        bool mayBeFromPlugin(const std::string& name) noexcept;

        /**
         * @brief Display a warning message
         *
         * @param message
         * @param node
         */
        static void compilerWarning(const std::string& message, const internal::Node& node);

        /**
         * @brief Throw a nice error message
         *
         * @param message
         * @param node
         */
        [[noreturn]] static void throwCompilerError(const std::string& message, const internal::Node& node);

        /**
         * @brief Compile an expression (a node) recursively
         *
         * @param x the internal::Node to compile
         * @param p the current page number we're on
         * @param is_result_unused
         * @param is_terminal
         * @param var_name
         */
        void compileExpression(const internal::Node& x, Page p, bool is_result_unused, bool is_terminal, const std::string& var_name = "");

        void compileSymbol(const internal::Node& x, Page p, bool is_result_unused);
        void compileListInstruction(const internal::Node& c0, const internal::Node& x, Page p, bool is_result_unused);
        void compileIf(const internal::Node& x, Page p, bool is_result_unused, bool is_terminal, const std::string& var_name);
        void compileFunction(const internal::Node& x, Page p, bool is_result_unused, const std::string& var_name);
        void compileLetMutSet(internal::Keyword n, const internal::Node& x, Page p);
        void compileWhile(const internal::Node& x, Page p);
        void compilePluginImport(const internal::Node& x, Page p);
        void handleCalls(const internal::Node& x, Page p, bool is_result_unused, bool is_terminal, const std::string& var_name);

        /**
         * @brief Register a given node in the symbol table
         * @details Can throw if the table is full
         *
         * @param sym
         * @return uint16_t
         */
        uint16_t addSymbol(const internal::Node& sym);

        /**
         * @brief Register a given node in the value table
         * @details Can throw if the table is full
         *
         * @param x
         * @return uint16_t
         */
        uint16_t addValue(const internal::Node& x);

        /**
         * @brief Register a page id (function reference) in the value table
         * @details Can throw if the table is full
         *
         * @param page_id
         * @param current A reference to the current node, for context
         * @return std::size_t
         */
        uint16_t addValue(std::size_t page_id, const internal::Node& current);
    };
}

#endif
