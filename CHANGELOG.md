# Change Log

## Unreleased changes
### Added

### Changed
- moved the lib files in subfolders to be more organized
- re-updating the import rules on the parser side to be able to import files in subfolders from the standard library
- updating naming convention of the modules

### Removed

## 3.0.6-b
### Added
- adding `lib/Functional.ark` to store `(compose f g)`

### Changed
- the VM now deletes all scopes except the global one in case of failure, when the persisting flag is set
- fixing plugin importation

## 3.0.6
### Added
- function arity handling in the VM, can be disabled with the option `-fno-function-arity-check`
- `sliceStr` in `lib/Slice.ark`, taking a string, a starting index (can't be less than 0), and the length (can't be less than 1), returning a portion of the given string
- `reverseStr` in `lib/Reverse.ark` taking a string and returning it in reverse

### Changed
- updated the include module, which was randomly adding `/` before the path of the files
- dispatched the unit tests in multiple files
- now using a separated class to hold the description values for the VM
- updated assertions in `split` in `lib/Split.ark` (works only with single character separators)
- fixing import bug

## 3.0.5
### Added
- the parser can now recognize expressions like `((f f) x)`
- we can now create `Ark::Value` with floats

### Changed
- the `init()` internal method of the VM shouldn't stop when a binded function isn't used in the code, just ignore it
- the compiler shouldn't crash on a bad code block
- better line counter in lexer
- the VM shouldn't try to run a non-existing bytecode file if the compilation fails (when calling `doFile()`)
- `VM.call` should return `nil` if the stack is empty, otherwise it results in a `vector subscript out of range` and that's bad
- the SFML plugin was updated to run on Windows

## 3.0.4
### Added
- with the option `-L|--lib` we can set the path to the Ark standard library
- we can now load C++ lambdas into the ArkVM, as well as C++ functions
- adding function `sleep`, taking a duration in milliseconds
- adding function `system`, taking a command from a string ; can be deleted if `ARK_ENABLE_SYSTEM` is set to 0 in the CMakeLists

### Changed
- updating CMakeLists.txt to avoid building unuseful stuff from google benchmark
- `toNumber` doesn't raise an unrecoverable error but returns `nil` if the conversion failed
- `print` no longer add a space between each element
- updating the way the builtins are handled (in VM/FFI) to make it easier to add functions

### Removed
- `doc` folder, now everything is on the wiki

## 3.0.3
### Added
- should be able to compare lists
- chained operators: `(+ 1 2 3)` is automatically expanded (at compile time) into `(+ (+ 1 2) 3)` by the compiler

### Changed
- some functions playing with list should also be able to play with Strings: `headof`, `tailof`, `firstof`, `len`, `empty?`, `@`
- `firstof` should segfault when the list/String is empty
- fixing type of `nil` to be `"Nil"` instead of `nil` when using `(type nil)`
- uniformised names of builtins: pascal case (impacted functions are `firstOf`, `headOf` and `tailOf`, as well as `hasField`)
- fixing bug with `writeFile` when sending a mode: the mode was also the content of the file, it no longer is

## 3.0.2
### Added
- cmake options `ARK_BUILD_EXE` and `ARK_BUILD_BENCHMARK` to choose what to build
- when the VM crash, displaying stack trace
- added function `time` to the FFI (time in seconds since epoch)
- adding VM.doFile

### Changed
- updated the VM to be able to call functions defined in Ark from C++
- `del sym` set `sym` to `undefined` (internal value only, not the `undefined` of Javascript) instead of `nil`
- fixed imports
- fixed automatic compilation of not-up-to-date files

### Removed
- flag `-c|--compile` to force compilation was not useful

## 3.0.1
### Added
- we can now call functions captured by closures, inside the scope of the closure, using the dot notation

### Changed
- the CLI is checking the timestamp of the file to know if it should recompile it or not
- the CLI knows if it should recompile the given file or not

## 3.0.0
### Added
- adding `del` and `mut` keywords. Now `let` is for settings constants and `mut` for variables. Also it isn't possible to use `let` to define the same constant twice
- `google/benchmark` library for the benchmarks
- Ark version section in bytecode
- timestamp (build date)
- major versions of the compiler and the virtual machine used must match, a compatibility accross versions will be kept if they have the same major number
- many opcodes to handle the operators
- persist flag for the VM (if persist is false (default value), each time we call vm.run(), the frames will be reseted)
- adding captures through functions arguments: `(fun (&captured std-argument) (...))`
- adding closure fields reading (readonly)

### Changed
- moved everything from the "folder namespaces" to a single `Ark::internal` namespace
- using `#` instead of `'` for the comments, using `'` to quote instead of `` ` ``
- the lexer is now detecting the type of the tokens it's playing with
- using `std::runtime_error`s instead of `exit(1)` when an error occured
- the VM should throw an error if we try to use `set` on a constant
- we can avoid passing all the arguments to a function, they will just be undefined
- the CLI is now able to determine if it should compile & run, run from the arkscript cache or run the file as a bytecode file

### Removed
- Lexer::check, we should see if the program is correct when building the AST
- removed from the bytecode `NEW_ENV`

## 2.2.0-dev
### Added
- option in the CMakeLists.txt to use MPIR or not (defaults to no MPIR)
- information about the compilation options used for Ark in the CLI
- we can now use `` ` `` to quote

### Changed
- using a vector instead of a map in the `Frame` to speed up things
- using double or MPIR depending on the compilation options
- moving `mod` in the FFI
- renamed methods in `Node`

### Removed
- `Defer.ark` from the standard library
- supporting both BigNum and double is a bad idea, using only double now
- removed the interpreter

## 2.1.0-dev
### Added
- adding `switch` and `defer1` in the standard library (`defer1` shall be rewritten using `quote`)
- keyword `quote`, macro version is `` ` ``
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

## 2.0.0-dev
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

## 1.2.2-dev
### Added
- adding `import` keyword (handled by parser), throwing an error if a cyclic included is detected

### Changed
- CMakeLists.txt to add `install` rules: installing Ark in bin/ and the Ark standard library in share/.Ark/lib/
- updated documentation

## 1.2.1-dev
### Added
- runtime typechecking
- exceptions (in the C++ Ark API)

### Changed
- updated the FFI to add the runtime typechecking
- micro optimization: using numbers as variable names internally, instead of strings

### Removed
- unnecessary destructors removed to let the compiler auto generate T(T&&) (to avoid implicitly using T(const T&))

## 1.2.0-dev
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

## 1.1.0-dev
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

## 1.0.0-dev
## Added
- beginning of the documentation
- compiler (ark code to ark bytecode)
- bytecode reader (human readable format)
- dozerg::HugeNumber to handle big numbers
- simple VM handling all the instructions, able to run an ark bytecode
- interpreter and VM FFI
- logger

## 0.1.0-dev
### Added
- Node (to represent an AST node and a Node in the language)
- Environment to map variables and values
- Program executing Ark code from the AST
- standard library (builtin functions)
- Lexer and parser
- default CLI can handle the interpreter
- tests
- utils to play with files

## 0.0.1-dev
### Added
- utils to play with strings and numbers
- default CLI (using clipp)
- CMakeLists to compile the project
- ryjen::format to format strings
