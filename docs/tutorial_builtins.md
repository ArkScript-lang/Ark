@page tutorial_builtins The ArkScript builtins

Those are C++ functions, using the ArkScript virtual machine API to communicate with it. They can be used as any other function in ArkScript code.

# Basic template of a builtin

~~~~{.cpp}
#include <Ark/VM/VM.hpp>
#include <Ark/VM/Value.hpp>

Ark::Value myBuiltin(std::vector<Ark::Value>& parameters, Ark::VM* vm)
{
    return Ark::Nil;
}
~~~~

See @ref tutorial_embedding for more details on how to use them.

# Builtins category

We currently have a few categories for our builtins:
* IO, prefixed by `io:` in ArkScript
* List, prefixed by `list:` in ArkScript
* Mathematics, prefixed by `math:` in ArkScript
* String, prefixed by `str:` in ArkScript
* System, prefixed by `sys:` in ArkScript
* Time, prefixed by nothing in ArkScript

# Adding a builtin

1. You need to identify the category it belongs to
2. Then add its prototype in `include/Ark/Builtins/Builtins.hpp` under the right namespace
3. Add it to the builtins list in `src/arkreactor/Builtins/Builtins.cpp`, as follows: `{ "name", Value(category::functionName) }`
4. The implementation will have to be done in `src/arkreactor/Builtins/{category}.cpp`
5. Don't forget to document the new function, using the following snippet:

~~~~{.cpp}
/**
 * @name {function name}
 * @brief {description}
 * @details {optional, more details}
 * @param {name1} {description}
 * @param {name2} {description}
 * =begin
 * optional code sample to show how to use it
 * =end
 * @author {link to the author profile on github}
 */
~~~~
