#include <boost/ut.hpp>

#include <CLI/REPL/Utils.hpp>

#include <ranges>

using namespace boost;

ut::suite<"Repl"> repl_suite = [] {
    using namespace ut;

    "countOpenEnclosures"_test = [] {
        expect(that % Ark::internal::countOpenEnclosures("", '(', ')') == 0);
        expect(that % Ark::internal::countOpenEnclosures("(", '(', ')') == 1);
        expect(that % Ark::internal::countOpenEnclosures(")", '(', ')') == -1);
        expect(that % Ark::internal::countOpenEnclosures("{}", '(', ')') == 0);
        expect(that % Ark::internal::countOpenEnclosures("{)(()}", '(', ')') == 0);
        expect(that % Ark::internal::countOpenEnclosures("{)(()}", '{', '}') == 0);
    };

    "trimWhitespace"_test = [] {
        const std::string expected = "hello world";
        std::string line = expected;

        Ark::internal::trimWhitespace(line);
        expect(that % line == expected);

        line = "  \thello world";
        Ark::internal::trimWhitespace(line);
        expect(that % line == expected);

        line = "hello world  \t";
        Ark::internal::trimWhitespace(line);
        expect(that % line == expected);

        line = "  \thello world  \t";
        Ark::internal::trimWhitespace(line);
        expect(that % line == expected);
    };

    const auto kws = Ark::internal::getAllKeywords();
    const auto colors = Ark::internal::getColorPerKeyword();

    "getters"_test = [&] {
        expect(that % kws.size() != 0);
        expect(that % kws.size() <= colors.size());
    };

    "hints"_test = [&] {
        int length = 5;
        const auto completions = Ark::internal::hookCompletion(kws, "appen", length);

        expect(that % completions.size() == 2);
        expect(std::ranges::find_if(completions, [](const replxx::Replxx::Completion& v) {
                   return v.text() == "append";
               }) != completions.end());
        expect(std::ranges::find_if(completions, [](const replxx::Replxx::Completion& v) {
                   return v.text() == "append!";
               }) != completions.end());

        length = 5;
        replxx::Replxx::Color color;
        const auto hints = Ark::internal::hookHint(kws, "toStr", length, color);

        expect(that % hints.size() == 1);
        expect(color == replxx::Replxx::Color::GREEN);
    };
};
