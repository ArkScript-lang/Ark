#ifndef SRC_UTF8_CHAR_HPP
#define SRC_UTF8_CHAR_HPP

#include <array>
#include <string>
#include <limits>

#undef max

namespace Ark::internal
{
    class utf8_char_t
    {
    public:
        using codepoint_t = int;
        using length_t = unsigned char;
        using repr_t = std::array<unsigned char, 5>;

        utf8_char_t() :
            m_codepoint(0), m_length(0), m_repr({ 0 }) {}

        utf8_char_t(const codepoint_t cp, const length_t len, const repr_t repr) :
            m_codepoint(cp), m_length(len), m_repr(repr) {}

        /**
         * @brief Parse a codepoint and compute its length and representation
         * @details https://github.com/sheredom/utf8.h/blob/4e4d828174c35e4564c31a9e35580c299c69a063/utf8.h#L1178
         * @param it iterator in a string
         * @param end end iterator, used to avoid going out of bound
         * @return std::pair<std::string::iterator, utf8_char_t> the iterator points to the beginning of the next codepoint, the utf8_char_t represents the parsed codepoint
         */
        static std::pair<std::string::iterator, utf8_char_t> at(const std::string::iterator it, const std::string::iterator end)
        {
            codepoint_t cp;
            length_t length;
            repr_t repr = {};

            if (0xf0 == (0xf8 & *it))  // 4 byte utf8 codepoint
            {
                if (it + 3 == end || it + 2 == end || it + 1 == end)
                    return std::make_pair(end, utf8_char_t {});

                cp = (static_cast<codepoint_t>(0x07 & *it) << 18) |
                    (static_cast<codepoint_t>(0x3f & *(it + 1)) << 12) |
                    (static_cast<codepoint_t>(0x3f & *(it + 2)) << 6) |
                    static_cast<codepoint_t>(0x3f & *(it + 3));
                length = 4;
            }
            else if (0xe0 == (0xf0 & *it))  // 3 byte utf8 codepoint
            {
                if (it + 2 == end || it + 1 == end)
                    return std::make_pair(end, utf8_char_t {});

                cp = (static_cast<codepoint_t>(0x0f & *it) << 12) |
                    (static_cast<codepoint_t>(0x3f & *(it + 1)) << 6) |
                    static_cast<codepoint_t>(0x3f & *(it + 2));
                length = 3;
            }
            else if (0xc0 == (0xe0 & *it))  // 2 byte utf8 codepoint
            {
                if (it + 1 == end)
                    return std::make_pair(end, utf8_char_t {});

                cp = (static_cast<codepoint_t>(0x1f & *it) << 6) |
                    static_cast<codepoint_t>(0x3f & *(it + 1));
                length = 2;
            }
            else  // 1 byte utf8 codepoint otherwise
            {
                cp = static_cast<unsigned char>(*it);
                length = 1;
            }

            for (length_t i = 0; i < length; ++i)
                repr[i] = static_cast<unsigned char>(*(it + static_cast<int>(i)));

            return std::make_pair(it + static_cast<long>(length),
                                  utf8_char_t(cp, length, repr));
        }

        /**
         *
         * @return true if the given codepoint is printable according to std::isprint
         */
        [[nodiscard]] bool isPrintable() const
        {
            if (m_codepoint < std::numeric_limits<char>::max())
                return std::isprint(m_codepoint);
            return true;
        }

        [[nodiscard]] const char* c_str() const { return reinterpret_cast<const char*>(m_repr.data()); }
        [[nodiscard]] std::size_t size() const { return m_length; }
        [[nodiscard]] codepoint_t codepoint() const { return m_codepoint; }

    private:
        codepoint_t m_codepoint;
        length_t m_length;
        repr_t m_repr;
    };
}

#endif
