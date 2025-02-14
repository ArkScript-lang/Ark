#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Rosetta"> rosetta_suite = [] {
    using namespace ut;

    "[run arkscript rosetta code solutions]"_test = [] {
        iter_test_files(
            "RosettaSuite",
            [](TestData&& data) {
                Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

                should("compile " + data.stem) = [&] {
                    try
                    {
                        const bool ok = mut(state).doFile(data.path);
                        expect(ok) << fatal << "compilation failed";
                    }
                    catch (const Ark::CodeError&)
                    {
                        expect(false) << fatal << "encountered an exception while compiling";
                    }
                };

                Ark::VM vm(state);
                should("run " + data.stem + " without errors (exit code 0)") = [&] {
                    expect(mut(vm).run() == 0);
                };
            });
    };
};
