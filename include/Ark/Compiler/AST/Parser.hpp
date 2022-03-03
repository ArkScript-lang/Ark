/**
 * @file Parser.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Parses a token stream into an AST by using the Ark::Node
 * @version 0.4
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef COMPILER_AST_PARSER_HPP
#define COMPILER_AST_PARSER_HPP

#include <string>
#include <list>
#include <ostream>
#include <vector>
#include <cinttypes>

#include <Ark/Constants.hpp>
#include <Ark/Compiler/AST/Lexer.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    inline NodeType similarNodetypeFromTokentype(TokenType tt)
    {
        if (tt == TokenType::Capture)
            return NodeType::Capture;
        else if (tt == TokenType::GetField)
            return NodeType::GetField;
        else if (tt == TokenType::Spread)
            return NodeType::Spread;

        return NodeType::Symbol;
    }

    /**
     * @brief The parser is responsible of constructing the Abstract Syntax Tree from a token list
     * 
     */
    class Parser
    {
    public:
        /**
         * @brief Construct a new Parser object
         * 
         * @param debug the debug level
         * @param options the parsing options
         * @param lib_env fallback library search path
         */
        Parser(unsigned debug, uint16_t options, const std::vector<std::string>& lib_env) noexcept;

        /**
         * @brief Give the code to parse
         * 
         * @param code the ArkScript code
         * @param filename the name of the file
         */
        void feed(const std::string& code, const std::string& filename = ARK_NO_NAME_FILE);

        /**
         * @brief Return the generated AST
         * 
         * @return const Node& 
         */
        const Node& ast() const noexcept;

        /**
         * @brief Return the list of files imported by the code given to the parser
         * 
         * Each path of each imported file is relative to the filename given when feeding the parser.
         * 
         * @return const std::vector<std::string>& 
         */
        const std::vector<std::string>& getImports() const noexcept;

        friend std::ostream& operator<<(std::ostream& os, const Parser& P) noexcept;

    private:
        unsigned m_debug;
        std::vector<std::string> m_libenv;
        uint16_t m_options;
        Lexer m_lexer;
        Node m_ast;
        Token m_last_token;

        // path of the current file
        std::string m_file;
        // source code of the current file
        std::string m_code;
        // the files included by the "includer" to avoid multiple includes
        std::vector<std::string> m_parent_include;

        /**
         * @brief Applying syntactic sugar: {...} => (begin...), [...] => (list ...)
         * 
         * @param tokens a list of tokens
         */
        void sugar(std::vector<Token>& tokens) noexcept;

        /**
         * @brief Parse a list of tokens recursively
         * 
         * @param tokens 
         * @param authorize_capture if we are authorized to consume TokenType::Capture tokens
         * @param authorize_field_read if we are authorized to consume TokenType::GetField tokens
         * @param in_macro if we are in a macro, there a bunch of things we can tolerate
         * @return Node 
         */
        Node parse(std::list<Token>& tokens, bool authorize_capture = false, bool authorize_field_read = false, bool in_macro = false);

        void parseIf(Node&, std::list<Token>&, bool);
        void parseLetMut(Node&, Token&, std::list<Token>&, bool);
        void parseSet(Node&, Token&, std::list<Token>&, bool);
        void parseFun(Node&, Token&, std::list<Token>&, bool);
        void parseWhile(Node&, Token&, std::list<Token>&, bool);
        void parseBegin(Node&, std::list<Token>&, bool);
        void parseImport(Node&, std::list<Token>&);
        void parseQuote(Node&, std::list<Token>&, bool);
        void parseDel(Node&, std::list<Token>&);
        Node parseShorthand(Token&, std::list<Token>&, bool);
        void checkForInvalidTokens(Node&, Token&, bool, bool, bool);

        /**
         * @brief Get the next token if possible, from a list of tokens
         * 
         * The list of tokens is modified.
         * 
         * @param tokens list of tokens to get the next token from
         * @return Token 
         */
        Token nextToken(std::list<Token>& tokens);

        /**
         * @brief Convert a token to a node
         * 
         * @param token the token to converts
         * @return Node 
         */
        Node atom(const Token& token);

        /**
         * @brief Search for all the includes in a given node, in its sub-nodes and replace them by the code of the included file
         * 
         * @param n 
         * @param parent the parent node of the current one
         * @param pos the position of the child node in the parent node list
         * @return true if we found an import and replaced it by the corresponding code
         */
        bool checkForInclude(Node& n, Node& parent, std::size_t pos = 0);

        /**
         * @brief Seek a file in the lib folder and everywhere
         * 
         * @param file 
         * @return std::string 
         */
        std::string seekFile(const std::string& file);

        /**
         * @brief Throw a parse exception is the given predicated is false
         * 
         * @param pred 
         * @param message error message to use
         * @param token concerned token
         */
        void expect(bool pred, const std::string& message, Token token);

        /**
         * @brief Throw a parse error related to a token (seek it in the related file and highlight the error)
         * 
         * @param message 
         * @param token 
         */
        [[noreturn]] void throwParseError(const std::string& message, Token token);
    };
}

#endif
