#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Ark/Utils/Utils.hpp>
#include <vector>
#include <TestsHelper.hpp>
#include <iostream>

using namespace boost;
using namespace Ark::literals;

// cppcheck-suppress [constParameterCallback, constParameterReference]
Ark::Value my_function(std::vector<Ark::Value>& args, Ark::VM* vm [[maybe_unused]])
{
    // checking argument number
    if (args.size() != 4)
        throw std::runtime_error("my_function needs 4 arguments!");

    auto a = args[0],
         b = args[1],
         c = args[2],
         d = args[3];

    // checking arguments type
    if (a.valueType() != Ark::ValueType::Number ||
        b.valueType() != Ark::ValueType::Number ||
        c.valueType() != Ark::ValueType::Number ||
        d.valueType() != Ark::ValueType::Number)
        throw Ark::TypeError("Type mismatch for my_function: need only numbers");

    // type is automatically deducted from the argument
    return Ark::Value(a.number() * b.number() - c.number() / d.number());
}

Ark::Value bad_resolve(std::vector<Ark::Value>& args, Ark::VM* vm [[maybe_unused]])
{
    vm->resolve(vm->getDefaultContext(), args);
    return Ark::Nil;
}

enum class Breakfast
{
    Eggs,
    Bacon,
    Pizza
};

Breakfast& getBreakfast()
{
    static Breakfast bf = Breakfast::Pizza;
    return bf;
}

Ark::UserType::ControlFuncs* get_cfs()
{
    static Ark::UserType::ControlFuncs cfs;

    cfs.ostream_func = [](std::ostream& os, const Ark::UserType& a) -> std::ostream& {
        os << "Breakfast::";
        switch (a.as<Breakfast>())
        {
            case Breakfast::Eggs: os << "Eggs"; break;
            case Breakfast::Bacon: os << "Bacon"; break;
            case Breakfast::Pizza: os << "Pizza"; break;
            default: os << "Unknown"; break;
        }
        return os;
    };

    return &cfs;
}

