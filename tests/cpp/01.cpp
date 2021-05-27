#include <iostream>

#include <Ark/Ark.hpp>

#include "Tests.hpp"

int main()
{
    // A state can be shared by multiple virtual machines (note that they will NEVER modify it)
    // leave constructor empty to select the default standard library (loaded from an environment variable $ARKSCRIPT_PATH/lib)
    // persistance is needed to use vm.call(function_name, args...)
    Ark::State state(/* options */ Ark::FeaturePersist);

    // Will automatically compile the file if needed (if not, will take it from the ark cache)
    state.doString("(let foo (fun (x y) (+ x y 2)))");

    Ark::VM vm(&state);
    CHECK_VM_RUN(vm)

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
    CHECK_VALUE_NUMBER(value, 13.0)

    RETURN_PASSED()
}