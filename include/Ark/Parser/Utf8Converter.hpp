#ifndef ark_parser_utf8converter
#define ark_parser_utf8converter

#include <string>

namespace Ark::internal
{
    std::string ws_to_utf8(const std::wstring& s);
    std::wstring utf8_to_ws(const std::string& utf8);
}

#endif