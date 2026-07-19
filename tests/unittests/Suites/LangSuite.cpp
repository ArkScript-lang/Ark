#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

void testResource(Ark::State& state, const bool is_res, const std::string& path, const uint16_t features)
{
    using namespace ut;

    try
    {
        const bool ok = mut(state).doFile(is_res ? getResourcePath(path) : path, features);
        expect(ok) << fatal << "compilation failed";
    }
    catch (const Ark::CodeError&)
    {
        expect(false) << fatal << "encountered an exception while compiling";
    }

    Ark::VM vm(state);
    should("return exit code 0") = [&] {
        expect(mut(vm).run() == 0_i);
    };

    should("have no failures") = [&] {
        const auto failure_count = mut(vm).operator[]("failure_count");
        expect(failure_count.valueType() == Ark::ValueType::Number &&
               failure_count.number() == 0.0)
            << "failure_count = 0\n";
    };
}

ut::suite<"Lang"> lang_suite = [] {
    using namespace ut;

    constexpr uint16_t features_no_inline = Ark::FeatureImportSolver | Ark::FeatureMacroProcessor | Ark::FeatureIROptimiser | Ark::FeatureNameResolver;
    constexpr uint16_t features_inline = Ark::FeatureIRInliner | Ark::FeatureImportSolver | Ark::FeatureMacroProcessor | Ark::FeatureIROptimiser | Ark::FeatureNameResolver;

    "run arkscript unittests"_test = [] {
        "without IR inliner"_test = [] {
            Ark::State state({ lib_path, unittests_path });
            testResource(state, /* is_res= */ true, "LangSuite/unittests.ark", features_no_inline);
        };

        "with IR inliner"_test = [] {
            Ark::State state({ lib_path, unittests_path });
            testResource(state, /* is_res= */ true, "LangSuite/unittests.ark", features_inline);
        };
    };

    "run arkscript stdlib unittests"_test = [] {
        "without IR inliner"_test = [] {
            Ark::State state({ lib_path });
            testResource(state, /* is_res= */ false, ARK_TESTS_ROOT "lib/std/tests/all.ark", features_no_inline);
        };

        "with IR inliner"_test = [] {
            Ark::State state({ lib_path });
            testResource(state, /* is_res= */ false, ARK_TESTS_ROOT "lib/std/tests/all.ark", features_inline);
        };
    };
};
