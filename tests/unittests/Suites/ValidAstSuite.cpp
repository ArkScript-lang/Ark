#include <boost/ut.hpp>

#include <CLI/JsonCompiler.hpp>
#include <string>

#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"AST"> ast_suite = [] {
    using namespace ut;

    "[generate valid ast]"_test = [] {
        iter_test_files(
            "ASTSuite",
            [](TestData&& data) {
                JsonCompiler compiler(false, { ARK_TESTS_ROOT "lib/" });

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
