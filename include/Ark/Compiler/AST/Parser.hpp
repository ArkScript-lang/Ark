#ifndef COMPILER_AST_PARSER_HPP
#define COMPILER_AST_PARSER_HPP

#include <Ark/Compiler/AST/BaseParser.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/AST/Import.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Platform.hpp>

#include <string>
#include <optional>
#include <vector>
#include <functional>

#include <utf8.hpp>

namespace Ark::internal
{
    class ARK_API Parser : public BaseParser
    {
    public:
        Parser();

        void processFile(const std::string& filename);
        void processString(const std::string& code);

        const Node& ast() const;
        const std::vector<Import>& imports() const;

    private:
        Node m_ast;
        std::vector<Import> m_imports;
        unsigned m_allow_macro_behavior;  ///< Toggled on when inside a macro definition, off afterward

        void run();

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
        std::optional<Node> macroBlock();
        std::optional<Node> macroArgs();
        std::optional<Node> macro();
        std::optional<Node> functionCall();
        std::optional<Node> list();

        inline std::optional<Node> number()
        {
            auto pos = getCount();

            std::string res;
            if (signedNumber(&res))
            {
                double output;
                if (Utils::isDouble(res, &output))
                    return Node(output);
                else
                {
                    backtrack(pos);
                    error("Is not a valid number", res);
                }
            }
            return std::nullopt;
        }

        inline std::optional<Node> string()
        {
            std::string res;
            if (accept(IsChar('"')))
            {
                while (true)
                {
                    if (accept(IsChar('\\')))
                    {
                        if (accept(IsChar('"')))
                            res += '\"';
                        else if (accept(IsChar('\\')))
                            res += '\\';
                        else if (accept(IsChar('n')))
                            res += '\n';
                        else if (accept(IsChar('t')))
                            res += '\t';
                        else if (accept(IsChar('v')))
                            res += '\v';
                        else if (accept(IsChar('r')))
                            res += '\r';
                        else if (accept(IsChar('a')))
                            res += '\a';
                        else if (accept(IsChar('b')))
                            res += '\b';
                        else if (accept(IsChar('0')))
                            res += '\0';
                        else if (accept(IsChar('f')))
                            res += '\f';
                        else if (accept(IsChar('u')))
                        {
                            std::string seq;
                            if (hexNumber(4, &seq))
                            {
                                char utf8_str[5];
                                utf8::decode(seq.c_str(), utf8_str);
                                if (*utf8_str == '\0')
                                    error("Invalid escape sequence", "\\u" + seq);
                                res += utf8_str;
                            }
                            else
                                error("Invalid escape sequence", "\\u");
                        }
                        else if (accept(IsChar('U')))
                        {
                            std::string seq;
                            if (hexNumber(8, &seq))
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
                    else if (isEOF())
                        errorMissingSuffix('"', "string");
                }

                return Node(NodeType::String, res);
            }
            return std::nullopt;
        }

        inline std::optional<Node> field()
        {
            std::string symbol;
            if (!name(&symbol))
                return std::nullopt;

            Node leaf = Node(NodeType::Field);
            leaf.push_back(Node(NodeType::Symbol, symbol));

            while (true)
            {
                if (leaf.list().size() == 1 && !accept(IsChar('.')))  // Symbol:abc
                    return std::nullopt;

                if (leaf.list().size() > 1 && !accept(IsChar('.')))
                    break;
                std::string res;
                if (!name(&res))
                    errorWithNextToken("Expected a field name: <symbol>.<field>");
                leaf.push_back(Node(NodeType::Symbol, res));
            }

            return leaf;
        }

        inline std::optional<Node> symbol()
        {
            std::string res;
            if (!name(&res))
                return std::nullopt;
            return Node(NodeType::Symbol, res);
        }

        inline std::optional<Node> spread()
        {
            std::string res;
            if (sequence("..."))
            {
                if (!name(&res))
                    errorWithNextToken("Expected a name for the variadic");
                return Node(NodeType::Spread, res);
            }
            return std::nullopt;
        }

        inline std::optional<Node> nil()
        {
            if (!accept(IsChar('(')))
                return std::nullopt;
            newlineOrComment();
            if (!accept(IsChar(')')))
                return std::nullopt;

            return Node(NodeType::Symbol, "nil");
        }

        std::optional<Node> atom();
        std::optional<Node> anyAtomOf(std::initializer_list<NodeType> types);
        std::optional<Node> nodeOrValue();
        std::optional<Node> wrapped(std::optional<Node> (Parser::*parser)(), const std::string& name, char prefix, char suffix);
    };
}

#endif
