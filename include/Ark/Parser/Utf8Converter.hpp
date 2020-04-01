#ifndef ark_parser_utf8converter
#define ark_parser_utf8converter

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <string>

namespace Ark::internal
{
    /**
     * @brief Converts a wide string to a standard string
     * 
     * @param s the wide string to convert
     * @return std::string 
     */
    std::string ws_to_utf8(const std::wstring& s);

    /**
     * @brief Converts a standard string to wide string
     * 
     * @param utf8 the string to convert
     * @return std::wstring 
     */
    std::wstring utf8_to_ws(const std::string& utf8);
}

#endif