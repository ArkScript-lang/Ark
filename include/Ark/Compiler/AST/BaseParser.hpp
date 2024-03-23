#ifndef SRC_BASEPARSER_HPP
#define SRC_BASEPARSER_HPP

#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <initializer_list>

#include <Ark/Platform.hpp>
#include <Ark/Compiler/AST/Predicates.hpp>
#include <Ark/Compiler/AST/utf8_char.hpp>

namespace Ark::internal
{
    struct FilePosition
    {
        std::size_t row;
        std::size_t col;
    };

    class ARK_API BaseParser
    {
    public:
        BaseParser() = default;

    private:
        bool m_remove_comments;
        std::string m_str;
        std::string::iterator m_it, m_next_it;
        utf8_char_t m_sym;

        /*
            getting next character and changing the values of count/row/col/sym
        */
        void next();

    protected:
        std::string m_filename;

        void initParser(const std::string& filename, const std::string& code);

        FilePosition getCursor();

        void error(const std::string& error, std::string exp);
        void errorWithNextToken(const std::string& message);
        void errorMissingSuffix(char suffix, const std::string& node_name);

        inline long getCount() { return std::distance(m_str.begin(), m_it); }
        inline std::size_t getSize() { return m_str.size(); }
        inline bool isEOF() { return m_it == m_str.end(); }

        void backtrack(long n);

        /*
            Function to use and check if a Character Predicate was able to parse
            the current symbol.
            Add the symbol to the given string (if there was one) and call next()
        */
        bool accept(const CharPred& t, std::string* s = nullptr);

        /*
            Function to use and check if a Character Predicate was able to parse
            the current Symbol.
            Add the symbol to the given string (if there was one) and call next().
            Throw a CodeError if it couldn't.
        */
        bool expect(const CharPred& t, std::string* s = nullptr);

        // basic parsers
        bool space(std::string* s = nullptr);
        bool inlineSpace(std::string* s = nullptr);
        bool endOfLine(std::string* s = nullptr);
        bool comment();
        bool newlineOrComment();
        bool prefix(char c);
        bool suffix(char c);
        bool number(std::string* s = nullptr);
        bool signedNumber(std::string* s = nullptr);
        bool hexNumber(unsigned length, std::string* s = nullptr);
        bool name(std::string* s = nullptr);
        bool sequence(const std::string& s);
        bool packageName(std::string* s = nullptr);
        bool anyUntil(const CharPred& delim, std::string* s = nullptr);

        bool oneOf(std::initializer_list<std::string> words, std::string* s = nullptr);
    };
}

#endif
