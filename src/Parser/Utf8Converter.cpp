#include <Ark/Parser/Utf8Converter.hpp>

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <locale>
#include <codecvt>
#include <exception>
#include <stdexcept>

namespace Ark::internal
{
    /*
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string narrow = converter.to_bytes(wide_utf16_source_string);
        std::wstring wide = converter.from_bytes(narrow_utf8_source_string);
    */

    std::string ws_to_utf8(const std::wstring& s)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string narrow = converter.to_bytes(s);
        if (converter.converted() < narrow.size())
            throw std::runtime_error("incomplete conversion from wide string to utf8 string");
        return narrow;
    }

    std::wstring utf8_to_ws(const std::string& utf8)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(utf8);
        if (converter.converted() < wide.size())
            throw std::runtime_error("incomplete conversion from utf8 string to wide string");
        return wide;
    }
}