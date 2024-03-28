#include <Ark/Compiler/AST/BaseParser.hpp>
#include <Ark/Exceptions.hpp>

#include <algorithm>
#include <utility>

namespace Ark::internal
{
    void BaseParser::registerNewLine(std::string::iterator it, std::size_t row)
    {
        // search for an existing new line position
        if (std::find_if(m_it_to_row.begin(), m_it_to_row.end(), [it](const auto& pair) {
                return pair.first == it;
            }) != m_it_to_row.end())
            return;

        for (std::size_t i = 0, end = m_it_to_row.size(); i < end; ++i)
        {
            auto current = m_it_to_row[i].first;
            auto next = i + 1 < end ? m_it_to_row[i + 1].first : m_str.end();
            if (current < it && it < next)
            {
                m_it_to_row.insert(m_it_to_row.begin() + i + 1, std::make_pair(it, row));
                break;
            }
        }
    }

    void BaseParser::next()
    {
        m_it = m_next_it;
        if (isEOF())
        {
            m_sym = utf8_char_t();  // reset sym to EOF
            return;
        }

        // getting a character from the stream
        auto [it, sym] = utf8_char_t::at(m_it);
        m_next_it = it;
        m_sym = sym;

        if (*m_it == '\n')
        {
            ++m_filepos.row;
            m_filepos.col = 0;
            registerNewLine(m_it, m_filepos.row);
        }
        else if (m_sym.isPrintable())
            m_filepos.col += m_sym.size();
    }

    void BaseParser::initParser(const std::string& filename, const std::string& code)
    {
        m_filename = filename;

        // if the input string is empty, raise an error
        if (code.empty())
        {
            m_sym = utf8_char_t();
            error("Expected symbol, got empty string", "");
        }
        m_str = code;

        m_it = m_next_it = m_str.begin();

        // otherwise, get the first symbol
        next();
    }

    void BaseParser::backtrack(long n)
    {
        if (static_cast<std::size_t>(n) >= m_str.size())
            return;

        m_it = m_str.begin() + n;
        auto [it, sym] = utf8_char_t::at(m_it);
        m_next_it = it;
        m_sym = sym;

        // TODO: create a kind of map vec<pair<it, row>>
        // search for the nearest it < m_it in the map to know the line number
        for (std::size_t i = 0, end = m_it_to_row.size(); i < end; ++i)
        {
            auto [at, line] = m_it_to_row[i];
            if (it < at)
            {
                m_filepos.row = line - 1;
                break;
            }
        }
        // compute the position in the line
        std::string_view view = m_str;
        auto it_pos = std::distance(m_str.begin(), m_it);
        view = view.substr(0, it_pos);
        auto nearest_newline_index = view.find_last_of('\n');
        if (nearest_newline_index != std::string_view::npos)
            m_filepos.col = it_pos - nearest_newline_index + 1;
        else
            m_filepos.col = it_pos + 1;
    }

    FilePosition BaseParser::getCursor()
    {
        return m_filepos;
    }

    void BaseParser::error(const std::string& error, std::string exp)
    {
        FilePosition pos = getCursor();
        throw CodeError(error, m_filename, pos.row, pos.col, std::move(exp), m_sym);
    }

    void BaseParser::errorWithNextToken(const std::string& message)
    {
        auto pos = getCount();
        std::string next_token;

        anyUntil(IsEither(IsInlineSpace, IsEither(IsChar('('), IsChar(')'))), &next_token);
        backtrack(pos);

        error(message, next_token);
    }

    void BaseParser::errorMissingSuffix(char suffix, const std::string& node_name)
    {
        errorWithNextToken("Missing '" + std::string(1, suffix) + "' after " + node_name);
    }

    bool BaseParser::accept(const CharPred& t, std::string* s)
    {
        if (isEOF())
            return false;

        // return false if the predicate couldn't consume the symbol
        if (!t(m_sym.codepoint()))
            return false;
        // otherwise, add it to the string and go to the next symbol
        if (s != nullptr)
            *s += m_sym.c_str();

        next();
        return true;
    }

