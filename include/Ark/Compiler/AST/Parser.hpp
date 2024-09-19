/**
 * @file Parser.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Parse ArkScript code, but do not handle any import declarations
 * @version 0.2
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef COMPILER_AST_PARSER_HPP
#define COMPILER_AST_PARSER_HPP

#include <Ark/Compiler/AST/BaseParser.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>
#include <Ark/Logger.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Platform.hpp>

#include <string>
#include <optional>
#include <vector>

#include <utf8.hpp>

namespace Ark::internal
{
    class ARK_API Parser final : public BaseParser
    {
    public:
        /**
         * @brief Constructs a new Parser object
         * @param debug debug level
         * @param interpret interpret escape codes in strings
         */
        explicit Parser(unsigned debug, bool interpret = true);

        void process(const std::string& filename, const std::string& code);

        [[nodiscard]] const Node& ast() const noexcept;
        [[nodiscard]] const std::vector<Import>& imports() const;

    private:
        bool m_interpret;
        Logger m_logger;
        Node m_ast;
        std::vector<Import> m_imports;
        unsigned m_allow_macro_behavior;  ///< Toggled on when inside a macro definition, off afterward

        void run();

        Node& setNodePosAndFilename(Node& node, const std::optional<FilePosition>& cursor = std::nullopt) const;

        std::optional<Node> node();
        std::optional<Node> letMutSet();
        std::optional<Node> del();
        std::optional<Node> condition();
        std::optional<Node> loop();
        std::optional<Node> import_();
        std::optional<Node> block();
        std::optional<Node> functionArgs();
        std::optional<Node> function();
        std::optional<Node> macroCondition();
        std::optional<Node> macroArgs();
        std::optional<Node> macro();
        std::optional<Node> functionCall();
        std::optional<Node> list();

        std::optional<Node> number()
        {
            auto pos = getCount();

            std::string res;
            if (signedNumber(&res))
            {
                double output;
                if (Utils::isDouble(res, &output))
                    return std::optional<Node>(output);
                backtrack(pos);
                error("Is not a valid number", res);
            }
            return std::nullopt;
        }

        std::optional<Node> string()
        {
            std::string res;
            if (accept(IsChar('"')))
            {
                while (true)
                {
                    if (accept(IsChar('\\')))
                    {
                        if (!m_interpret)
                            res += '\\';

                        if (accept(IsChar('"')))
                            res += '"';
                        else if (accept(IsChar('\\')))
                            res += '\\';
                        else if (accept(IsChar('n')))
                            res += m_interpret ? '\n' : 'n';
                        else if (accept(IsChar('t')))
                            res += m_interpret ? '\t' : 't';
                        else if (accept(IsChar('v')))
                            res += m_interpret ? '\v' : 'v';
                        else if (accept(IsChar('r')))
                            res += m_interpret ? '\r' : 'r';
                        else if (accept(IsChar('a')))
                            res += m_interpret ? '\a' : 'a';
                        else if (accept(IsChar('b')))
                            res += m_interpret ? '\b' : 'b';
                        else if (accept(IsChar('f')))
                            res += m_interpret ? '\f' : 'f';
                        else if (accept(IsChar('u')))
                        {
                            std::string seq;
                            if (hexNumber(4, &seq))
                            {
                                if (m_interpret)
                                {
                                    char utf8_str[5];
                                    utf8::decode(seq.c_str(), utf8_str);
                                    if (*utf8_str == '\0')
                                        error("Invalid escape sequence", "\\u" + seq);
                                    res += utf8_str;
                                }
                                else
                                    res += "u" + seq;
                            }
                            else
                                error("Invalid escape sequence", "\\u");
                        }
                        else if (accept(IsChar('U')))
                        {
                            std::string seq;
                            if (hexNumber(8, &seq))
                            {
                                if (m_interpret)
                                {
                                    std::size_t begin = 0;
                                    for (; seq[begin] == '0'; ++begin)
                                        ;
                                    char utf8_str[5];
                                    utf8::decode(seq.c_str() + begin, utf8_str);
                                    if (*utf8_str == '\0')
                                        error("Invalid escape sequence", "\\U" + seq);
                                    res += utf8_str;
                                }
                                else
                                    res += "U" + seq;
                            }
                            else
                                error("Invalid escape sequence", "\\U");
                        }
                        else
                        {
                            backtrack(getCount() - 1);
                            error("Unknown escape sequence", "\\");
                        }
                    }
                    else
                        accept(IsNot(IsEither(IsChar('\\'), IsChar('"'))), &res);

                    if (accept(IsChar('"')))
                        break;
                    if (isEOF())
                        errorMissingSuffix('"', "string");
                }

                return { Node(NodeType::String, res) };
            }
            return std::nullopt;
        }

        std::optional<Node> field()
        {
            std::string sym;
            if (!name(&sym))
                return std::nullopt;

            std::optional<Node> leaf { Node(NodeType::Field) };
            setNodePosAndFilename(leaf.value());
            leaf->push_back(Node(NodeType::Symbol, sym));

            while (true)
            {
                if (leaf->list().size() == 1 && !accept(IsChar('.')))  // Symbol:abc
                    return std::nullopt;

                if (leaf->list().size() > 1 && !accept(IsChar('.')))
                    break;
                std::string res;
                if (!name(&res))
                    errorWithNextToken("Expected a field name: <symbol>.<field>");
                leaf->push_back(Node(NodeType::Symbol, res));
            }

            return leaf;
        }

        std::optional<Node> symbol()
        {
            std::string res;
            if (!name(&res))
                return std::nullopt;
            return { Node(NodeType::Symbol, res) };
        }

        std::optional<Node> spread()
        {
            std::string res;
            if (sequence("..."))
            {
                if (!name(&res))
                    errorWithNextToken("Expected a name for the variadic");
                return { Node(NodeType::Spread, res) };
            }
            return std::nullopt;
        }

        std::optional<Node> nil()
        {
            if (!accept(IsChar('(')))
                return std::nullopt;

            std::string comment;
            newlineOrComment(&comment);
            if (!accept(IsChar(')')))
                return std::nullopt;

            if (m_interpret)
                return { Node(NodeType::Symbol, "nil").attachNearestCommentBefore(comment) };
            return { Node(NodeType::List).attachNearestCommentBefore(comment) };
        }

        std::optional<Node> atom();
        std::optional<Node> anyAtomOf(std::initializer_list<NodeType> types);
        std::optional<Node> nodeOrValue();
        std::optional<Node> wrapped(std::optional<Node> (Parser::*parser)(), const std::string& name);
    };
}

#endif
