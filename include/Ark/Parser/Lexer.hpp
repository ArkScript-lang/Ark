#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>

#include <Ark/Utils.hpp>

namespace Ark
{
    namespace Parser
    {
        class Lexer
        {
        public:
            Lexer();
            ~Lexer();

            void feed(const std::string& code);
            bool check();

            const std::vector<std::string>& tokens();

        private:
            std::vector<std::string> m_tokens;
            const std::vector<std::regex> m_regexs = {
                std::regex("^\"[^\"]*\"")  // strings
                , std::regex("^[\\(\\)\\[\\]{}]")  // parenthesis
                , std::regex("^((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?")  // numbers
                , std::regex("^(\\+|-|\\*|/|%|<=|>=|!=|<|>|=|\\^)")  // operators
                , std::regex("^[_!?\\-\\w]+")  // words
                , std::regex("^\\s+")  // whitespaces
                , std::regex("^'.*")  // comments
            };
        };
    }
}

#endif  // ark_lexer
