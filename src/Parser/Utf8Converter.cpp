#include <Ark/Parser/Utf8Converter.hpp>

#include <locale>
#include <codecvt>
#include <exception>
#include <stdexcept>

namespace Ark::internal
{
    std::string ws_to_utf8(const std::wstring& s)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::string narrow = converter.to_bytes(s);
        return narrow;
    }

    std::wstring utf8_to_ws(const std::string& utf8)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(utf8);
        return wide;
    }
}