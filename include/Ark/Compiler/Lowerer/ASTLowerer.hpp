/**
 * @file ASTLowerver.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript compiler is in charge of transforming the AST into IR
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_COMPILER_LOWERER_ASTLOWERER_HPP
#define ARK_COMPILER_LOWERER_ASTLOWERER_HPP

#include <stack>
#include <vector>
#include <string>
#include <cinttypes>
#include <optional>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Utils/Logger.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Compiler/IntermediateRepresentation/Entity.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/ValTableElem.hpp>
#include <Ark/Compiler/Lowerer/LocalsLocator.hpp>

namespace Ark
{
    class State;
    class Welder;
}

namespace Ark::internal
{
    struct PageCreationData
    {
        bool temp { false };
        bool closure { false };
        std::optional<std::string> name { std::nullopt };
    };

    /**
     * @brief The ArkScript AST to IR compiler
     *
     */
    class ARK_API ASTLowerer final : public Pass
    {
    public:
        /**
         * @brief Construct a new ASTLowerer object
         *
         * @param debug the debug level
         */
        explicit ASTLowerer(unsigned debug);

        /**
         * @brief Pre-fill tables (used by the debugger)
         *
         * @param symbols
         * @param constants
         */
        void addToTables(const std::vector<std::string>& symbols, const std::vector<ValTableElem>& constants);

        /**
         * @brief Start bytecode pages at a given offset (by default, 0)
         *
         * @param offset
         */
        void offsetPagesBy(std::size_t offset);

        /**
         * @brief Start the compilation
         *
         * @param ast
         */
        void process(Node& ast);

        /**
         * @brief Return the IR blocks (one per scope)
         *
         * @return const std::vector<Block>&
         */
        [[nodiscard]] const std::vector<IR::Block>& intermediateRepresentation() const noexcept;

        /**
         * @brief Return the symbol table pre-computed
         *
         * @return const std::vector<std::string>&
         */
        [[nodiscard]] const std::vector<std::string>& symbols() const noexcept;

        /**
         * @brief Return the value table pre-computed
         *
         * @return const std::vector<ValTableElem>&
         */
        [[nodiscard]] const std::vector<ValTableElem>& values() const noexcept;

        [[nodiscard]] IR::label_t lastLabel() const noexcept
        {
            return m_current_label;
        }

    private:
        struct Page
        {
            std::size_t index;
            bool is_temp;
        };

        struct Var
        {
            std::string name;
            std::size_t argument_count;
        };

        LocalsLocator m_locals_locator;

        // tables: symbols, values, plugins and codes
        std::vector<std::string> m_symbols;
        std::vector<ValTableElem> m_values;
        std::size_t m_start_page_at_offset = 0;  ///< Used to offset the page numbers when compiling code in the debugger
        std::vector<IR::Block> m_code_pages;
        std::vector<IR::Block> m_temp_pages;  ///< we need temporary code pages for some compilations passes
        IR::label_t m_current_label = 0;
        std::stack<Var> m_opened_vars;  ///< stack of vars we are currently declaring

        enum class ErrorKind
        {
            InvalidNodeMacro,
            InvalidNodeNoReturnValue,
            InvalidNodeInOperatorNoReturnValue,
            InvalidNodeInTailCallNoReturnValue
        };

        Page createNewCodePage(PageCreationData&& args = PageCreationData {}) noexcept
        {
            if (!args.temp)
            {
                const std::size_t new_page_addr = m_start_page_at_offset + m_code_pages.size();
                m_code_pages.emplace_back(
                    IR::Block::Metadata {
                        .name = args.name,
                        .argument_count = 0,
                        .addr = new_page_addr,
                        .is_closure = args.closure },
                    IR::Block::vec_t {});
                return Page { .index = new_page_addr, .is_temp = false };
            }

            m_temp_pages.emplace_back();
            return Page { .index = m_temp_pages.size() - 1u, .is_temp = true };
        }

        IR::Block& block(const Page page) noexcept
        {
            if (!page.is_temp)
                return m_code_pages[page.index - m_start_page_at_offset];
            return m_temp_pages[page.index];
        }

        /**
         * @brief helper functions to get a temp or finalised code page
         *
         * @param page page descriptor
         * @return std::vector<IR::Entity>&
         */
        IR::Block::vec_t& page(const Page page) noexcept
        {
            if (!page.is_temp)
                return m_code_pages[page.index - m_start_page_at_offset].data;
            return m_temp_pages[page.index].data;
        }

        /**
         * @brief Check if we are in a recursive self call
         *
         * @param name symbol name being compiled
         * @return true if the name passed is the name of the last function we entered
         */
        [[nodiscard]] bool isFunctionCallingItself(const std::string& name) noexcept
        {
            return !m_opened_vars.empty() && m_opened_vars.top().name == name;
        }

        /**
         * @brief Checking if a symbol is an operator
         *
         * @param name symbol name
         * @return std::optional<Instruction> operator instruction
         */
        static std::optional<Instruction> getOperator(const std::string& name) noexcept;

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
         * @return std::optional<Instruction> list instruction
         */
        static std::optional<Instruction> getListInstruction(const std::string& name) noexcept;

        /**
         * Checks if a node is a list and is a call to 'breakpoint'
         * @param node node to check
         * @return true if the node is a 'breakpoint' call: (breakpoint <cond>)
         * @return false otherwise
         */
        static bool isBreakpoint(const Node& node);

        /**
         * Checks if a node is a list and has a keyboard as its first node, indicating if it's producing a value on the stack or not
         * @param node node to check
         * @return true if the node produces an output on the stack (fun, if, begin)
         * @return false otherwise (let, mut, set, while, import, del)
         */
        static bool nodeProducesOutput(const Node& node);

        /**
         * @brief Check if a given instruction is unary (takes only one argument)
         *
         * @param inst
         * @return true the instruction is unary, false otherwise
         */
        static bool isUnaryInst(Instruction inst) noexcept;

        /**
         * @brief Check if a given instruction is ternary (takes three arguments)
         *
         * @param inst
         * @return true the instruction is ternary, false otherwise
         */
        static bool isTernaryInst(Instruction inst) noexcept;

        /**
         * @brief Check if an operator can be repeated
         *
         * @param inst
         * @return true the instruction can be repeated, eg (+ 1 2 3) compiles to (+ (+ 1 2) 3), false otherwise
         */
        static bool isRepeatableOperation(Instruction inst) noexcept;

        /**
         * @brief Display a warning message
         *
         * @param message
         * @param node
         */
        void warning(const std::string& message, const Node& node);

        /**
         * @brief Throw a nice error message
         *
         * @param message
         * @param node
         */
        [[noreturn]] static void buildAndThrowError(const std::string& message, const Node& node);

        /**
         * @brief Throw a nice error message, using a message builder
         *
         * @param kind error kind
         * @param node erroneous node
         * @param additional_ctx optional context for the error, e.g. the macro name
         */
        static void makeError(ErrorKind kind, const Node& node, const std::string& additional_ctx);

        /**
         * @brief Compile an expression (a node) recursively
         *
         * @param x the Node to compile
         * @param p the current page number we're on
         * @param is_result_unused
         * @param is_terminal
         * @param can_use_ref
         */
        void compileExpression(Node& x, Page p, bool is_result_unused, bool is_terminal, bool can_use_ref);

        void compileSymbol(const Node& x, Page p, bool is_result_unused, bool can_use_ref);
        void compileListInstruction(Node& x, Page p, bool is_result_unused);
        void compileApplyInstruction(Node& x, Page p, bool is_result_unused);
        void compileIf(Node& x, Page p, bool is_result_unused, bool is_terminal, bool can_use_ref);
        void compileFunction(Node& x, Page p, bool is_result_unused);
        void setFunctionMetadata(Page p, std::size_t arg_count, bool mutates_args);
        void compileLetMutSet(Keyword n, Node& x, Page p, bool is_result_unused);
        void compileWhile(Node& x, Page p);
        void compilePluginImport(const Node& x, Page p);
        void pushFunctionCallArguments(Node& call, Page p, bool is_tail_call);
        void handleCalls(Node& x, Page p, bool is_result_unused, bool is_terminal, bool can_use_ref);
        void handleShortcircuit(Node& x, Page p, bool can_use_ref);
        void handleOperator(Node& x, Page p, Instruction op);
        bool handleFunctionCall(Node& x, Page p, bool is_terminal);

        /**
         * @brief Register a given node in the symbol table
         * @details Can throw if the table is full
         *
         * @param sym
         * @return uint16_t
         */
        uint16_t addSymbol(const Node& sym);

        /**
         * @brief Register a given node in the value table
         * @details Can throw if the table is full
         *
         * @param x
         * @return uint16_t
         */
        uint16_t addValue(const Node& x);

        /**
         * @brief Register a page id (function reference) in the value table
         * @details Can throw if the table is full
         *
         * @param page_id
         * @param current A reference to the current node, for context
         * @return std::size_t
         */
        uint16_t addValue(std::size_t page_id, const Node& current);
    };
}

#endif
