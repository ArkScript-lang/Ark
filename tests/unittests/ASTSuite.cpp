#include <boost/ut.hpp>

#include <Ark/Compiler/JsonCompiler.hpp>
#include <Ark/Files.hpp>

#include <string>
#include <filesystem>

namespace fs = std::filesystem;
using namespace boost;

ut::suite<"AST"> ast_suite = [] {
    using namespace ut;

    "[generate valid ast]"_test = [] {
        for (const auto& entry : fs::directory_iterator("tests/unittests/resources/ASTSuite"))
        {
            if (entry.path().extension() != ".ark")
                continue;

            std::string path = entry.path().string();
            std::string stem = entry.path().stem().string();
            fs::path expected_path = entry.path();
            expected_path.replace_extension("json");

            Ark::JsonCompiler compiler(false, { "lib/std/" });

            should("parse " + stem + " and generate a valid AST") = [&] {
                expect(nothrow([&] {
                    mut(compiler).feed(path);
                }));

                std::string json = compiler.compile();
                expect(that % json == Ark::Utils::readFile(expected_path.string()));
            };
        }
    };
};