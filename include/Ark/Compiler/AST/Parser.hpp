/**
 * @file Parser.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Parse ArkScript code, but do not handle any import declarations
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024-2026
 *
 */

#ifndef ARK_COMPILER_AST_PARSER_HPP
#define ARK_COMPILER_AST_PARSER_HPP

#include <Ark/Compiler/AST/BaseParser.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>
#include <Ark/Utils/Logger.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Utils/Platform.hpp>

#include <string>
#include <vector>
#include <optional>
#include <functional>

#include <utf8.hpp>

namespace Ark::internal
{
    enum class ParserMode
    {
        Interpret,  ///< Escape sequences and `()` will be replaced by their UTF8 representation and `nil`, respectively
        Raw         ///< Keep all text as is without modifying it (useful for the code formatter)
    };

    class ARK_API Parser final : public BaseParser
    {
    public:
        /**
         * @brief Constructs a new Parser object
         * @param debug debug level
         * @param mode how the parser should behave regarding certain nodes and errors
         */
        explicit Parser(unsigned debug, ParserMode mode = ParserMode::Interpret);

        /**
         * @brief Parse the given code
         * @param filename can be left empty, used for error generation
         * @param code content of the file
         */
        void process(const std::string& filename, const std::string& code);

        /**
         *
         * @return const Node& resulting AST after processing the given code
         */
        [[nodiscard]] const Node& ast() const noexcept;

        /**
         *
         * @return const std::vector<Import>& list of imports detected by the parser
         */
        [[nodiscard]] const std::vector<Import>& imports() const;

    private:
        ParserMode m_mode;
        Logger m_logger;
        Node m_ast;
        std::vector<Import> m_imports;
        unsigned m_allow_macro_behavior;  ///< Toggled on when inside a macro definition, off afterward
        std::size_t m_nested_nodes;       ///< Nested node counter
        std::vector<std::function<std::optional<Node>(FilePosition)>> m_parsers;

        [[nodiscard]] Node positioned(Node node, FilePosition cursor) const;
        [[nodiscard]] std::optional<Node>& positioned(std::optional<Node>& node, FilePosition cursor) const;

        std::optional<Node> node();
        std::optional<Node> letMutSet(FilePosition filepos);
        std::optional<Node> del(FilePosition filepos);
        std::optional<Node> condition(FilePosition filepos);
        std::optional<Node> loop(FilePosition filepos);
        std::optional<Node> import_(FilePosition filepos);
        std::optional<Node> block(FilePosition filepos);
        std::optional<Node> functionArgs(FilePosition filepos);
        std::optional<Node> function(FilePosition filepos);
        std::optional<Node> macroCondition(FilePosition filepos);
        std::optional<Node> macroArgs(FilePosition filepos);
        std::optional<Node> macro(FilePosition filepos);
        std::optional<Node> functionCall(FilePosition filepos);
        std::optional<Node> list(FilePosition filepos);

        std::optional<Node> number(FilePosition filepos);
        std::optional<Node> string(FilePosition filepos);
        std::optional<Node> field(FilePosition filepos);
        std::optional<Node> symbol(FilePosition filepos);
        std::optional<Node> spread(FilePosition filepos);
        std::optional<Node> nil(FilePosition filepos);

        /**
         * @brief Try to parse an atom (number, string, spread, field, symbol, nil)
         * @return std::optional<Node> std::nullopt if no atom could be parsed
         */
        std::optional<Node> atom();

        /**
         * @brief Try to parse an atom, if any, match its type against the given list
         * @param types authorized types
         * @return std::optional<Node> std::nullopt if the parsed atom didn't match the given types
         */
        std::optional<Node> anyAtomOf(std::initializer_list<NodeType> types);

        /**
         * @brief Try to parse an atom first, if it fails try to parse a node
         * @return std::optional<Node> std::nullopt if no atom or node could be parsed
         */
        std::optional<Node> nodeOrValue();

        /**
         * @brief Try to parse using a given parser, prefixing and suffixing it with (...), handling comments around the parsed node
         * @param parser parser method returning a std::optional<Node>
         * @param name construction name, eg "let", "condition"
         * @return std::optional<Node> std::nullopt if the parser didn't match
         */
        std::optional<Node> wrapped(std::optional<Node> (Parser::*parser)(FilePosition), const std::string& name);
    };
}

#endif  // ARK_COMPILER_AST_PARSER_HPP
