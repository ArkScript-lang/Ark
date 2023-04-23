@page guidelines_naming Naming convention in ArkScript

## Definitions

_guideline_: general rule applying to ArkScript code, which must be followed when contributing to the standard library and examples of the main repository
_standard library (aka lib)_: the files and functions in the `lib` folder
_builtins_: functions and constants defined through C++ code, available without having to import anything
_module_: C++ plugin for the ArkScript virtual machine, allowing use of C++ code (eg: the SFML)

## Naming

### ArkScript

It should follow `first:second`, where `first` is related to the module, builtin or lib function area of effect. `second` should be the name of the function it self.

We can see this kind of method as *namespacing* in other languages such as C++.

Example:
~~~~{.clojure}
# for math functions
(math:sin 0.5421)

# for list functions
(list:forEach the-list (fun (element) (...)))

# for a console module
(console:color "red")
~~~~

**Mutation**: if a function does explicit mutation of a state (eg in a closure) instead of returning a new value and being pure, it should be suffixed with `!`, eg `append!` for in place mutations on lists.

### Modules (C++)

It must follow **snake_case** naming convention like : `name_operation_...` where `name` is the name of module, `operation` what the function does and `...` means the rest of name of function, but the recommended keyword limit in name of a function is four.

Avoid C++'s namespacing

Example:
~~~~{.cpp}
#include <ark_msgpack.hpp>

ARK_API Ark::mapping* getFunctionsMapping()
{
    static Ark::mapping[] map = {
        { "msgPack", ArkMsgpack::pack },
        { "msgUnpack", ArkMsgpack::unpack },
        { "msgObjectStr", ArkMsgpack::object_str }
    };

    return map;
}
~~~~
