/**
 * @file Compiler.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript compiler is in charge of transforming the AST into bytecode
 * @version 1.2
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
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
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Compiler/AST/Optimizer.hpp>
#include <Ark/Compiler/ValTableElem.hpp>

namespace Ark
{
    class State;

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
         * @param options the compilers options
         */
        Compiler(unsigned debug, const std::vector<std::string>& libenv, uint16_t options = DefaultFeatures);

        /**
         * @brief Feed the differents variables with information taken from the given source code file
         * 
         * @param code the code of the file
         * @param filename the name of the file
         */
        void feed(const std::string& code, const std::string& filename = ARK_NO_NAME_FILE);

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
        internal::Parser m_parser;
        internal::Optimizer m_optimizer;
        uint16_t m_options;
        // tables: symbols, values, plugins and codes
        std::vector<internal::Node> m_symbols;
        std::vector<std::string> m_defined_symbols;
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
         * @param i page index, if negative, refers to a temporary code page
         * @return std::vector<internal::Word>& 
         */
        inline std::vector<internal::Word>& page(int i) noexcept
        {
            if (i >= 0)
                return m_code_pages[i];
            return m_temp_pages[-i - 1];
        }

        /**
         * @brief helper functions to get a temp or finalized code page
         * 
         * @param i page index, if negative, refers to a temporary code page
         * @return std::vector<internal::Word>* 
         */
        inline std::vector<internal::Word>* page_ptr(int i) noexcept
        {
            if (i >= 0)
                return &m_code_pages[i];
            return &m_temp_pages[-i - 1];
        }

        /**
         * @brief Count the number of "valid" ark objects in a node
         * @details Isn't considered valid a GetField, because we use
         *          this function to count the number of arguments of function calls.
         * 
         * @param lst 
         * @return std::size_t 
         */
        std::size_t countArkObjects(const std::vector<internal::Node>& lst) noexcept;

        /**
         * @brief Checking if a symbol is an operator
         * 
         * @param name symbol name
         * @return std::optional<std::size_t> position in the operators' list
         */
        std::optional<std::size_t> getOperator(const std::string& name) noexcept;

        /**
         * @brief Checking if a symbol is a builtin
         * 
         * @param name symbol name
         * @return std::optional<std::size_t> position in the builtins' list
         */
        std::optional<std::size_t> getBuiltin(const std::string& name) noexcept;

        /**
         * @brief Check if a symbol needs to be compiled to a specific instruction
         * 
         * @param name 
         * @return std::optional<internal::Instruction> corresponding instruction if it exists
         */
        inline std::optional<internal::Instruction> getSpecific(const std::string& name) noexcept
        {
            if (name == "list")
                return internal::Instruction::LIST;
            else if (name == "append")
                return internal::Instruction::APPEND;
            else if (name == "concat")
                return internal::Instruction::CONCAT;
            else if (name == "append!")
                return internal::Instruction::APPEND_IN_PLACE;
            else if (name == "concat!")
                return internal::Instruction::CONCAT_IN_PLACE;
            else if (name == "pop")
                return internal::Instruction::POP_LIST;
            else if (name == "pop!")
                return internal::Instruction::POP_LIST_IN_PLACE;

            return std::nullopt;
        }

        /**
         * @brief Check if a given instruction is unary (takes only one argument)
         * 
         * @param inst 
         * @return true the instruction is unary
         * @return false 
         */
        bool isUnaryInst(internal::Instruction inst) noexcept;

        /**
         * @brief Compute specific instruction argument count
         * 
         * @param inst 
         * @param previous 
         */
        uint16_t computeSpecificInstArgc(internal::Instruction inst, uint16_t previous) noexcept;

        /**
         * @brief Checking if a symbol may be coming from a plugin
         * 
         * @param name symbol name
         * @return true the symbol may be from a plugin, loaded at runtime
         * @return false 
         */
        bool mayBeFromPlugin(const std::string& name) noexcept;

        /**
         * @brief Throw a nice error message
         * 
         * @param message 
         * @param node 
         */
        [[noreturn]] void throwCompilerError(const std::string& message, const internal::Node& node);

        /**
         * @brief Compile an expression (a node) recursively
         * 
         * @param x the internal::Node to compile
         * @param p the current page number we're on
         * @param is_result_unused 
         * @param is_terminal 
         * @param var_name 
         */
        void compileExpression(const internal::Node& x, int p, bool is_result_unused, bool is_terminal, const std::string& var_name = "");

        void compileSymbol(const internal::Node& x, int p, bool is_result_unused);
        void compileSpecific(const internal::Node& c0, const internal::Node& x, int p, bool is_result_unused);
        void compileIf(const internal::Node& x, int p, bool is_result_unused, bool is_terminal, const std::string& var_name);
        void compileFunction(const internal::Node& x, int p, bool is_result_unused, const std::string& var_name);
        void compileLetMutSet(internal::Keyword n, const internal::Node& x, int p);
        void compileWhile(const internal::Node& x, int p);
        void compileQuote(const internal::Node& x, int p, bool is_result_unused, bool is_terminal, const std::string& var_name);
        void compilePluginImport(const internal::Node& x, int p);
        void handleCalls(const internal::Node& x, int p, bool is_result_unused, bool is_terminal, const std::string& var_name);

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

        /**
         * @brief Register a symbol as defined, so that later we can throw errors on undefined symbols
         * 
         * @param sym 
         */
        void addDefinedSymbol(const std::string& sym);

        /**
         * @brief Checks for undefined symbols, not present in the defined symbols table
         * 
         */
        void checkForUndefinedSymbol();

        /**
         * @brief Suggest a symbol of what the user may have meant to input
         *
         * @param str the string
         * @return std::string
         */
        std::string offerSuggestion(const std::string& str);
    };
}

#endif
