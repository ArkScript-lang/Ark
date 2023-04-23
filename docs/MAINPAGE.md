@mainpage Home

This is the **implementation** documentation of the ArkScript programming language, if you want to contribute to it in any way.

# Key features of the language

ArkScript is
* **small**: the core fit under 8000 lines of code ; also small in terms of keywords (only 10)
* **a scripting language**: very easy to embed it in your projects. Registering your own functions in the language is made easy
* **portable**: a unique bytecode which can be run everywhere the virtual machine is
* **a functional language**: every parameter is passed by value, everything is immutable unless specified
* **powerful**: provides closures and explicit capture
* **promoting functionalities before performances**: expressiveness often brings more productivity, though performances aren't left behind
* **a Lisp inspired language**, with fewer parentheses: `[...]` is expanded to `(list ...)` and `{}` to `(begin ...)`
* **extensible**: supports C++ module to use it in the language, adding functionalities

Also it has:
* **macros**: if/else, values, and functions
* tail call optimization
* a REPL with autocompletion and coloration
* a growing standard library, composed of ArkScript code (under `lib/std/`) and C++ (under `lib/ext/`)
* a lot of unit tests (but never enough), which are ran before every release to ensure everything works as expected
* docker images:
    * [stable](https://hub.docker.com/r/arkscript/stable), built after each release
    * [nightly](https://hub.docker.com/r/arkscript/nightly), built after each commit

# Guidelines

- @subpage guidelines_coding "Coding guidelines in ArkScript"
- @subpage guidelines_naming "Naming guidelines in ArkScript"
- @subpage guidelines_moduleserrorhandling "How to do error handling in your modules"

# Links

- [Source code](https://github.com/ArkScript-lang/Ark)
- @ref tutorials "Tutorials on the language"
- @ref implementation "Implementation details of the language"
- [Main website](https://arkscript-lang.github.io)
