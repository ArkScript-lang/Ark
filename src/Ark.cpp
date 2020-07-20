#include <Ark/Ark.hpp>

// check if MSVC is correctly configured for UTF8
// from: https://www.reddit.com/r/cpp/comments/75gohf/i_just_found_a_use_for_the_poop_emoji_in_c/
#ifdef _MSC_VER
    static_assert(
        (static_cast<unsigned char>("💩"[0]) == 0xF0) &&
        (static_cast<unsigned char>("💩"[1]) == 0x9F) &&
        (static_cast<unsigned char>("💩"[2]) == 0x92) &&
        (static_cast<unsigned char>("💩"[3]) == 0xA9), "Source or compiler not UTF-8 compliant! Add flag /utf-8 for Visual Studio");
#endif