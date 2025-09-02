#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <TestsHelper.hpp>

using namespace boost;

ut::suite<"NameResolution"> name_resolution_suite = [] {
    using namespace ut;

    "[run a (import b, c:*, lamp)]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/basic/a.ark")));
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
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/forward_reference/a.ark")));
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
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/shadowing/a.ark")));
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
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/namespace_stacking/a.ark")));
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

    "[run a (import b (import c :odd)), (import c :abs)]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/deep_import_symbols/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto b_ok = mut(vm).operator[]("b_ok");
            expect(b_ok.valueType() == Ark::ValueType::True) << "(= (abs -1) -1)\n";

            const auto c_ok = mut(vm).operator[]("c_ok");
            expect(c_ok.valueType() == Ark::ValueType::True) << "(= (test 5) 5)\n";
        };
    };

    "[symbol import should not be shadowed by hidden symbol]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/shadowing_symbol/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto a_range = mut(vm).operator[]("a_range");
            expect(a_range.valueType() == Ark::ValueType::True) << "(= (range) \"b:map\")\n";

            const auto a_map = mut(vm).operator[]("a_map");
            expect(a_map.valueType() == Ark::ValueType::True) << "(= (map) \"c:map\")\n";
        };
    };

    "[symbol import should not be shadowed by hidden symbol (bis)]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/shadowing_symbol_swap_import_order/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto a_range = mut(vm).operator[]("a_range");
            expect(a_range.valueType() == Ark::ValueType::True) << "(= (range) \"b:map\")\n";

            const auto a_map = mut(vm).operator[]("a_map");
            expect(a_map.valueType() == Ark::ValueType::True) << "(= (map) \"c:map\")\n";
        };
    };

    "[importing two symbols from different modules with the same hidden symbols should resolve correctly]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/hidden_shadowing/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto a_ok = mut(vm).operator[]("ok");
            expect(a_ok.valueType() == Ark::ValueType::True) << "(and (= foo \"b:foo\") (= bar \"c:bar\"))\n";
        };
    };

    "[importing two symbols with the same name from two different files]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/symbol_imports_same_name/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto a_ok = mut(vm).operator[]("ok");
            expect(a_ok.valueType() == Ark::ValueType::True) << "(and (= find \"b.ark\") (= b:find \"b.ark\") (= c:find \"c.ark\"))\n";
        };
    };

    "[importing two symbols with the same name from two different files, one prefixed, one unprefixed]"_test = [] {
        Ark::State state({ lib_path });

        should("compile the resource without any error") = [&] {
            expect(mut(state).doFile(getResourcePath("NameResolutionSuite/resolve_prefixed_in_correct_import/a.ark")));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("resolve symbols from all namespaces without generating bad fully qualified names") = [&] {
            const auto a_ok = mut(vm).operator[]("ok");
            expect(a_ok.valueType() == Ark::ValueType::True) << "(and (= forEach \"b\") (= c:forEach \"c\") (= b:forEach \"b\"))\n";
        };
    };
};
