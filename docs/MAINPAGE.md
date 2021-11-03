@mainpage Home

This is the **implementation** documentation of the ArkScript programming language, if you want to contribute to it in any way.

# Key features of the language

* small: the compiler and the virtual machines fit under 8000 lines, but also small in terms of keywords (it has only 10)!
* a scripting language: it's very easy to embed it in your application. The builtin construction is quite easy to understand, so adding your own functions to the virtual machine is effortless
* portable: it produces a bytecode which is run by its virtual machine, the same way Java does it (with a smaller memory footprint)
* a functional language: every parameter is passed by value, everything is immutable unless you use `mut` to specify your need of mutability
* powerful: it can handle object-oriented programming in a very elegant way with its closures and explicit captures (see `examples/closures.ark`)
* promoting functionalities before performances: expressiveness often brings more productivity, even though performances aren't left behind
* easy to compile: it takes less than 200ms to compile and check a complex code with a lot of branches and sub-branches of 200 lines
* a Lisp-like, but with fewer parentheses: `[...]` is expanded to `(list ...)` and `{}` to `(begin ...)`. More shorthands will come in the future
* extensible: it is very easy to create a C++ module to use it in the language, adding functionalities

Also it has:
* macros: if/else, values, and functions
* a REPL with autocompletion and coloration
* a growing standard library, composed of ArkScript code (under `lib/std/`) and C++ (under `lib/ext/`)
* a lot of unit tests (but never enough), which are ran before every release to ensure everything work as intended
* docker images to try the language without compiling it:
    * [stable](https://hub.docker.com/r/arkscript/stable), images built after each release
    * [nightly](https://hub.docker.com/r/arkscript/nightly), image built after each commit

# Links

- [Source code](https://github.com/ArkScript-lang/Ark)
- @ref tutorials
- [Main website](https://arkscript-lang.github.io)
