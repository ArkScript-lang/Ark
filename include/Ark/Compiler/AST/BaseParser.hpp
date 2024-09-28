#ifndef SRC_BASEPARSER_HPP
#define SRC_BASEPARSER_HPP

#include <string>
#include <vector>
#include <initializer_list>

#include <Ark/Platform.hpp>
#include <Ark/Compiler/AST/Predicates.hpp>
#include <Ark/Compiler/AST/utf8_char.hpp>

namespace Ark::internal
{
    /**
     * @brief Describe a position in a given file ; handled by the BaseParser
     */
    struct FilePosition
    {
        std::size_t row = 0;
        std::size_t col = 0;
    };

    class ARK_API BaseParser
    {
    public:
        BaseParser() = default;

    private:
        std::string m_str;
        std::vector<std::pair<std::string::iterator, std::size_t>> m_it_to_row;  ///< A crude map of \n position to line number to speed up line number computing
        std::string::iterator m_it, m_next_it;
        utf8_char_t m_sym;       ///< The current utf8 character we're on
        FilePosition m_filepos;  ///< The position of the cursor in the file

        /**
         * @brief Register the position of a new line, with an iterator pointing to the new line and the row number
         *
         * @param it
         * @param row
         */
        void registerNewLine(std::string::iterator it, std::size_t row);

        /**
         * @brief getting next character and changing the values of count/row/col/sym
         */
        void next();

    protected:
        std::string m_filename;

        void initParser(const std::string& filename, const std::string& code);

        FilePosition getCursor() const;

        /**
         *
         * @param error an error message
         * @param exp the expression causing the error
         */
        void error(const std::string& error, std::string exp);

        /**
         * @brief Fetch the next token (space and paren delimited) to generate an error
         *
         * @param message an error message
         */
        void errorWithNextToken(const std::string& message);

        /**
         * @brief Generate an error for a given node when a suffix is missing
         *
         * @param suffix a suffix char, eg " or )
         * @param node_name can be "string", "node" ; represents a structure
         */
        void errorMissingSuffix(char suffix, const std::string& node_name);

        /**
         *
         * @return distance in characters from the beginning of the file to the cursor
         */
        long getCount() { return std::distance(m_str.begin(), m_it); }

        /**
         *
         * @return file size in bytes
         */
        std::size_t getSize() const { return m_str.size(); }

        /**
         *
         * @return true if the cursor is positioned at the end of the file
         */
        [[nodiscard]] bool isEOF() const { return m_it == m_str.end(); }

        void backtrack(long n);

        /**
         * @brief check if a Character Predicate was able to parse, call next() if matching
         *
         * @param t a char predicate to match
         * @param s optional string to append the matching chars to
         * @return true if matched
         */
        bool accept(const CharPred& t, std::string* s = nullptr);

        /**
         * @brief heck if a Character Predicate was able to parse, call next() if matching ; throw a CodeError if it doesn't match
         * @param t a char predicate to match
         * @param s optional string to append the matching chars to
         * @return true if matched
         */
        bool expect(const CharPred& t, std::string* s = nullptr);

        // basic parsers

        bool space(std::string* s = nullptr);
        bool inlineSpace(std::string* s = nullptr);
        bool comment(std::string* s = nullptr);
        bool spaceComment(std::string* s = nullptr);
        bool newlineOrComment(std::string* s = nullptr);
        bool prefix(char c);
        bool suffix(char c);
        bool number(std::string* s = nullptr);
        bool signedNumber(std::string* s = nullptr);
        bool hexNumber(unsigned length, std::string* s = nullptr);
        bool name(std::string* s = nullptr);
        bool sequence(const std::string& s);
        bool packageName(std::string* s = nullptr);

        /**
         * @brief Match any char that do not match the predicate
         *
         * @param delim delimiter predicate
         * @param s optional string to append the matching chars to
         * @return true if matched
         */
        bool anyUntil(const CharPred& delim, std::string* s = nullptr);

        /**
         * @brief Fetch a token and try to match one of the given words
         *
         * @param words list of words to match against
         * @param s optional string to append the matching chars to
         * @return true if matched
         */
        bool oneOf(std::initializer_list<std::string> words, std::string* s = nullptr);
    };
}

#endif
