#ifndef UTF8_DECODER_H
#define UTF8_DECODER_H

#include <array>
#include <cstdint>

namespace utf8
{
    enum class Utf8Type
    {
        Ascii = 0,
        LatinExtra = 1,
        BasicMultiLingual = 2,
        OthersPlanesUnicode = 3,
        OutRange = 4
    };

    namespace details
    {
        // clang-format off
        constexpr char no = static_cast<char>(-1);
        constexpr std::array<char, 128> ASCIIHexToInt =
            {
                no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
                no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
                no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
                 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, no, no, no, no, no, no,
                no, 10, 11, 12, 13, 14, 15, no, no, no, no, no, no, no, no, no,
                no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
                no, 10, 11, 12, 13, 14, 15, no, no, no, no, no, no, no, no, no,
                no, no, no, no, no, no, no, no, no, no, no, no, no, no, no, no,
            };
        // clang-format on
    }

    inline Utf8Type utf8type(const char* input, int32_t* out = nullptr)
    {
        int32_t codepoint_ = 0;
        int shift = 0;

        for (const char* s = input; *s != 0; ++s)
        {
            codepoint_ = ((codepoint_ << shift) | details::ASCIIHexToInt[*s]);
            shift = 4;
        }

        if (out != nullptr)
            *out = codepoint_;

        if (codepoint_ >= 0x0000 && codepoint_ <= 0x007f)
            return Utf8Type::Ascii;
        if (codepoint_ > 0x007f && codepoint_ <= 0x07ff)
            return Utf8Type::LatinExtra;
        if (codepoint_ > 0x07ff && codepoint_ <= 0xffff)
            return Utf8Type::BasicMultiLingual;
        if (codepoint_ > 0xffff && codepoint_ <= 0x10ffff)
            return Utf8Type::OthersPlanesUnicode;

        return Utf8Type::OutRange;
    }

    /**
     * @brief Convert hex string to utf8 string
     * @param input
     * @param dest Output utf8 string (size [2,5]). Empty (\0) if input is invalid or out of range
     */
    inline void decode(const char* input, char* dest)
    {
        int32_t cdp = 0;
        const Utf8Type type = utf8type(input, &cdp);
        const char c0 = details::ASCIIHexToInt[input[0]];
        const char c1 = details::ASCIIHexToInt[input[1]];
        const char c2 = details::ASCIIHexToInt[input[2]];
        const char c3 = details::ASCIIHexToInt[input[3]];

        switch (type)
        {
            case Utf8Type::Ascii:
            {
                dest[0] = static_cast<char>(cdp);
                dest[1] = 0;
                break;
            }

            case Utf8Type::LatinExtra:
            {
                dest[0] = (0xc0 | ((c1 & 0x7) << 2)) | ((c2 & 0xc) >> 2);
                dest[1] = (0x80 | ((c2 & 0x3) << 4)) | c3;
                dest[2] = 0;
                break;
            }

            case Utf8Type::BasicMultiLingual:
            {
                dest[0] = 0xe0 | c0;
                dest[1] = (0x80 | (c1 << 2)) | ((c2 & 0xc) >> 2);
                dest[2] = (0x80 | ((c2 & 0x3) << 4)) | c3;
                dest[3] = 0;
                break;
            }

            case Utf8Type::OthersPlanesUnicode:
            {
                const char c4 = details::ASCIIHexToInt[input[4]];

                if (cdp <= 0xfffff)
                {
                    dest[0] = 0xf0 | ((c0 & 0xc) >> 2);
                    dest[1] = (0x80 | ((c0 & 0x3) << 4)) | c1;
                    dest[2] = (0x80 | (c2 << 2)) | ((c3 & 0xc) >> 2);
                    dest[3] = (0x80 | ((c3 & 0x3) << 4)) | c4;
                    dest[4] = 0;
                }
                else
                {
                    const char c5 = details::ASCIIHexToInt[input[5]];

                    dest[0] = (0xf0 | ((c0 & 0x1) << 2)) | ((c1 & 0xc) >> 2);
                    dest[1] = ((0x80 | ((c1 & 0x3) << 4)) | ((c1 & 0xc) >> 2)) | c2;
                    dest[2] = (0x80 | (c3 << 2)) | ((c4 & 0xc) >> 2);
                    dest[3] = (0x80 | ((c4 & 0x3) << 4)) | c5;
                    dest[4] = 0;
                }
                break;
            }

            case Utf8Type::OutRange:
                *dest = 0;
                break;
        }
    }

