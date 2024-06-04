#include <boost/ut.hpp>

#include <Ark/Compiler/BytecodeReader.hpp>
#include <string>

using namespace boost;

ut::suite<"BytecodeReader"> bcr_suite = [] {
    using namespace ut;

    Ark::BytecodeReader bcr;
    bcr.feed(ARK_TESTS_ROOT "tests/unittests/resources/BytecodeReaderSuite/ackermann.arkc");

    "bytecode"_test = [bcr] {
        should("find the version") = [bcr] {
            auto [major, minor, patch] = bcr.version();
            expect(that % major == 4);
            expect(that % minor == 0);
            expect(that % patch == 0);
        };

        should("find the timestamp") = [bcr] {
            const auto time = bcr.timestamp();
            expect(that % time == 1717523961ull);
        };

        should("find the sha256") = [bcr] {
            const auto sha256 = bcr.sha256();
            const auto expected_sha = std::vector<unsigned char> {
                0xcf, 0x79, 0x82, 0x6b, 0x81, 0x5c, 0xe4, 0x11,
                0xce, 0x25, 0xbe, 0xc3, 0x05, 0x91, 0x21, 0x7f,
                0x6c, 0x70, 0x54, 0x70, 0xd8, 0x8b, 0x2b, 0x90,
                0x82, 0xcd, 0x70, 0x2e, 0xeb, 0x51, 0xb2, 0x75
            };
            expect(that % sha256 == expected_sha);
        };

        const auto [pages, start_code] = bcr.code();
        const auto values_block = bcr.values();
        const auto symbols_block = bcr.symbols();

        should("list all symbols") = [symbols_block] {
            using namespace std::literals::string_literals;

            const auto expected_symbols = std::vector<std::string> {
                "ackermann", "m", "n"
            };
            expect(that % symbols_block.symbols == expected_symbols);
            // 'ark\0' + version (2 bytes per number) + timestamp + sha -> first byte of the sym table
            expect(that % symbols_block.start == 4 + 6 + 8 + 32ull);
            // 50 = 4 + 6 + 8 + 32
            // + 1 for the header
            // + 2 because we need to count the size of the table (uint16)
            // + 3 because we need to count the \0
            expect(that % symbols_block.end == 50 + 1 + 2 + "ackermann"s.size() + "m"s.size() + "n"s.size() + 3);
        };

        should("list all values") = [symbols_block, values_block] {
            const auto expected_values = std::vector<Ark::Value> {
                Ark::Value(static_cast<uint16_t>(1)),
                Ark::Value(0),
                Ark::Value(1),
                Ark::Value(7),
                Ark::Value(3)
            };
            expect(that % values_block.values.size() == expected_values.size());
            expect(that % values_block.start == symbols_block.end);
            // + 1 for the header
            // + 2 for the size
            // + 5 for the type tags
            // + 2 for the pageaddr
            // + 4*8 for the numbers represented as strings on 8 chars
            // + 5 for the \0 at the end of each value
            expect(that % values_block.end == values_block.start + 1 + 2 + 5 + 2 + 4 * 8 + 5);
        };

        should("list all code page") = [values_block, pages, start_code] {
            expect(that % start_code == values_block.end);
            expect(that % pages.size() == 2ull);
            // 7 instructions on 4 bytes
            expect(that % pages[0].size() == 7 * 4ull);
            // 32 instructions on 4 bytes
            expect(that % pages[1].size() == 32 * 4ull);
        };
    };
};