ut::suite<"Embedding"> embedding_suite = [] {
    using namespace ut;

    "[run string and call arkscript function from cpp without args]"_test = [] {
        Ark::State state;
        state.setDebug(3);  // so that the logger branches can be used as well, and covered

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let foo (fun () 4))"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have symbol foo registered") = [&] {
            const auto func = mut(vm)["foo"];
            expect(func.isFunction());
        };

        should("(foo) have a value of 4") = [&] {
            const auto value = mut(vm).call("foo");
            expect(value.valueType() == Ark::ValueType::Number);
            expect(value.number() == 4.0_d);
        };
    };

    "[run string and call arkscript function from cpp]"_test = [] {
        Ark::State state;

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let foo (fun (x y) (+ x y 2)))"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have symbol foo registered") = [&] {
            const auto func = mut(vm)["foo"];
            expect(func.isFunction());
        };

        should("(foo 5 6.0) have a value of 13") = [&] {
            const auto value = mut(vm).call("foo", 5, 6.0);
            expect(value.valueType() == Ark::ValueType::Number);
            expect(value.number() == 13.0_d);
        };

        should("get nil when retrieving unbound symbol") = [&] {
            const auto value = mut(vm)["unknown"];
            expect(value.valueType() == Ark::ValueType::Nil);
        };
    };

    "[run string and try to call an arkscript number from cpp]"_test = [] {
        Ark::State state;

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let foo 5)"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have symbol foo registered") = [&] {
            const auto func = mut(vm)["foo"];
            expect(func.valueType() == Ark::ValueType::Number);
        };

        should("(foo) should not be callable") = [&] {
            try
            {
                const auto value = mut(vm).call("foo");
                expect(false) << "calling foo should result in an error";
            }
            catch (const std::exception& e)
            {
                std::string msg = e.what();
                expect(that % Ark::Utils::ltrim(Ark::Utils::rtrim(msg)) == std::string("TypeError: Can't call 'foo': it isn't a Function but a Number"));
            }
        };
    };

    "[reset the VM and use it to run code again]"_test = [] {
        Ark::State state;

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let foo (fun (a b) (+ a b))) (let t (time))"));
        };

        Ark::VM vm(state);
        double timestamp = 0.0;
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
            timestamp = vm["t"].number();
        };

        should("have symbol foo registered") = [&] {
            const auto func = mut(vm)["foo"];
            expect(func.isFunction());
        };

        should("reset VM by running code again") = [&] {
            expect(mut(vm).run() == 0_i);
            const double new_time = vm["t"].number();
            expect(that % timestamp < new_time);
        };
    };

    "[load cpp function with captured data]"_test = [] {
        Ark::State state;

        int capture = 42;
        // cppcheck-suppress constParameterReference
        state.loadFunction("my_function", [=](std::vector<Ark::Value>& args, [[maybe_unused]] Ark::VM* /*vm*/) {
            int solution = 0;
            for (const Ark::Value& value : args)
            {
                solution += value.number();
            }
            return Ark::Value(capture + solution);
        });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let bar (my_function 1 2 3 1))"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("compute egg to 49") = [&] {
            auto egg = mut(vm)["bar"];
            expect(egg.valueType() == Ark::ValueType::Number);
            expect(egg.number() == 49_i);
        };
    };

    "[load cpp function with captured reference]"_test = [] {
        Ark::State state;

        std::string name;
        // cppcheck-suppress constParameterReference
        state.loadFunction("my_function", [&name](std::vector<Ark::Value>& args, [[maybe_unused]] Ark::VM* /*vm*/) {
            for (const Ark::Value& value : args)
            {
                name.append(value.string());
            }
            return Ark::Value();
        });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString(R"(
                (my_function "Iron" " " "Man")
            )"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have mutated the capture variable") = [&] {
            expect(name == "Iron Man");
        };
    };

    "[load cpp function and call it from arkscript]"_test = [] {
        Ark::State state;
        state.loadFunction("my_function", my_function);
        // cppcheck-suppress constParameterReference
        state.loadFunction("foo", [](std::vector<Ark::Value>& args, Ark::VM* /*vm*/) {
            return Ark::Value(static_cast<int>(args.size()));
        });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let bar (my_function 1 2 3 1)) (let egg (foo 1 2 3))"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("compute bar to 1.0 * 2.0 - 3.0 / 1.0") = [&] {
            auto bar = mut(vm)["bar"];
            expect(bar.valueType() == Ark::ValueType::Number);
            expect(bar.number() == -1.0_d);
        };

        should("compute egg to 3") = [&] {
            auto egg = mut(vm)["egg"];
            expect(egg.valueType() == Ark::ValueType::Number);
            expect(egg.number() == 3_i);
        };
    };

    "[errors in C++ functions called in ArkScript bubble up]"_test = [] {
        constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

        Ark::State state;
        state.loadFunction("my_function", my_function);

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let bar (my_function 1 2 nil 1))", features));
        };

        Ark::VM vm(state);
        should("return exit code 1") = [&] {
            expect(throws([&] {
                mut(vm).run(/* fail_with_exception= */ true);
            }));
        };
    };

    "[errors in C++ functions called in ArkScript bubble up when resolving a non-function]"_test = [] {
        constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;

        Ark::State state;
        state.loadFunction("bad_resolve", bad_resolve);

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let bar (bad_resolve 1 2))", features));
        };

        Ark::VM vm(state);
        should("return exit code 1") = [&] {
            expect(throws([&] {
                mut(vm).run(/* fail_with_exception= */ true);
            }));
        };
    };

    "[fail to compile embedded code]"_test = [] {
        constexpr uint16_t features = Ark::DefaultFeatures | Ark::FeatureTestFailOnException;
        Ark::State state({ ARK_TESTS_ROOT "lib" });
        const std::string code = "(import std.Sys) (let foo sys:args) (let b bar)";
        const std::string expected = R"(    1 | (import std.Sys) (let foo sys:args) (let b bar)
      |                                            ^~~
        Unbound variable "bar" (variable is used but not defined))";

        should("compile the string with an error") = [&] {
            try
            {
                const bool ok = mut(state).doString(code, features);
                expect(!ok) << fatal;  // we shouldn't be here, the compilation has to fail
            }
            catch (const Ark::CodeError& e)
            {
                std::stringstream stream;
                Ark::Diagnostics::generateWithCode(e, code, stream, /* colorize= */ false);
                std::string diag = stream.str();
                diag.erase(std::ranges::remove(diag, '\r').begin(), diag.end());
                Ark::Utils::rtrim(diag);

                expectOrDiff(expected, diag);
            }
        };
    };

    "[retrieve sys:args in embedded code]"_test = [] {
        Ark::State state({ ARK_TESTS_ROOT "lib" });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(import std.Sys) (let foo sys:args) (let b foo)"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have symbol foo registered") = [&] {
            const auto foo = mut(vm)["foo"];
            expect(foo.valueType() == Ark::ValueType::List);
            expect(foo.constList().size() == 0_z);
        };
    };

    "[set and retrieve sys:args in embedded code]"_test = [] {
        Ark::State state({ ARK_TESTS_ROOT "lib" });
        state.setArgs({ "foo", "bar", "--eggs" });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(import std.Sys) (let foo sys:args) (let b foo)"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("have symbol foo registered") = [&] {
            const auto foo = mut(vm)["foo"];
            expect(foo.valueType() == Ark::ValueType::List);
            expect(foo.constList().size() == 3_z);
            expect(foo.constList()[0].string() == "foo");
            expect(foo.constList()[1].string() == "bar");
            expect(foo.constList()[2].string() == "--eggs");
        };
    };

    "[load usertype and cpp lambdas and call them from arkscript]"_test = [] {
        Ark::State state;
        // cppcheck-suppress constParameterReference
        state.loadFunction("getBreakfast", [](std::vector<Ark::Value>& n [[maybe_unused]], Ark::VM* vm [[maybe_unused]]) -> Ark::Value {
            // we need to send the address of the object, which will be cast
            // to void* internally
            // register the unique control functions block for this usertype
            // this cfs block can be shared between multiple usertype to reduce memory usage
            Ark::Value v = Ark::Value(Ark::UserType(&getBreakfast(), get_cfs()));

            return v;
        });

        state.loadFunction("useBreakfast", [](std::vector<Ark::Value>& n, Ark::VM* vm [[maybe_unused]]) -> Ark::Value {
            if (n[0].valueType() == Ark::ValueType::User && n[0].usertype().is<Breakfast>())
            {
                const auto& bf = n[0].usertypeRef().as<Breakfast>();
                if (bf == Breakfast::Pizza)
                    return Ark::Value(1);
                return Ark::Value(2);
            }

            return Ark::Value(0);
        });

        should("compile the string without any error") = [&] {
            expect(mut(state).doString("(let a (getBreakfast)) (let b (useBreakfast a))"));
        };

        Ark::VM vm(state);
        should("return exit code 0") = [&] {
            expect(mut(vm).run() == 0_i);
        };

        should("compute a to usertype(breakfast::pizza)") = [&] {
            auto a = mut(vm)["a"];
            expect(a.valueType() == Ark::ValueType::User);
            expect(a.usertype().is<Breakfast>());
            expect(a.usertype().as<Breakfast>() == Breakfast::Pizza);
        };

        should("compute b to 1") = [&] {
            auto b = mut(vm)["b"];
            expect(b.valueType() == Ark::ValueType::Number);
            expect(b.number() == 1_i);
        };
    };
};
