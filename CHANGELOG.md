# Change Log

## Unreleased
### Added
- option in the CMakeLists.txt to use MPIR or not (defaults to no MPIR)

### Changed
- using a vector instead of a map in the `Frame` to speed up things
- using double or MPIR depending on the compilation options

### Removed

# 2.1.0
### Added
- adding `switch` and `defer1` in the standard library (`defer1` shall be rewritten using `quote`)
- keyword `quote`, macro version is `\``
- added VM::get<T>(name) to retrieve values from the top stack of the Virtual Machine. Types currently supported are `Ark::BigNum`, `std::string` and `bool`
- adding `mod` in the standard library
- module `sfml`, need the SFML 2.5
- adding `@`, `and`, `or` and `headof` in the FFI
- adding a guard in the VM to be sure the builtins are all present in the VM FFI

### Changed
- the frames stack is handled differently, using shared pointers to avoid unecessary copies of frame's environments, it improves execution speed by *a lot*
- new CLI
- handling floating pointer numbers and rational numbers

### Removed

## 2.0.0
### Added
- configure.py script, to download, build and install mpir 3.0.0
- builtins functions: input, toNumber, toString
- **breaking change** adding `PLUGIN_TABLE_START` with a value of 3 in the compiler/VM
- adding plugins management

### Changed
- splitted lib/Exceptions.ark into lib/Exceptions.ark and lib/Either.ark
- renamed FindGMP FindMPIR, and we're now searching for MPIR and linking with it
- proper exception handling
- the VM shouldn't throw a runtime error if it can't link a function name and a function address
- **breaking change** the `CODE_SEGMENT_START` is now equal to 4
- fixing a bug in the bytecode reader: it didn't handle `NOP`
- `import` should be able to load plugins, also `import` takes only one argument now
- **breaking change** `POP_JUMP_IF_FALSE` is now an absolute jump as well
- upgrading CMakeLists to add `-rpath` option to the linker (with GCC), so that it still finds the lib after being installed

### Removed
- `hastype` keyword because I never had to implement compile time typechecking, so it's not useful

## 1.2.2
### Added
- adding `import` keyword (handled by parser), throwing an error if a cyclic included is detected

### Changed
- CMakeLists.txt to add `install` rules: installing Ark in bin/ and the Ark standard library in share/.Ark/lib/
- updated documentation

## 1.2.1
### Added
- runtime typechecking
- exceptions (in the C++ Ark API)

### Changed
- updated the FFI to add the runtime typechecking
- micro optimization: using numbers as variable names internally, instead of strings

### Removed
- unnecessary destructors removed to let the compiler auto generate T(T&&) (to avoid implicitly using T(const T&))

## 1.2.0
### Added
- syntactic sugar handling in the parser
- GMP lib to handle very large number
- REPL (can be launched from the CLI)
- tests

### Changed
- changed syntax: using `{...}` as a `(begin ...)` and `[...]` as a `(list ...)`
- updated documentation according the new syntax
- the lexer is now using a Token structure to store the line and column as well as the token itself
- generating the FFI using include/Ark/MakeFFI.hpp, everything defined in one file to avoid having 2 files to update
- tests

### Removed
- dozerg::HugeNumber, it was too slow

## 1.1.0
### Added
- test.cpp to try to embed Ark into a C++ project
- updated the documentation
- the compiler can now return a read only version of the bytecode being executed
- the VM can take a bytecode or a filename
- *OOP* test with Ark using closures
- closures support
- Types.hpp (for the VM) to store the definitions of the NFT (Nil/True/False enum class) and the PageAddr_t
- Function.hpp to get a lambda from the interpreter and call it from C++ code

### Changed
- CMakeLists.txt, adding an option to chose between compiling main.cpp or test.cpp
- moved the VM FFI into include/Ark/VM

## 1.0.0
## Added
- beginning of the documentation
- compiler (ark code to ark bytecode)
- bytecode reader (human readable format)
- dozerg::HugeNumber to handle big numbers
- simple VM handling all the instructions, able to run an ark bytecode
- interpreter and VM FFI
- logger

## 0.1.0
### Added
- Node (to represent an AST node and a Node in the language)
- Environment to map variables and values
- Program executing Ark code from the AST
- standard library (builtin functions)
- Lexer and parser
- default CLI can handle the interpreter
- tests
- utils to play with files

## 0.0.1
### Added
- utils to play with strings and numbers
- default CLI (using clipp)
- CMakeLists to compile the project
- ryjen::format to format strings