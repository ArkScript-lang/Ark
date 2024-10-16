#ifndef SRC_PREDICATES_HPP
#define SRC_PREDICATES_HPP

#include <string>
#include <cctype>

#include <Ark/Compiler/AST/utf8_char.hpp>

namespace Ark::internal
{
    struct CharPred
    {
        const std::string name;

        explicit CharPred(std::string n) :
            name(std::move(n)) {}
        virtual ~CharPred() = default;

        virtual bool operator()(utf8_char_t::codepoint_t c) const = 0;
    };

    inline struct IsSpace final : CharPred
    {
        IsSpace() :
            CharPred("space") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && std::isspace(c) != 0;
        }
    } IsSpace;

    inline struct IsInlineSpace final : CharPred
    {
        IsInlineSpace() :
            CharPred("inline space") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && (std::isspace(c) != 0) && (c != '\n') && (c != '\r');
        }
    } IsInlineSpace;

    inline struct IsDigit final : CharPred
    {
        IsDigit() :
            CharPred("digit") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && std::isdigit(c) != 0;
        }
    } IsDigit;

    inline struct IsHex final : CharPred
    {
        IsHex() :
            CharPred("hex") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && std::isxdigit(c) != 0;
        }
    } IsHex;

    inline struct IsAlpha final : CharPred
    {
        IsAlpha() :
            CharPred("alphabetic") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && std::isalpha(c) != 0;
        }
    } IsAlpha;

    inline struct IsAlnum final : CharPred
    {
        IsAlnum() :
            CharPred("alphanumeric") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return 0 <= c && c <= 255 && std::isalnum(c) != 0;
        }
    } IsAlnum;

    struct IsChar final : CharPred
    {
        explicit IsChar(const char c) :
            CharPred("'" + std::string(1, c) + "'"), m_k(c)
        {}
        explicit IsChar(const utf8_char_t& c) :
            CharPred(std::string(c.c_str())), m_k(c.codepoint())
        {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return m_k == c;
        }

    private:
        const utf8_char_t::codepoint_t m_k;
    };

    struct IsEither final : CharPred
    {
        explicit IsEither(const CharPred& a, const CharPred& b) :
            CharPred("(" + a.name + " | " + b.name + ")"), m_a(a), m_b(b)
        {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return m_a(c) || m_b(c);
        }

    private:
        const CharPred& m_a;
        const CharPred& m_b;
    };

    struct IsNot final : CharPred
    {
        explicit IsNot(const CharPred& a) :
            CharPred("~" + a.name), m_a(a)
        {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            return !m_a(c);
        }

    private:
        const CharPred& m_a;
    };

    inline struct IsSymbol final : CharPred
    {
        IsSymbol() :
            CharPred("sym") {}
        bool operator()(const utf8_char_t::codepoint_t c) const override
        {
            switch (c)
            {
                case ':':
                case '!':
                case '?':
                case '@':
                case '_':
                case '-':
                case '+':
                case '*':
                case '/':
                case '|':
                case '=':
                case '<':
                case '>':
                case '%':
                case '$':
                    return true;

                default:
                    return false;
            }
        }
    } IsSymbol;

    const IsChar IsMinus('-');
}

#endif
