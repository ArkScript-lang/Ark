#include <boost/ut.hpp>

#include <Ark/Files.hpp>
#include <Ark/Compiler/AST/Parser.hpp>
#include <Ark/Exceptions.hpp>
#include <termcolor/proxy.hpp>

#include <sstream>
#include <algorithm>

#include "TestsHelper.hpp"

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

ut::suite<"Parser"> parser_suite = [] {
    using namespace ut;

    "[successful parsing]"_test = [] {
        iter_test_files(
            "ParserSuite/success",
            [](TestData&& data) {
                Ark::internal::Parser parser;

                should("parse " + data.stem) = [&] {
                    expect(nothrow([&] {
                        const std::string code = Ark::Utils::readFile(data.path);
                        mut(parser).process(data.path, code);
                    }));
                };

                const std::string ast = astToString(parser);

                should("output the same AST and imports (" + data.stem + ")") = [&] {
                    expect(that % ast == data.expected);
                };
            });
    };

    "[error reporting]"_test = [] {
        iter_test_files(
            "ParserSuite/failure",
            [](TestData&& data) {
                try
                {
                    Ark::internal::Parser parser;
                    const std::string code = Ark::Utils::readFile(data.path);
                    parser.process(data.path, code);
                }
                catch (const Ark::CodeError& e)
                {
                    std::stringstream ss;
                    ss << termcolor::nocolorize;
                    Ark::Diagnostics::generate(e, "", ss);

                    should("output the same error message (" + data.stem + ")") = [&] {
                        std::string tested = ss.str();
                        tested.erase(std::ranges::remove(tested, '\r').begin(), tested.end());
                        tested.erase(tested.find(ARK_TESTS_ROOT), std::size(ARK_TESTS_ROOT) - 1);

                        expect(that % tested == data.expected);
                    };
                }
                catch (...)
                {
                    expect(fatal(false)) << "parsing " << data.stem << " should have thrown a CodeError exception";
                }
            });
    };
};
