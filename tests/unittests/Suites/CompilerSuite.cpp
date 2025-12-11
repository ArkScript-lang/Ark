#include <boost/ut.hpp>

#include <Ark/Compiler/Welder.hpp>
#include <Ark/Compiler/IntermediateRepresentation/Word.hpp>
#include <Ark/Compiler/Serialization/IEEE754Serializer.hpp>
#include <Ark/Compiler/Serialization/IntegerSerializer.hpp>

#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Compiler"> compiler_suite = [] {
    using namespace ut;

    const std::vector<double> nums = { 0.11, 0.000000000011, 2, -2, 12, 6, 4, 0, 14657892.35, 3.141592653589, 4092.7984 };

    "IEEE754 serialization"_test = [&] {
        using namespace Ark::internal::ieee754;

        for (double original : nums)
        {
            const auto decomp = serialize(original);
            auto recomp = deserialize(decomp);
            expect(that % recomp == original);
        }
    };

    "IEEE754 serialization via integer serialization Little Endian"_test = [&] {
        using namespace Ark::internal;

        for (const double original : nums)
        {
            std::vector<uint8_t> bytecode {};

            const auto [exponent, mantissa] = ieee754::serialize(original);
            serializeToVecLE(exponent, bytecode);
            serializeToVecLE(mantissa, bytecode);

            ieee754::DecomposedDouble d { 0, 0 };
            d.exponent = deserializeLE<decltype(ieee754::DecomposedDouble::exponent)>(bytecode.begin(), bytecode.end());
            d.mantissa = deserializeLE<decltype(ieee754::DecomposedDouble::mantissa)>(
                bytecode.begin() + static_cast<std::vector<uint8_t>::difference_type>(sizeof(decltype(ieee754::DecomposedDouble::exponent))),
                bytecode.end());

            double val = ieee754::deserialize(d);

            expect(that % val == original);
        }
    };

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

        iterTestFiles(
            "CompilerSuite/ir",
            [](const TestData& data) {
                Ark::Welder welder(0, { lib_path }, features);

                should("compile without error ir/" + data.stem) = [&] {
                    expect(mut(welder).computeASTFromFile(data.path));
                    expect(mut(welder).generateBytecode());
                };

                should("output expected IR for " + data.stem) = [&] {
                    std::string ir = welder.textualIR();

                    Ark::Utils::ltrim(Ark::Utils::rtrim(ir));
                    expectOrDiff(data.expected, ir);
                };
            });
    };

    "IR generation and optimization"_test = [] {
        constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

        iterTestFiles(
            "CompilerSuite/optimized_ir",
            [](const TestData& data) {
                Ark::Welder welder(0, { lib_path }, features);

                should("compile without error optimized_ir/" + data.stem) = [&] {
                    expect(mut(welder).computeASTFromFile(data.path));
                    expect(mut(welder).generateBytecode());
                };

                should("output expected optimized IR for " + data.stem) = [&] {
                    std::string ir = welder.textualIR();

                    Ark::Utils::ltrim(Ark::Utils::rtrim(ir));
                    expectOrDiff(data.expected, ir);
                };
            });
    };
};
