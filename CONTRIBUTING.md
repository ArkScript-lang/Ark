# Contributing

You will find here a summary of the different things you should do / look up to contribute to the project.

## Starting

* First, [fork](https://github.com/ArkScript-lang/Ark/fork) the repository
* Then, clone your fork: 
    * HTTPS: `git clone https://github.com/<username>/Ark.git`
    * or SSH: `git clone git@github.com:<username>/Ark.git`
* Create a branch for your feature: `git checkout -b feat/my-awesome-idea`
* When you're done, push it to your fork and submit a pull request

Don't know what to work on? No worries, we have a [list of things to do](https://github.com/ArkScript-lang/Ark/projects). Also, you can check the issues to find something to do!

## The architecture of the project

For an in-depth explanation: [Understanding the project architecture](https://arkscript-lang.dev/impl/d2/d0b/brief_architecture.html).

For a quick glance at the organization of the project, check this out:

![diagram of the folders and subfolders in the project](images/diagram.svg)

## Contributing to a module (or creating one)

The repository for this is [ArkScript-lang/modules](https://github.com/ArkScript-lang/modules), and the language used is C++.

See the guide [to create a module](https://arkscript-lang.dev/impl/d1/d8c/modules.html).

## Submitting a pull request

### Checking the enforcement of the coding style

If it is a C++ project, it is strongly advised to run clang-format on your code *before* submitting a pull request.

You can do as follows if you are a Windows user:
```powershell
Function run-on {
    param (
        $folder
    )
    Get-ChildItem -Path $folder -File -Recurse | Foreach {clang-format -style=file -i $_.fullname}
}

run-on .\include\Ark
run-on .\src
```

### Running the test suite

The ArkScript test suite requires the console module for text colors. You can build all the modules by including the `-DARK_BUILD_MODULES` CMake switch when building ArkScript. Once you have the modules built, you can run the ArkScript test suite from the root folder using `./build/arkscript tests/arkscript/unittests.ark --lib ./lib`.

The standard library test suite should be ran from the root folder of the standard library project (the console module will need to be copied to the folder). You can run standard library test suite using `./arkscript tests/all.ark --lib ./`.

## C++ coding guidelines

* Avoid `auto` whenever possible. Using it is tolerated for complex types such as iterators
* Indent with **4 spaces**
* Every brace (`{`, `}`) must be on its own line
* Conditions with a single statement (`if (condition) do_this();`) do not need to be enclosed in braces
* Put a space between `for`, `while`, `if` and `(...)`, around each `=` sign (wherever it is, even in for-loops), between `#include` and the file to include
* Prefer `enum class` over `enum`
* Left-const over right-const
* The `*` and `&` are part of the type:
```cpp
// AVOID THIS
int *i = new int(5);
int &j = k;

// PREFERRED
int* i = new int(5);
int& j = k;
```
* Naming:
    * Methods and functions: camelCase
    * Variables: snake_case
    * Constant expressions: PascalCase
    * Enumeration members: PascalCase
* For-each loops should use const references or rvalue references instead of plain copies:
```cpp
// AVOID THIS
for (auto value : container)

// PREFERRED
for (const auto& value : container)
for (auto&& value : container)
```
* For-loops should be optimized whenever possible, as follows:
```cpp
// AVOID THIS
for (std::size_t i = 0; i < container.size(); ++i)
    ...

// PREFERRED
for (std::size_t i = 0, end = container.size(); i < end; i++)
    ...
```
* Header-guards should be written using `#ifndef`, `#define` and `#endif`, the define being in MACRO_CASE
* Functions and methods which can not throw exceptions must be marked `noexcept`
* In header files, have a blank line between each method / function / structure / constant ; also put blank lines around include blocks
* In classes, avoid using `this`. Prefix every member variable with `m_`
* In classes, do not define the methods in the header file, only in the source file (unless they are `inline` or templated)
* Add Doxygen file comments at the top of each newly created header file:
```cpp
/**
 * @file Lexer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Tokenize ArkScript code
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */
 
#include <bla>

code...
```

Snippet to recapitulate guidelines for headers:

```cpp
/**
 * @file Lexer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Tokenize ArkScript code
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef HEADER_GUARD
#define HEADER_GUARD

#include <Ark/Compiler/Something.hpp>
#include <vector>

namespace Ark
{
    /**
     * @brief doxygen documentation about the class
     *
     */
    class Lexer
    {
    public:
        /**
         * @brief doxygen documentation here
         * 
         */
        Lexer();

        /**
         * @brief doxygen documentation here
         * 
         * @param a_parameter defines the power of the flux capacitor
         */
        void aMethod(const std::string& a_parameter);

    private:
        int m_member;  ///< This is a doxygen comment
    };
}

#endif  // Adding an empty line at the end of each file is strongly advised

```

## ArkScript coding guidelines

Check out the [ArkScript RFC 001](https://github.com/ArkScript-lang/rfc/blob/master/001-coding-guidelines.md). We are quite flexible about it, those are just general guidelines to produce a readable ArkScript code.
