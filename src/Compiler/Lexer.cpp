#include <Ark/Compiler/Lexer.hpp>

#include <Ark/Utils.hpp>
#include <Ark/Log.hpp>

namespace Ark::internal
{
    Lexer::Lexer(unsigned debug) :
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

        auto append_token_from_buffer = [&](){
            TokenType type = guessType(buffer);
            // tokenizing error management
            if (type == TokenType::Mismatch)
                throwTokenizingError("invalid token '" + buffer + "'", buffer, line, character, code);
            else if (type == TokenType::Capture || type == TokenType::GetField)
                buffer = buffer.substr(1);  // remove the & or the .
            m_tokens.emplace_back(type, buffer, saved_line, saved_char);
            buffer.clear();
        };

        for (std::size_t pos=0; pos < code.size(); ++pos)
        {
            char current = code[pos];

            if (m_debug >= 5)
                std::cout << "buffer: " << buffer << " - "
                            << "ctrl_char: " << ctrl_char << " - "
                            << "current: '" << std::string(1, current) << "' - "
                            << "line: " << line << ", char: " << character << " - "
                            << "\n";

            // position counter
            if (current == '\n')
            {
                line++;
                character = 0;

                // close comments, don't append them
                if (in_comment)
                {
                    in_comment = false;
                    buffer.clear();
                    continue;
                }

                if (!buffer.empty())
                    append_token_from_buffer();
                continue;
            }

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
                // handle shorthands
                else if (current == '\'')
                {
                    if (!buffer.empty())
                        append_token_from_buffer();
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
                else if ((current == ' ' || current == '\t' || current == '\v'))
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
                // getfield
                else if (current == '.')
                {
                    // check numbers, we don't want to split 3.0 into 3 and .0
                    if (!buffer.empty() && !(('0' <= buffer[0] && buffer[0] <= '9') || buffer[0] == '+' || buffer[0] == '-'))
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
                                case '"' : buffer +=  '"'; break;
                                case 'n' : buffer += '\n'; break;
                                case 'a' : buffer += '\a'; break;
                                case 'b' : buffer += '\b'; break;
                                case 't' : buffer += '\t'; break;
                                case 'r' : buffer += '\r'; break;
                                case 'f' : buffer += '\f'; break;
                                case '\\': buffer += '\\'; break;
                                case '0' : buffer += '\0'; break;

                                default:
                                    throwTokenizingError("unknown control character '\\" + ctrl_char + "' in string", buffer, line, character, code);
                                    break;
                            }
                        }
                        else
                        {
                            switch (ctrl_char[0])
                            {
                                /// @todo
                                case 'x': break;
                                case 'u': break;
                                case 'U': break;

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

            // update position
            character++;
        }

        // debugging information
        if (m_debug >= 3)
        {
            auto last_token = m_tokens.back();
            for (auto& last_token : m_tokens)
            {
                std::cout << "TokenType: " << tokentype_string[static_cast<std::size_t>(last_token.type)] << "\t";
                std::cout << "Line: " << last_token.line << " \t";
                std::cout << "[" << last_token.col << "\t]\t";
                std::cout << "Token: " << last_token.token << std::endl;
            }
        }
    }

    const std::vector<Token>& Lexer::tokens()
    {
        return m_tokens;
    }
}
