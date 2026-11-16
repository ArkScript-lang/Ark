#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Ungolfed"> ungolfed_suite = [] {
    using namespace ut;

    "[run arkscript ungolfed golf code solutions]"_test = [] {
        iterTestFiles(
            "UngolfedSuite",
            [](TestData&& data) {
                Ark::State state({ lib_path });

                bool ok = false;

                should("compile " + data.stem) = [&] {
                    try
                    {
                        ok = mut(state).doFile(data.path);
                        expect(ok) << "compilation failed";
                    }
                    catch (const Ark::CodeError&)
                    {
                        expect(false) << "encountered an exception while compiling";
                    }
                };

                if (ok)
                {
                    Ark::VM vm(state);
                    should("run " + data.stem + " without errors (exit code 0)") = [&] {
                        expect(mut(vm).run() == 0);
                    };
                }
            });
    };
};
