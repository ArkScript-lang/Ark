#include <boost/ut.hpp>

#include <Ark/Utils/Literals.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Utils/Files.hpp>

using namespace boost;

ut::suite<"Tools"> tools_suite = [] {
    using namespace ut;
    using namespace Ark::literals;
    using namespace std::string_literals;

    "Utils::splitString"_test = [] {
        expect(that % Ark::Utils::splitString("a,b,c", ',') == std::vector<std::string> { "a", "b", "c" });
        expect(that % Ark::Utils::splitString("a,b,c", ';') == std::vector<std::string> { "a,b,c" });
    };

    "Utils::ltrim, Utils::rtrim"_test = [] {
        std::string input = "abc";
        expect(that % Ark::Utils::ltrim(input) == "abc"s);
        input = "  abc";
        expect(that % Ark::Utils::ltrim(input) == "abc"s);
        input = "abc  ";
        expect(that % Ark::Utils::ltrim(input) == "abc  "s);

        input = "abc";
        expect(that % Ark::Utils::rtrim(input) == "abc"s);
        input = "  abc";
        expect(that % Ark::Utils::rtrim(input) == "  abc"s);
        input = "abc  ";
        expect(that % Ark::Utils::rtrim(input) == "abc"s);
    };

    "Utils::levenshteinDistance"_test = [] {
        expect(that % Ark::Utils::levenshteinDistance("arkscript", "arkscript") == 0_z);
        expect(that % Ark::Utils::levenshteinDistance("arkscript", "Orkscript") == 1_z);
        expect(that % Ark::Utils::levenshteinDistance("arkscript", "OrCscript") == 2_z);
        expect(that % Ark::Utils::levenshteinDistance("arkscript", "OrC") == 8_z);
    };

    "Utils::fileExists"_test = [] {
        expect(Ark::Utils::fileExists(".gitignore"));
        expect(!Ark::Utils::fileExists(""));
    };
};
