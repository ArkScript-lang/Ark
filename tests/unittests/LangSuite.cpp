#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Lang"> lang_suite = [] {
    using namespace ut;

    "[run arkscript unittests]"_test = [] {
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(get_resource_path("LangSuite/unittests.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };
    };
};
