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
        };

        class Lexer
        {
        public:
            Lexer(bool debug=false);
            ~Lexer();

            void feed(const std::string& code);
            bool check();

            const std::vector<Token>& tokens();

        private:
            bool m_debug;
            std::vector<Token> m_tokens;
            const std::vector<std::regex> m_regexs = {
                std::regex("^\"[^\"]*\"")  // strings
                , std::regex("^[\\(\\)\\[\\]{}]")  // parenthesis
                , std::regex("^((\\+|-)?[[:digit:]]+)(/(([[:digit:]]+)?))?")  // numbers
                , std::regex("^(\\+|-|\\*|/|%|<=|>=|!=|<|>|=|\\^)")  // operators
                , std::regex("^[a-zA-Z_][a-zA-Z0-9_\\-!?']*")  // words
                , std::regex("^\\s+")  // whitespaces
                , std::regex("^'.*")  // comments
            };
        };
    }
}

#endif  // ark_lexer
