#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"Lang"> lang_suite = [] {
    using namespace ut;

    "[run arkscript unittests]"_test = [] {
        Ark::State state({ lib_path, unittests_path });

        try
        {
            const bool ok = mut(state).doFile(getResourcePath("LangSuite/unittests.ark"));
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

        should("have no failures") = [&] {
            const auto failure_count = mut(vm).operator[]("failure_count");
            expect(failure_count.valueType() == Ark::ValueType::Number &&
                   failure_count.number() == 0.0)
                << "failure_count = 0\n";
        };
    };

    "[run arkscript stdlib unittests]"_test = [] {
        Ark::State state({ lib_path });

        try
        {
            const bool ok = mut(state).doFile(ARK_TESTS_ROOT "lib/std/tests/all.ark");
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

        should("have no failures") = [&] {
            const auto failure_count = mut(vm).operator[]("failure_count");
            expect(failure_count.valueType() == Ark::ValueType::Number &&
                   failure_count.number() == 0.0)
                << "failure_count = 0\n";
        };
    };
};
