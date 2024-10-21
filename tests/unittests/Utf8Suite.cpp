#include <boost/ut.hpp>

#include <utf8.hpp>
#include <array>

using namespace boost;

bool is_equal(const std::array<char, 5>& left, const std::array<unsigned, 5>& right)
{
    return static_cast<unsigned char>(left[0]) == right[0] &&
        static_cast<unsigned char>(left[1]) == right[1] &&
        static_cast<unsigned char>(left[2]) == right[2] &&
        static_cast<unsigned char>(left[3]) == right[3] &&
        static_cast<unsigned char>(left[4]) == right[4];
}

ut::suite<"Utf8"> utf8_suite = [] {
    using namespace ut;

    "[utf8::decode]"_test = [] {
        std::array<char, 5> utf8_str { 0, 0, 0, 0, 0 };

        utf8::decode("61\0\0\0", utf8_str.data());
        expect(is_equal(utf8_str, { 0x61, 0, 0, 0, 0 }));

        utf8::decode("400\0\0", utf8_str.data());
        expect(is_equal(utf8_str, { 0xC0, 0xFF, 0, 0, 0 }));

        utf8::decode("5500\0", utf8_str.data());
        expect(is_equal(utf8_str, { 0xE5, 0x94, 0x80, 0, 0 }));

        utf8::decode("67891", utf8_str.data());
        expect(is_equal(utf8_str, { 0xF1, 0xA7, 0xA2, 0x91, 0 }));
    };

    "[utf8::codepoint]"_test = [] {
        expect(that % utf8::codepoint("a") == 97);
    };
};
