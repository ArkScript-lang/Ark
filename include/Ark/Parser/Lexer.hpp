#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>

#include <Ark/Utils.hpp>

namespace Ark
{
    namespace Parser
    {
        struct Token
        {
            std::string token;
            std::size_t line;
            std::size_t col;

            Token(const std::string& tok, std::size_t line, std::size_t col) :
                token(tok), line(line), col(col)
            {}
        };

        class Lexer
        {
        public:
            Lexer(bool debug=false);

            void feed(const std::string& code);
            bool check();

            const std::vector<Token>& tokens();

        private:
            bool m_debug;
            std::vector<Token> m_tokens;
            const std::vector<std::regex> m_regexs = {
                std::regex("^\"[^\"]*\"")  // strings
                , std::regex("^[\\(\\)\\[\\]\\{\\}]")  // parenthesis
                , std::regex("^((\\+|-)?[[:digit:]]+)([\\.|/](([[:digit:]]+)?))?")  // numbers
                , std::regex("^(\\+|-|\\*|/|<=|>=|!=|<|>|=|\\^|@)")  // operators
                , std::regex("^[a-zA-Z_][a-zA-Z0-9_\\-!?']*")  // words
                , std::regex("^\\s+")  // whitespaces
                , std::regex("^'.*")  // comments
                , std::regex("^`")  // quote
            };
        };
    }
}

#endif  // ark_lexer