    bool BaseParser::expect(const CharPred& t, std::string* s)
    {
        // throw an error if the predicate couldn't consume the symbol
        if (!t(m_sym.codepoint()))
            error("Expected " + t.name, m_sym.c_str());
        // otherwise, add it to the string and go to the next symbol
        if (s != nullptr)
            *s += m_sym.c_str();
        next();
        return true;
    }

    bool BaseParser::space(std::string* s)
    {
        if (accept(IsSpace))
        {
            if (s != nullptr)
                s->push_back(' ');
            // loop while there are still ' ' to consume
            while (accept(IsSpace))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::inlineSpace(std::string* s)
    {
        if (accept(IsInlineSpace))
        {
            if (s != nullptr)
                s->push_back(' ');
            // loop while there are still ' ' to consume
            while (accept(IsInlineSpace))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::endOfLine(std::string* s)
    {
        if ((accept(IsChar('\r')) || true) && accept(IsChar('\n')))
        {
            if (s != nullptr)
                s->push_back('\n');
            while ((accept(IsChar('\r')) || true) && accept(IsChar('\n')))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::comment(std::string* s)
    {
        if (accept(IsChar('#'), s))
        {
            while (accept(IsNot(IsChar('\n')), s))
                ;
            accept(IsChar('\n'), s);
            return true;
        }
        return false;
    }

    bool BaseParser::newlineOrComment(std::string* s)
    {
        bool matched = space();
        while (!isEOF() && comment(s))
        {
            space();
            matched = true;
        }

        return matched;
    }

    bool BaseParser::prefix(char c)
    {
        if (!accept(IsChar(c)))
            return false;
        return true;
    }

    bool BaseParser::suffix(char c)
    {
        return accept(IsChar(c));
    }

    bool BaseParser::number(std::string* s)
    {
        if (accept(IsDigit, s))
        {
            // consume all the digits available,
            // stop when the symbol isn't a digit anymore
            while (accept(IsDigit, s))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::signedNumber(std::string* s)
    {
        accept(IsMinus, s);
        if (!number(s))
            return false;

        // (optional) floating part
        accept(IsChar('.'), s) && number(s);
        // (optional) scientific part
        if (accept(IsEither(IsChar('e'), IsChar('E')), s))
        {
            accept(IsEither(IsMinus, IsChar('+')), s);
            number(s);
        }

        return true;
    }

    bool BaseParser::hexNumber(unsigned int length, std::string* s)
    {
        while (length != 0)
        {
            if (!accept(IsHex, s))
                return false;
            --length;
        }
        return true;
    }

    bool BaseParser::name(std::string* s)
    {
        auto alpha_symbols = IsEither(IsAlpha, IsSymbol);
        auto alnum_symbols = IsEither(IsAlnum, IsSymbol);

        if (accept(alpha_symbols, s))
        {
            while (accept(alnum_symbols, s))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::sequence(const std::string& s)
    {
        for (char i : s)
        {
            if (!accept(IsChar(i)))
                return false;
        }

        return true;
    }

    bool BaseParser::packageName(std::string* s)
    {
        if (accept(IsAlnum, s))
        {
            while (accept(IsEither(IsAlnum, IsEither(IsChar('_'), IsChar('-'))), s))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::anyUntil(const CharPred& delim, std::string* s)
    {
        if (accept(IsNot(delim), s))
        {
            while (accept(IsNot(delim), s))
                ;
            return true;
        }
        return false;
    }

    bool BaseParser::oneOf(std::initializer_list<std::string> words, std::string* s)
    {
        std::string buffer;
        if (!name(&buffer))
            return false;

        if (s)
            *s = buffer;

        for (const auto& word : words)
        {
            if (word == buffer)
                return true;
        }
        return false;
    }
}
