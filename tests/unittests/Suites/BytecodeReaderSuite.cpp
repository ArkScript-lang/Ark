#include <boost/ut.hpp>

#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Compiler/Welder.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Proxy/Picosha2.hpp>

#include <string>
#include <chrono>

#include <TestsHelper.hpp>

using namespace boost;
using namespace Ark::literals;

ut::suite<"BytecodeReader"> bcr_suite = [] {
    using namespace ut;

    "ackermann.ark"_test = [] {
        Ark::Welder welder(0, { lib_path });
        const std::string script_path = getResourcePath("BytecodeReaderSuite/ackermann.ark");

        const auto time_start =
            static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::seconds>(
                                                std::chrono::system_clock::now().time_since_epoch())
                                                .count());

        should("compile without error") = [&] {
            expect(mut(welder).computeASTFromFile(script_path));
            expect(mut(welder).generateBytecode());
        };

        const auto time_end =
            static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::seconds>(
                                                std::chrono::system_clock::now().time_since_epoch())
                                                .count());

        Ark::BytecodeReader bcr;
        const auto bytecode = welder.bytecode();
        bcr.feed(bytecode);

        should("find the version") = [bcr] {
            auto [major, minor, patch] = bcr.version();
            expect(that % major == ARK_VERSION_MAJOR);
            expect(that % minor == ARK_VERSION_MINOR);
            expect(that % patch == ARK_VERSION_PATCH);
        };

        should("find the timestamp") = [bcr, time_start, time_end] {
            const auto time = bcr.timestamp();
            expect(that % time >= time_start);
            expect(that % time <= time_end);
        };

        should("find the sha256") = [bcr, bytecode] {
            const auto sha256 = bcr.sha256();
            std::vector<unsigned char> expected_sha(picosha2::k_digest_size);
            // compute sha256 after header + sha
            picosha2::hash256(bytecode.begin() + Ark::internal::bytecode::HeaderSize + 32, bytecode.end(), expected_sha);

            expect(that % sha256 == expected_sha);
        };

        const auto symbols_block = bcr.symbols();
        const auto values_block = bcr.values(symbols_block);
        const auto filenames_block = bcr.filenames(values_block);
        const auto inst_locations_block = bcr.instLocations(filenames_block);
        const auto [pages, start_code] = bcr.code(inst_locations_block);

        should("list all symbols") = [symbols_block] {
            using namespace std::literals::string_literals;

            const auto expected_symbols = std::vector<std::string> {
                "ackermann", "n", "m"
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
                Ark::Value("Ackermann-Péter function, m=3, n=6: "),
                Ark::Value(3),
                Ark::Value(6)
            };
            expect(that % values_block.values.size() == expected_values.size());
            expect(that % values_block.start == symbols_block.end);
            expect(
                that % values_block.end == values_block.start + 1  // header size
                    + 2                                            // size of the table
                    + 6                                            // number of type tags
                    + 2                                            // page addr length
                    + 4 * 12                                       // number represented as DecomposedDouble
                    + 37                                           // string length
                    + 6                                            // null terminator
            );
        };

        should("list all filenames") = [values_block, filenames_block, script_path] {
            expect(that % values_block.end == filenames_block.start);
            expect(that % filenames_block.filenames.size() == 1_z);
            expect(that % filenames_block.filenames.front() == script_path);
        };

        should("have registered some instruction locations") = [filenames_block, inst_locations_block] {
            expect(that % filenames_block.end == inst_locations_block.start);
            expect(that % inst_locations_block.locations.size() > 1_z);
            expect(that % inst_locations_block.locations.front().page_pointer == 0_z);
            expect(that % inst_locations_block.locations.back().page_pointer == 1_z);
        };

        should("list all code page") = [inst_locations_block, pages, start_code] {
            expect(that % start_code == inst_locations_block.end);
            expect(that % pages.size() == 2ull);
            expect(that % pages[0].size() == 10 * 4ull);
            expect(that % pages[1].size() == 20 * 4ull);
        };
    };
};
