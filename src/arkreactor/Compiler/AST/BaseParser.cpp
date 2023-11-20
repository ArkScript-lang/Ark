#include <Ark/Compiler/AST/BaseParser.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
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
    }

    void BaseParser::initParser(const std::string& filename, const std::string& code)
    {
        m_filename = filename;

        // if the input string is empty, raise an error
        if (code.size() == 0)
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
    }

    FilePosition BaseParser::getCursor()
    {
        FilePosition pos { 0, 0 };

        // adjust the row/col count (this is going to be VERY inefficient)
        auto tmp = m_str.begin();
        while (true)
        {
            auto [it, sym] = utf8_char_t::at(tmp);
            if (*tmp == '\n')
            {
                ++pos.row;
                pos.col = 0;
            }
            else if (sym.isPrintable())
                pos.col += sym.size();
            tmp = it;

            if (tmp > m_it || tmp == m_str.end())
                break;
        }

        return pos;
    }

    void BaseParser::error(const std::string& error, const std::string exp)
    {
        FilePosition pos = getCursor();
        throw CodeError(error, m_filename, pos.row, pos.col, exp, m_sym);
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

    bool BaseParser::comment()
    {
        if (accept(IsChar('#')))
        {
            while (accept(IsNot(IsChar('\n'))))
                ;
            accept(IsChar('\n'));
            return true;
        }
        return false;
    }

    bool BaseParser::newlineOrComment()
    {
        bool matched = space();
        while (!isEOF() && comment())
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
        newlineOrComment();
        return true;
    }

    bool BaseParser::suffix(char c)
    {
        newlineOrComment();
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
        for (std::size_t i = 0, end = s.size(); i < end; ++i)
        {
            if (!accept(IsChar(s[i])))
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

        for (auto word : words)
        {
            if (word == buffer)
                return true;
        }
        return false;
    }
}
