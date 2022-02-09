@page tutorial_embedding Embedding ArkScript in C++ code

# Using ArkScript

An example is often worth a thousands words:

~~~~{.cpp}
#include <Ark/Ark.hpp>

int main()
{
    // A state can be shared by multiple virtual machines (note that they will NEVER modify it)
    // leave constructor empty to select the default standard library (loaded from an environment variable $ARKSCRIPT_PATH/lib)
    Ark::State state;

    // Will automatically compile the file if needed (if not, will take it from the ark cache)
    state.doString("(let foo (fun (x y) (+ x y 2)))");

    Ark::VM vm(&state);
    vm.run();

    /*
        If you just want to run a precompiled bytecode file:

        Ark::State state;
        state.feed("mybytecode.arkc");
        Ark::VM vm(&state);
        vm.run();
    */

    /*
        To run an ArkScript function from C++ code and retrieve the result:
        we will say the code is (let foo (fun (x y) (+ x y 2)))
    */
    auto value = vm.call("foo", 5, 6.0);
    std::cout << value << "\n";  // displays 13

    return 0;
}
~~~~

# Adding your own functions

~~~~{.cpp}
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
    Ark::State state;

    state.loadFunction("my_function", my_function);
    // we can also load C++ lambdas
    // we could have done this after creating the VM, it would still works
    // we just need to do that BEFORE we call vm.run()
    state.loadFunction("foo", [](std::vector<Ark::Value>& args, Ark::VM* vm) {
        return Ark::Value(static_cast<int>(args.size()));
    });

    state.doString("(let bar (my_function 1 2 3 1)) (let egg (foo 1 2 3))");  // we can call state.doFile() before or after state.loadFunction()

    Ark::VM vm(&state);
    vm.run();

    auto bar = vm["bar"];
    std::cout << bar << "\n";

    auto egg = vm["egg"];
    std::cout << egg << "\n";

    return 0;
}
~~~~

# Adding your own types in ArkScript

~~~~{.cpp}
enum class Breakfast { Eggs, Bacon, Pizza };

Breakfast& getBreakfast()
{
    static Breakfast bf = Breakfast::Pizza;
    return bf;
}

UserType::ControlFuncs* get_cfs()
{
    static UserType::ControlFuncs cfs;

    cfs.ostream_func = [](std::ostream& os, const UserType& a) -> std::ostream& {
        os << "Breakfast::";
        switch (a.as<Breakfast>())
        {
            case Breakfast::Eggs:  os << "Eggs";    break;
            case Breakfast::Bacon: os << "Bacon";   break;
            case Breakfast::Pizza: os << "Pizza";   break;
            default:               os << "Unknown"; break;
        }
        return os;
    };

    return &cfs;
}

int main()
{
    Ark::State state;

    state.loadFunction("getBreakfast", [](std::vector<Ark::Value>& n, Ark::VM* vm) -> Ark::Value {
        // we need to send the address of the object, which will be casted
        // to void* internally
        Ark::Value v = Ark::Value(Ark::UserType(&getBreakfast()));

        // register the unique control functions block for this usertype
        // this cfs block can be shared between multiple usertype to reduce memory usage
        v.usertypeRef().setControlFuncs(get_cfs());

        return v;
    });

    state.loadFunction("useBreakfast", [](std::vector<Ark::Value>& n, Ark::VM* vm) -> Ark::Value {
        if (n[0].valueType() == Ark::ValueType::User && n[0].usertype().is<Breakfast>())
        {
            std::cout << "UserType detected as an enum class Breakfast" << std::endl;
            Breakfast& bf = n[0].usertype().as<Breakfast>();
            std::cout << "Got " << n[0].usertype() << "\n";
            if (bf == Breakfast::Pizza)
                std::cout << "Good choice! Have a nice breakfast ;)" << std::endl;
        }

        return Ark::Nil;
    });

    state.doString("(begin (let a (getBreakfast)) (print a) (useBreakfast a))");
    Ark::VM vm(&state);
    vm.run();

    /*
        Will print

        Breakfast::Pizza
        UserType detected as an enum class Breakfast
        Got Breakfast::Pizza
        Good choice! Have a nice breakfast ;)
    */

    return 0;
}
~~~~
