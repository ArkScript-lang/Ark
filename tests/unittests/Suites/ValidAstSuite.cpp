#include <boost/ut.hpp>

#include <CLI/JsonCompiler.hpp>
#include <string>

#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"AST"> ast_suite = [] {
    using namespace ut;

    "[generate valid ast]"_test = [] {
        iterTestFiles(
            "ASTSuite",
            [](const TestData& data) {
                JsonCompiler compiler(false, { lib_path });

                std::string json;
                should("parse " + data.stem) = [&] {
                    expect(nothrow([&] {
                        mut(compiler).feed(data.path);
                        json = mut(compiler).compile();
                    }));
                };

                should("output the expected AST for " + data.stem) = [&] {
                    expectOrDiff(data.expected, json);
                };
            },
            { .expected_ext = "json" });
    };
};
