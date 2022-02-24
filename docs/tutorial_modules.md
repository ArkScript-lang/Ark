@page tutorial_modules Creating modules

# Before starting

You will still need to dive a bit into the documentation of the project, to know how:
* the VM API works, and what it provides
* the possibilities of the Value type (comparisons, creations)
* how to use the `UserType`

Also, read the [RFC 004](https://github.com/ArkScript-lang/rfc/blob/master/004-module-error-handling.md) about module error handling to use the same conventions as the other modules, and the [RFC 003](https://github.com/ArkScript-lang/rfc/blob/master/003-naming-convention.md) about naming conventions in ArkScript (specifically see the *Modules (C++)* section).

## Creating a new module

In your [ArkScript-lang/modules](https://github.com/ArkScript-lang/modules) fork, run the Python script as follows `python3 shell/createmodules/create.py module_name`. This will create a new folder `module_name/` for you, alongside a few folders and files needed to get you started.

Create a `Main.cpp` file in `module_name/src/` with the following content:

~~~~{.cpp}
#include <Ark/Module.hpp>

Value foo(std::vector<Value>& n [[maybe_unused]], Ark::VM* vm [[maybe_unused]])
{
    return Value(1);
}

ARK_API mapping* getFunctionsMapping()
{
    mapping* map = mapping_create(1);
    mapping_add(map[0], "test:foo", foo);

    return map;
}
~~~~

Let's walk through this line by line:
- `#include <Ark/Module.hpp>` includes basic files from ArkScript to be able to use the VM, instanciate values, and generate the entry point of the module
- `Value foo(std::vector<Value>& n [[maybe_unused]], Ark::VM* vm [[maybe_unused]]) {...}` defines a function for our module, taking an argument list from the VM, and a non-owning pointer to the VM
- `ARK_API mapping* getFunctionsMapping()` declares the entrypoint of our module
- `mapping* map = mapping_create(1);` creates a mapping of a single element to hold the name -> function pointer association, defining the module
- `mapping_add(map[0], "test:foo", foo);` adds an element at position 0 in our mapping, using the previously defining function
    - note that the given name is `"test:foo"`: this is a convention in ArkScript, every module function must be prefixed by the module name's

## Building your module

Clone ArkScript, then copy your modules fork to lib/modules. This is required for CMake to be able to find ArkScript headers.

You will need to update `lib/modules/CMakeLists.txt` to add the following code:

~~~~{cmake}
set(ARK_MOD_MODULE_NAME Off CACHE BOOL "Build the module_name module")

if (ARK_MOD_MODULE_NAME OR ARK_MOD_ALL)
    add_subdirectory(${PROJECT_SOURCE_DIR}/module_name)
endif()
~~~~

Then, run `cmake . -Build -DARK_BUILD_MODULES=On -DARK_MOD_MODULE_NAME=On`, and build only your module with `cmake --build build --target module_name`.
