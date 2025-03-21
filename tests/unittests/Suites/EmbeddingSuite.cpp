#include <boost/ut.hpp>

#include <Ark/Ark.hpp>
#include <vector>
#include <iostream>

using namespace boost;

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

    "[load cpp function and call it from arkscript]"_test = [] {
        Ark::State state;
        state.loadFunction("my_function", my_function);
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

    "[load usertype and cpp lambdas and call them from arkscript]"_test = [] {
        Ark::State state;
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
                auto& bf = n[0].usertypeRef().as<Breakfast>();
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
