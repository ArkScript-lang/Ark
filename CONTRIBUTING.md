# Contributing

You will find here a summary of the different things you should do / look up to contribute to the project.

## Starting

* First, [fork](https://github.com/ArkScript-lang/Ark/fork) the repository
* Then, clone your fork: 
    * HTTPS: `git clone https://github.com/<username>/Ark.git`
    * or SSH: `git clone git@github.com:<username>/Ark.git`
* Create a branch for your feature: `git checkout -b feat/my-awesome-idea`
* When you're done, push it to your fork and submit a pull request!

Don't know what to work on? No worries, we have a [list of things to do](https://github.com/ArkScript-lang/Ark/projects). Also, you can check the issues to find something to do!

## C++ coding guidelines

* Avoid `auto` whenever possible. Using it is tolerated for complex types such as iterators
* Indent with **4 spaces**
* Every brace (`{`, `}`) must be on its own line
* Conditions with a single statement (`if (condition) do_this();`) do not need to be enclosed in braces
* Put a space between `for`, `while`, `if` and `(...)`, around each `=` sign (wherever it is, even in for-loops), between `#include` and the file to include
* Prefer `enum class` over `enum`
* Case
    * Methods and functions: camelCase
    * Variables: snake_case
    * Constant expressions: PascalCase
    * Enumeration members: PascalCase
* For-each loops should use const references or rvalue references instead of plain copies:
```cpp
// DEPRECATED
for (auto value : container)

// PREFERRED
for (const auto& value : container)
for (auto&& value : container)
```
* For-loops should be optimized whenever possible, as follows:
```cpp
// DEPRECATED
for (std::size_t i = 0; i < container.size(); ++i)
    ...

// PREFERRED
for (std::size_t i = 0, end = container.size(); i < end; i++)
    ...
```
* Header-guards should be written using `#ifndef`, `#define` and `#endif`, the define being in MACRO_CASE
* Functions and methods which can not throw exceptions must be marked `noexcept`
* In header files, have a blank line between each method / function / structure / constant ; also put blanks line around includes blocks
* In classes, avoid using `this`. Prefix every member variable with `m_`
* In classes, do not declare the methods in the header file, only in the source file (unless they are `inline` or templated)
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
         */
        void aMethod();
        
    private:
        int m_member;  ///< This is a doxygen comment
    };
}

#endif
```

## ArkScript coding guidelines

Check out the [ArkScript RFC 001](https://github.com/ArkScript-lang/rfc/blob/master/001-coding-guidelines.md). We are quite flexible about it, those are just general guidelines to produce a readable ArkScript code.