    /**
     * @brief Check the validity of a given string in UTF8
     * @param str
     * @return true if the given string is a valid UTF88 string
     */
    inline bool isValid(const char* str)
    {
        const char* s = str;

        if (str == nullptr)
            return false;

        while (*s != 0)
        {
            if (0xf0 == (0xf8 & *s))
            {
                if ((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])) || (0x80 != (0xc0 & s[3])))
                    return false;
                if (0x80 == (0xc0 & s[4]))
                    return false;
                if ((0 == (0x07 & s[0])) && (0 == (0x30 & s[1])))
                    return false;
                s += 4;
            }
            else if (0xe0 == (0xf0 & *s))
            {
                if ((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])))
                    return false;
                if (0x80 == (0xc0 & s[3]))
                    return false;
                if ((0 == (0x0f & s[0])) && (0 == (0x20 & s[1])))
                    return false;
                s += 3;
            }
            else if (0xc0 == (0xe0 & *s))
            {
                if (0x80 != (0xc0 & s[1]))
                    return false;
                if (0x80 == (0xc0 & s[2]))
                    return false;
                if (0 == (0x1e & s[0]))
                    return false;
                s += 2;
            }
            else if (0x00 == (0x80 & *s))
                s += 1;
            else
                return false;
        }

        return true;
    }

    /**
     * @brief Compute the UTF8 codepoint for a given UTF8 char
     * @param str
     * @return UTF8 codepoint if valid, -1 otherwise
     */
    inline int32_t codepoint(const char* str)
    {
        int32_t codepoint = 0;
        const char* s = str;

        if (isValid(str))
        {
            while (*s != 0)
            {
                if (0xf0 == (0xf8 & *s))
                {
                    codepoint = ((0x07 & s[0]) << 18) | ((0x3f & s[1]) << 12) | ((0x3f & s[2]) << 6) | (0x3f & s[3]);
                    s += 4;
                }
                else if (0xe0 == (0xf0 & *s))
                {
                    codepoint = ((0x0f & s[0]) << 12) | ((0x3f & s[1]) << 6) | (0x3f & s[2]);
                    s += 3;
                }
                else if (0xc0 == (0xe0 & *s))
                {
                    codepoint = ((0x1f & s[0]) << 6) | (0x3f & s[1]);
                    s += 2;
                }
                else if (0x00 == (0x80 & *s))
                {
                    codepoint = s[0];
                    ++s;
                }
                else
                    return -1;
            }
        }

        return codepoint;
    }

    /**
     * @brief Generate an UTF8 character from a given codepoint
     * @param codepoint
     * @param dest Output utf8 string (size [2,5]). Empty (\0) if input is invalid or out of range
     */
    inline void codepointToUtf8(const int32_t codepoint, char* dest)
    {
        if (codepoint >= 0x0000 && codepoint <= 0x007f)
        {
            dest[0] = codepoint;
            dest[1] = 0;
        }
        else if (codepoint > 0x007f && codepoint <= 0x07ff)
        {
            dest[0] = 0x80;
            if (codepoint > 0xff)
                dest[0] |= (codepoint >> 6);
            dest[0] |= ((codepoint & 0xc0) >> 6);
            dest[1] = 0x80 | (codepoint & 0x3f);
            dest[2] = 0;
        }
        else if (codepoint > 0x07ff && codepoint <= 0xffff)
        {
            dest[0] = 0xe0;
            if (codepoint > 0xfff)
                dest[0] |= ((codepoint & 0xf000) >> 12);
            dest[1] = (0x80 | ((codepoint & 0xf00) >> 6)) | ((codepoint & 0xf0) >> 6);
            dest[2] = (0x80 | (codepoint & 0x30)) | (codepoint & 0xf);
            dest[3] = 0;
        }
        else if (codepoint > 0xffff && codepoint <= 0x10ffff)
        {
            dest[0] = 0xf0;
            if (codepoint > 0xfffff)
                dest[0] |= ((codepoint & 0x100000) >> 18);
            dest[0] |= ((codepoint & 0xc0000) >> 18);
            dest[1] = (0x80 | ((codepoint & 0x30000) >> 12)) | ((codepoint & 0xf000) >> 12);
            dest[2] = (0x80 | ((codepoint & 0xf00) >> 6)) | ((codepoint & 0xc0) >> 6);
            dest[3] = (0x80 | (codepoint & 0x30)) | (codepoint & 0xf);
            dest[4] = 0;
        }
        else
            *dest = 0;
    }
}

#endif
