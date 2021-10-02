/**
 * @file Lexer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Tokenize ArkScript code
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef ARK_COMPILER_LEXER_HPP
#define ARK_COMPILER_LEXER_HPP

#include <vector>

#include <Ark/Compiler/AST/Token.hpp>
#include <Ark/Compiler/Common.hpp>

namespace Ark::internal
{
    /**
     * @brief The lexer, in charge of creating a list of tokens
     *
     */
    class Lexer
    {
    public:
        /**
         * @brief Construct a new Lexer object
         *
         * @param debug the debug level
         */
        explicit Lexer(unsigned debug) noexcept;

        /**
         * @brief Give code to tokenize and create the list of tokens
         *
         * @param code the ArkScript code
         */
        void feed(const std::string& code);

        /**
         * @brief Return the list of tokens
         *
         * @return std::vector<Token>&
         */
        std::vector<Token>& tokens() noexcept;

    private:
        unsigned m_debug;
        std::vector<Token> m_tokens;

        inline constexpr bool isHexChar(char chr)
        {
            return (('a' <= chr && chr <= 'f') || ('A' <= chr && chr <= 'F') || ('0' <= chr && chr <= '9'));
        }

        /**
         * @brief Helper function to determine the type of a token
         *
         * @param value
         * @return TokenType
         */
        TokenType guessType(const std::string& value) noexcept;

        /**
         * @brief Check if the value is a keyword in ArkScript
         *
         * @param value
         * @return true
         * @return false
         */
        bool isKeyword(const std::string& value) noexcept;
        /**
         * @brief Check if the value can be an identifier in ArkScript
         *
         * @param value
         * @return true
         * @return false
         */
        bool isIdentifier(const std::string& value) noexcept;

        /**
         * @brief Check if the value is an operator in ArkScript
         *
         * @param value
         * @return true
         * @return false
         */
        bool isOperator(const std::string& value) noexcept;

        /**
         * @brief Check if a control character / sequence is complete or not
         *
         * @param sequence the sequence without the leading \\
         * @param next the next character to come, maybe, in the sequence
         * @return true
         * @return false
         */
        bool endOfControlChar(const std::string& sequence, char next) noexcept;

        /**
         * @brief To throw nice lexer errors
         *
         * @param message
         * @param match
         * @param line
         * @param col
         * @param context
         */
        void throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col, const std::string& context);
    };
}

#endif
