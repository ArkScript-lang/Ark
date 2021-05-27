#include <iostream>
#include <vector>

#include <Ark/Ark.hpp>

#include "Tests.hpp"

enum class Breakfast { Eggs, Bacon, Pizza };

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
            Breakfast& bf = n[0].usertypeRef().as<Breakfast>();
            std::cout << "Got " << n[0].usertype() << "\n";
            if (bf == Breakfast::Pizza)
                std::cout << "Good choice! Have a nice breakfast ;)" << std::endl;
        }

        return Ark::Nil;
    });

    state.doString("(let a (getBreakfast)) (print a) (useBreakfast a)");
    Ark::VM vm(&state);
    CHECK_VM_RUN(vm)

    /*
        Will print

        Breakfast::Pizza
        UserType detected as an enum class Breakfast
        Got Breakfast::Pizza
        Good choice! Have a nice breakfast ;)
    */

    RETURN_PASSED()
}