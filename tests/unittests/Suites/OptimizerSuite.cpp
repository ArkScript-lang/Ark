#include <boost/ut.hpp>

#include <CLI/JsonCompiler.hpp>
#include <string>

#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Optimizer"> optimizer_suite = [] {
    using namespace ut;

    "[generate optimized ast]"_test = [] {
        iter_test_files(
            "OptimizerSuite",
            [](TestData&& data) {
                JsonCompiler compiler(false, { ARK_TESTS_ROOT "lib/" }, Ark::FeatureASTOptimizer);

                std::string json;
                should("parse " + data.stem) = [&] {
                    expect(nothrow([&] {
                        mut(compiler).feed(data.path);
                        json = mut(compiler).compile();
                    }));
                };

                should("output the expected AST for " + data.stem) = [&] {
                    expect_or_diff(data.expected, json);
                };
            },
            /* expected_ext= */ "json");
    };
};
