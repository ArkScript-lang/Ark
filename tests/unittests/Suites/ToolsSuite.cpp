#include <boost/ut.hpp>

#include <Ark/Literals.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Files.hpp>

using namespace boost;

ut::suite<"Tools"> tools_suite = [] {
    using namespace ut;
    using namespace Ark::literals;

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
