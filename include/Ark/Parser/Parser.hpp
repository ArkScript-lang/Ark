#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>
#include <iostream>
#include <vector>
#include <utility>
#include <cinttypes>

#include <Ark/Parser/Lexer.hpp>
#include <Ark/Parser/Node.hpp>

namespace Ark
{
    class Parser
    {
    public:
        Parser(unsigned debug, const std::string& lib_dir, uint16_t options);

        void feed(const std::string& code, const std::string& filename="FILE");
        // the generated Abstract Syntax Tree
        const internal::Node& ast() const;
        // returns the list of files imported by the code given to the parser
        // each path of each imported file is relative to the filename given when feeding the parser
        const std::vector<std::string>& getImports();

        friend std::ostream& operator<<(std::ostream& os, const Parser& P);

    private:
        unsigned m_debug;
        std::string m_libdir;
        uint16_t m_options;
        internal::Lexer m_lexer;
        internal::Node m_ast;
        internal::Token m_last_token;

        // source code of the current file
        std::string m_file;
        // the files included by the "includer" to avoid multiple includes
        std::vector<std::string> m_parent_include;
        // list of warning to avoid sending a warning twice
        std::vector<std::pair<std::size_t, std::size_t>> m_warns;

        // applying syntactic sugar: {...} => (begin...), [...] => (list ...)
        void sugar(std::vector<internal::Token>& tokens);

        internal::Node parse(std::list<internal::Token>& tokens, bool authorize_capture=false, bool authorize_field_read=false);
        internal::Token nextToken(std::list<internal::Token>& tokens);
        internal::Node atom(const internal::Token& token);

        bool checkForInclude(internal::Node& n);

        // error management functions
        inline void except(bool pred, const std::string& message, internal::Token token);
        inline void throwParseError(const std::string& message, internal::Token token);
        inline void throwParseError_(const std::string& message);
    };

    #include "Parser.inl"
}

#endif  // ark_parser
