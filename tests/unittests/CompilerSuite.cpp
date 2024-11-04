#include <boost/ut.hpp>

#include <Ark/Compiler/Welder.hpp>
#include <Ark/Compiler/Word.hpp>

#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Compiler"> compiler_suite = [] {
    using namespace ut;

    "Word construction"_test = [] {
        should("create a word with a single argument on 2 bytes") = [] {
            const auto word = Ark::internal::Word(12, 0x5678);
            expect(that % word.opcode == 12);
            expect(that % word.byte_1 == 0);
            expect(that % word.byte_2 == 0x56);
            expect(that % word.byte_3 == 0x78);
        };

        constexpr uint16_t primary_arg = 0x0567;
        constexpr uint16_t secondary_arg = 0x089a;
        const auto word = Ark::internal::Word(12, primary_arg, secondary_arg);
        should("split arguments evenly between 3 bytes") = [&] {
            expect(that % word.opcode == 12);
            expect(that % word.byte_1 == 0x89);
            expect(that % word.byte_2 == 0xa5);
            expect(that % word.byte_3 == 0x67);
        };

        should("be able to unpack both arguments from word") = [&] {
            const uint8_t padding = word.byte_1;
            const auto arg = static_cast<uint16_t>((word.byte_2 << 8) | word.byte_3);

            expect(that % primary_arg == (arg & 0x0fff));
            expect(that % secondary_arg == ((padding << 4) | (arg & 0xf000) >> 12));
        };
    };

    "IR generation"_test = [] {
        constexpr uint16_t features = Ark::FeatureImportSolver | Ark::FeatureMacroProcessor | Ark::FeatureASTOptimizer | Ark::FeatureNameResolver | Ark::FeatureTestFailOnException;

        iter_test_files(
            "CompilerSuite/ir",
            [](TestData&& data) {
                Ark::Welder welder(0, { std::filesystem::path(ARK_TESTS_ROOT "/lib/") }, features);

                should("compile without error ir/" + data.stem) = [&] {
                    expect(mut(welder).computeASTFromFile(data.path));
                    expect(mut(welder).generateBytecode());
                };

                should("output expected IR for " + data.stem) = [&] {
                    std::string ir = welder.textualIR();

                    ltrim(rtrim(ir));
                    expect(that % ir == data.expected);
                };
            });
    };

    "IR generation and optimization"_test = [] {
        constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

        iter_test_files(
            "CompilerSuite/optimized_ir",
            [](TestData&& data) {
                Ark::Welder welder(0, { std::filesystem::path(ARK_TESTS_ROOT "/lib/") }, features);

                should("compile without error ir/" + data.stem) = [&] {
                    expect(mut(welder).computeASTFromFile(data.path));
                    expect(mut(welder).generateBytecode());
                };

                should("output expected IR for " + data.stem) = [&] {
                    std::string ir = welder.textualIR();

                    ltrim(rtrim(ir));
                    expect(that % ir == data.expected);
                };
            });
    };
};
