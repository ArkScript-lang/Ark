#include <boost/ut.hpp>

#include <filesystem>

#include <Ark/Ark.hpp>
#include "TestsHelper.hpp"

using namespace boost;

ut::suite<"NameResolution"> name_resolution_suite = [] {
    using namespace ut;

    "[run a (import b, c:*, lamp)]"_test = [] {
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(get_resource_path("NameResolutionSuite/basic/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without mixing them up") = [&] {
            const auto b_ok = mut(vm).operator[]("b_ok");
            expect(b_ok.valueType() == Ark::ValueType::True) << "b:arg == 'b:arg'\n";

            const auto a_ok = mut(vm).operator[]("a_ok");
            expect(a_ok.valueType() == Ark::ValueType::True) << "arg == 'a:arg'\n";

            const auto b_foo_ok = mut(vm).operator[]("b_foo_ok");
            expect(b_foo_ok.valueType() == Ark::ValueType::True) << "(b:foo \"aa\" \"aaa\") == \"aa aaa\")\n";

            const auto c_ok = mut(vm).operator[]("c_ok");
            expect(c_ok.valueType() == Ark::ValueType::True) << "\"c:egg\" == c:egg, \"c:bacon\" == c:bacon\n";

            const auto d_ok = mut(vm).operator[]("d_ok");
            expect(d_ok.valueType() == Ark::ValueType::True) << "d:lamp == \"d:lamp\"\n";
        };
    };

    "[run a (import b, call closures)]"_test = [] {
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(get_resource_path("NameResolutionSuite/forward_reference/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without mixing them up") = [&] {
            const auto start = mut(vm).operator[]("start");
            expect(start.valueType() == Ark::ValueType::True) << "(b:parent.child.get) == [1 0]\n";

            const auto end = mut(vm).operator[]("end");
            expect(end.valueType() == Ark::ValueType::True) << "(b:parent.child.get) == [5 12]\n";
        };
    };

    "[run a (import b, c, with make defined in both)]"_test = [] {
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(get_resource_path("NameResolutionSuite/shadowing/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without mixing them up") = [&] {
            const auto b_ok = mut(vm).operator[]("b_ok");
            expect(b_ok.valueType() == Ark::ValueType::True) << "(= b:result \"b:make\")\n";

            const auto c_ok = mut(vm).operator[]("c_ok");
            expect(c_ok.valueType() == Ark::ValueType::True) << "(= c:result \"c:make\")\n";
        };
    };

    "[run a (import b (import c))]"_test = [] {
        Ark::State state({ std::filesystem::path(ARK_TESTS_ROOT "/lib/") });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(get_resource_path("NameResolutionSuite/namespace_stacking/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto b_ok = mut(vm).operator[]("b_ok");
            expect(b_ok.valueType() == Ark::ValueType::True) << "(= b:test \"b:test\")\n";

            const auto c_ok = mut(vm).operator[]("c_ok");
            expect(c_ok.valueType() == Ark::ValueType::True) << "(= c:suite \"c:suite\")\n";
        };
    };
};
