#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"Examples"> examples_suite = [] {
    using namespace ut;

    "[compile and run examples]"_test = [] {
        for (const auto& entry : std::filesystem::directory_iterator(ARK_TESTS_ROOT "examples"))
        {
            if (entry.path().extension() != ".ark")
                continue;

            auto path = entry.path().generic_string();
            auto stem = entry.path().stem().generic_string();

            should("run " + stem + " example") = [&] {
                Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });
                state.setArgs({});

                try
                {
                    const bool ok = mut(state).doFile(path);
                    expect(ok) << fatal << "compilation failed";
                }
                catch (const Ark::CodeError&)
                {
                    expect(false) << fatal << "encountered an exception while compiling";
                }

                Ark::VM vm(state);
                should("return exit code 0") = [&] {
                    expect(mut(vm).run() == 0_i);
                };
            };
        }
    };
};
