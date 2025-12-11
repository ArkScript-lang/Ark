#include <boost/ut.hpp>

#include <Ark/Utils/Files.hpp>
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Error/Exceptions.hpp>

#include <sstream>
#include <algorithm>

#include <TestsHelper.hpp>

using namespace boost;

std::string astToString(Ark::internal::Parser& parser)
{
    using namespace ut;

    std::stringstream ss;
    for (auto it = parser.ast().constList().begin() + 1, end = parser.ast().constList().end(); it != end; ++it)
        it->debugPrint(ss) << "\n";

    const auto& imports = parser.imports();

    if (!imports.empty())
        ss << "\n";
    for (std::size_t i = 0, end = imports.size(); i < end; ++i)
    {
        Ark::internal::Import data = imports[i];
        ss << i << ") " << data.prefix;
        if (data.isBasic())
            ss << " (basic)";
        if (data.is_glob)
            ss << " (glob)";
        ss << " ( ";
        for (const std::string& sym : data.symbols)
            ss << sym << " ";
        ss << ")\n";
    }

    return ss.str();
}

ut::suite<"Parser"> parser_suite = [] {
    using namespace ut;

    "[successful parsing]"_test = [] {
        iterTestFiles(
            "ParserSuite/success",
            [](TestData&& data) {
                Ark::internal::Parser parser(/* debug= */ 0);

                should("parse " + data.stem) = [&] {
                    expect(nothrow([&] {
                        const std::string code = Ark::Utils::readFile(data.path);
                        mut(parser).process(data.path, code);
                    }));
                };

                std::string ast = astToString(parser);
                Ark::Utils::ltrim(Ark::Utils::rtrim(ast));

                should("output the same AST and imports (" + data.stem + ")") = [&] {
                    expectOrDiff(data.expected, ast);
                };
            });
    };

    "[error reporting]"_test = [] {
        iterTestFiles(
            "ParserSuite/failure",
            [](TestData&& data) {
                try
                {
                    Ark::internal::Parser parser(/* debug= */ 0);
                    const std::string code = Ark::Utils::readFile(data.path);
                    parser.process(data.path, code);
                }
                catch (const Ark::CodeError& e)
                {
                    should("output the same error message (" + data.stem + ")") = [&] {
                        std::string tested = sanitizeCodeError(e);
                        Ark::Utils::ltrim(Ark::Utils::rtrim(tested));
                        expectOrDiff(data.expected, tested);
                        if (shouldWriteNewDiffsTofile() && data.expected != tested)
                            updateExpectedFile(data, tested);
                    };
                }
                catch (...)
                {
                    expect(fatal(false)) << "parsing " << data.stem << " should have thrown a CodeError exception";
                }
            });
    };
};
