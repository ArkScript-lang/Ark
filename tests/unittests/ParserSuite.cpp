#include <boost/ut.hpp>

#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Files.hpp>
#include <termcolor/proxy.hpp>

#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace boost;

std::string astToString(Ark::internal::Parser& parser)
{
    using namespace ut;

    std::stringstream ss;
    for (auto it = parser.ast().constList().begin() + 1, end = parser.ast().constList().end(); it != end; ++it)
        ss << *it << "\n";

    const auto& imports = parser.imports();

    if (!imports.empty())
        ss << "\n";
    for (std::size_t i = 0, end = imports.size(); i < end; ++i)
    {
        Ark::internal::Import data = imports[i];
        ss << i << ") " << data.prefix;
        if (data.isBasic())
            ss << " (basic)";
        else if (data.isGlob())
            ss << " (glob)";
        else
        {
            ss << " ( ";
            for (const std::string& sym : data.symbols)
                ss << sym << " ";
            ss << ")";
        }
        ss << "\n";
    }

    return ss.str();
}

ut::suite<"Parser"> errors = [] {
    using namespace ut;

    "[successful parsing]"_test = [] {
        for (const auto& entry : fs::directory_iterator("tests/unittests/resources/ParserSuite/success"))
        {
            if (entry.path().extension() != ".ark")
                continue;

            Ark::internal::Parser parser;

            std::string path = entry.path().string();
            std::string stem = entry.path().stem().string();
            fs::path expected_path = entry.path();
            expected_path.replace_extension("expected");

            should("parse " + stem) = [&] {
                expect(nothrow([&] {
                    mut(parser).processFile(path);
                }));
            };

            // expect(that % parser.ast().constList().size() >= 1_u);
            std::string ast = astToString(parser);

            should("output the same AST and imports (" + stem + ")") = [&] {
                expect(that % ast == Ark::Utils::readFile(expected_path.string()));
            };
        }
    };

    "[error reporting]"_test = [] {
        for (const auto& entry : fs::directory_iterator("tests/unittests/resources/ParserSuite/failure"))
        {
            if (entry.path().extension() != ".ark")
                continue;

            Ark::internal::Parser parser;

            std::string path = entry.path().string();
            std::string stem = entry.path().stem().string();
            fs::path expected_path = entry.path();
            expected_path.replace_extension("expected");

            try
            {
                parser.processFile(path);
            }
            catch (const Ark::CodeError& e)
            {
                std::stringstream ss;
                ss << termcolor::nocolorize;
                Ark::Diagnostics::generate(e, "", ss);

                should("output the same error message (" + stem + ")") = [&] {
                    expect(that % ss.str() == Ark::Utils::readFile(expected_path.string()));
                };
            }
            catch (...)
            {
                expect(fatal(false)) << "parsing " << stem << " should have thrown a CodeError exception";
            }
        }
    };
};