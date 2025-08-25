#include <boost/ut.hpp>

#include <CLI/REPL/Utils.hpp>

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

    "getters"_test = [] {
        const auto kws = Ark::internal::getAllKeywords();
        const auto colors = Ark::internal::getColorPerKeyword();

        expect(that % kws.size() != 0);
        expect(that % kws.size() <= colors.size());
    };
};
