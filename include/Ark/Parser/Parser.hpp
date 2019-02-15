#ifndef ark_parser
#define ark_parser

#include <string>

#include <Ark/Parser/Lexer.hpp>

namespace Ark
{
    namespace Parser
    {
        class Parser
        {
        public:
            Parser();
            ~Parser();

            void feed(const std::string& code);

        private:
            Lexer m_lexer;
        };
    }
}

#endif  // ark_parser