#include <Ark/Compiler/AST/Lexer.hpp>

#include <cstdio>  // TODO remove
#include <algorithm>
#include <utility>
#include <sstream>
#include <utf8_decoder.h>

#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Compiler/AST/makeErrorCtx.hpp>

namespace Ark::internal
{
    Lexer::Lexer(unsigned debug) noexcept :
        m_debug(debug)
    {}

    void Lexer::feed(const std::string& code)
    {
        std::size_t line = 0, character = 0;
        std::size_t saved_line = 0, saved_char = 0;
        // flags
        bool in_string = false, in_ctrl_char = false, in_comment = false;
        // buffers
        std::string buffer, ctrl_char;

        auto append_token_from_buffer = [&]() {
            TokenType type = guessType(buffer);
            // tokenizing error management
            if (type == TokenType::Mismatch)
                throwTokenizingError("invalid token '" + buffer + "'", buffer, line, character, code);
            else if (type == TokenType::Capture || type == TokenType::GetField)
                buffer = buffer.substr(1);  // remove the & or the .
            m_tokens.emplace_back(type, buffer, saved_line, saved_char);
            buffer.clear();
        };

        for (std::size_t pos = 0, end = code.size(); pos < end; ++pos)
        {
            char current = code[pos];

            if (m_debug >= 5)
                std::printf(
                    "buffer: %s - ctrl_char: %s - current: '%c' - line: %zu, char: %zu\n",
                    buffer.c_str(), ctrl_char.c_str(), current, line, character);

            if (!in_string)
            {
                // handle comments first
                if (in_comment)  // append every character to the buffer if we're in a comment, even spaces
                    buffer += current;
                // handle ()[]{} then
                else if (current == '(' || current == ')' || current == '[' || current == ']' || current == '{' || current == '}')
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
                    m_tokens.emplace_back(TokenType::Grouping, std::string(1, current), line, character);
                }
                // handle strings next
                else if (current == '"')
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
                    in_string = true;
                    buffer = "\"";
                    saved_line = line;
                    saved_char = character;
                }
                // handle shorthands, be careful with ! and !=
                else if (current == '\'' || (current == '!' && pos + 1 < code.size() && code[pos + 1] != '=' && buffer.empty()))
                {
                    m_tokens.emplace_back(TokenType::Shorthand, std::string(1, current), line, character);
                }
                // handle comments
                else if (current == '#')
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
                    in_comment = true;
                    buffer = "#";
                }
                // separation
                else if ((current == ' ' || current == '\t' || current == '\v' || current == '\n'))
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
                }
                // capture
                else if (current == '&')
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
                    buffer.clear();
                    buffer += current;
                }
                // getfield or spread
                else if (current == '.')
                {
                    // check numbers, we don't want to split 3.0 into 3 and .0
                    if (!buffer.empty() && !('0' <= buffer[0] && buffer[0] <= '9') && buffer[0] != '+' && buffer[0] != '-' && buffer[0] != '.')
                    {
                        append_token_from_buffer();
                        buffer.clear();
                    }
                    buffer += current;
                }
                // identifier, number, operator
                else
                {
                    if (buffer.empty())
                    {
                        saved_char = character;
                        saved_line = line;
                    }
                    buffer += current;
                }
            }
            else  // we are in a string here
            {
                // check for control character
                if (!in_ctrl_char)
                {
                    if (current == '\\')
                        in_ctrl_char = true;
                    else if (current == '"')  // end of string
                    {
                        buffer += current;
                        in_string = false;
                        m_tokens.emplace_back(TokenType::String, buffer, saved_line, saved_char);
                        buffer.clear();
                    }
                    else
                        buffer += current;
                }
                else
                {
                    // end of escape code
                    if (current == ' ' || endOfControlChar(ctrl_char, current))
                    {
                        // process escape code
                        if (ctrl_char.empty())
                            throwTokenizingError("empty control character '\\' in string", buffer, line, character, code);
                        else if (ctrl_char.size() == 1)
                        {
                            switch (ctrl_char[0])
                            {
                                case '"': buffer += '"'; break;
                                case 'n': buffer += '\n'; break;
                                case 'a': buffer += '\a'; break;
                                case 'b': buffer += '\b'; break;
                                case 't': buffer += '\t'; break;
                                case 'r': buffer += '\r'; break;
                                case 'f': buffer += '\f'; break;
                                case '\\': buffer += '\\'; break;
                                case '0': buffer += '\0'; break;

                                default:
                                    throwTokenizingError("unknown control character '\\" + ctrl_char + "' in string", buffer, line, character, code);
                                    break;
                            }
                        }
                        else
                        {
                            switch (ctrl_char[0])
                            {
                                case 'x': break;  /// @todo

                                case 'u':
                                {
                                    char utf8_str[5];
                                    utf8decode(ctrl_char.c_str() + 1, utf8_str);
                                    if (*utf8_str == '\0')
                                        throwTokenizingError("invalid escape sequence \\" + ctrl_char + " in string, expected hexadecimal number that in utf8 range, got a \"" + ctrl_char + "\"", buffer, line, character + 1, code);
                                    buffer += utf8_str;
                                    break;
                                }

                                case 'U':
                                {
                                    short begin = 1;
                                    for (; ctrl_char[begin] == '0'; ++begin)
                                        ;
                                    char utf8_str[5];
                                    utf8decode(ctrl_char.c_str() + begin, utf8_str);
                                    if (*utf8_str == '\0')
                                        throwTokenizingError("invalid escape sequence \\" + ctrl_char + " in string, expected hexadecimal number that in utf8 range, got a \"" + ctrl_char + "\"", buffer, line, character + 1, code);
                                    buffer += utf8_str;
                                    break;
                                }

                                default:
                                    throwTokenizingError("unknown control character '\\" + ctrl_char + "' in string", buffer, line, character, code);
                                    break;
                            }
                        }

                        ctrl_char.clear();
                        in_ctrl_char = false;

                        if (current == '"')  // end of string
                        {
                            buffer += current;
                            in_string = false;
                            m_tokens.emplace_back(TokenType::String, buffer, saved_line, saved_char);
                            buffer.clear();
                        }
                        else if (current == '\\')  // new escape code
                            in_ctrl_char = true;
                        else
                            buffer += current;
                    }
                    else  // the escape code continues
                        ctrl_char += current;
                }
            }

            // position counter
            if (current == '\n')
            {
                line++;
                character = 0;  // before first character

                // close comments, don't append them
                if (in_comment)
                {
                    in_comment = false;
                    buffer.clear();
                    continue;
                }
            }
            else
            {
                // update position
                character++;
            }
        }

        if (!buffer.empty() && buffer[0] != '#')
            append_token_from_buffer();

        // debugging information
        if (m_debug > 3)
        {
            for (auto& last_token : m_tokens)
            {
                std::printf(
                    "TokenType: %s\tLine: %zu\n[%zu\t]\tToken: %s\n",
                    tokentype_string[static_cast<std::size_t>(last_token.type)].data(),
                    last_token.line,
                    last_token.col,
                    last_token.token.c_str());
            }
        }
    }

    std::vector<Token>& Lexer::tokens() noexcept
    {
        return m_tokens;
    }

    TokenType Lexer::guessType(const std::string& value) noexcept
    {
        if (value.empty())
            return TokenType::Mismatch;

        // assuming we already detected ()[]{}, strings, shorthands and comments
        if (Utils::isDouble(value))  // works on (\+|-)?[[:digit:]]+(\.[[:digit:]]+)?([e|E](\+|-)?[[:digit]]+)?
            return TokenType::Number;
        else if (isOperator(value))
            return TokenType::Operator;
        else if (isKeyword(value))
            return TokenType::Keyword;
        else if (value[0] == '&' && value.size() > 1 && isIdentifier(value))
            return TokenType::Capture;
        else if (value.size() > 3 && value[0] == value[1] && value[1] == value[2] && value[2] == '.')
            return TokenType::Spread;
        else if (value[0] == '.' && value.size() > 1 && isIdentifier(value))
            return TokenType::GetField;
        // otherwise, identifier if it starts with [a-zA-Z_]
        else if (isIdentifier(value))
            return TokenType::Identifier;
        return TokenType::Mismatch;
    }

    bool Lexer::isKeyword(const std::string& value) noexcept
    {
        return std::find(keywords.begin(), keywords.end(), value) != keywords.end();
    }

    bool Lexer::isIdentifier(const std::string& value) noexcept
    {
        return utf8valid(value.c_str());
    }

    bool Lexer::isOperator(const std::string& value) noexcept
    {
        return std::find(operators.begin(), operators.end(), value) != operators.end();
    }

    bool Lexer::endOfControlChar(const std::string& sequence, char next) noexcept
    {
        switch (sequence[0])
        {
            case 'x':
                // \x[any number of hex digits]
                // if it's not a hex digit then it's most likely the end for us
                return !isHexChar(next);

            case 'u':
                return sequence.size() == 5;

            case 'U':
                return sequence.size() == 9;

            case '"':
            case 'n':
            case 'a':
            case 'b':
            case 't':
            case 'r':
            case 'f':
            case '\\':
            case '0':
                return true;
        }
        return false;
    }

    void Lexer::throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col, const std::string& context)
    {
        std::vector<std::string> ctx = Utils::splitString(context, '\n');

        std::stringstream ss;
        ss << message << "\n";
        ss << makeTokenBasedErrorCtx(match, line, col, context);

        throw SyntaxError(ss.str());
    }
}
