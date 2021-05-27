#include <iostream>
#include <vector>

#include <Ark/Ark.hpp>

Ark::Value my_function(std::vector<Ark::Value>& args, Ark::VM* vm)
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

int main()
{
    Ark::State state(/* options */ Ark::FeaturePersist);
    state.doString("(let bar (my_function 1 2 3 1)) (let egg (foo 1 2 3))");  // we can call state.doFile() before or after state.loadFunction()

    state.loadFunction("my_function", my_function);

    // we can also load C++ lambdas
    // we could have done this after creating the VM, it would still works
    // we just need to do that BEFORE we call vm.run()
    state.loadFunction("foo", [](std::vector<Ark::Value>& args, Ark::VM* vm) {
        return Ark::Value(static_cast<int>(args.size()));
    });

    Ark::VM vm(&state);
    if (vm.run() != 0)
        return 1;  // FAILED

    auto bar = vm["bar"];
    if (bar.valueType() != Ark::ValueType::Number || bar.number() != 1.0 * 2.0 - 3.0 / 1.0)
        return 1;  // FAILED

    auto egg = vm["egg"];
    if (egg.valueType() != Ark::ValueType::Number || egg.number() != 3)
        return 1;  // FAILED

    return 0;  // PASED
}