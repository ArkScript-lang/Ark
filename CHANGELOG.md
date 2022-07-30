# Change Log

## [Unreleased]
### Added
- added new `async` and `await` builtins
     - they can access the outer scope
- added methods to create and destroy an execution context and a future in the VM
- added new CLI option `--ast` to generate JSON from the generated abstract syntax tree
- added an AST to JSON compiler

### Changed
- printing a closure will now print its fields instead of `Closure<1432>`
- macros are always evaluated, even when they aren't any user defined macro
- argcount works on symbols and anonymous functions

### Deprecated
- deprecating `Value VM::resolve(const Value* val, Args&&... args)`

### Removed
- removed the `std::ostream& operator<<` of the Value, now using the `.toString(stream, vm reference)`
- removed the global VM lock

## [3.3.0] - 2022-07-02
### Added
- running the modules tests in the CI
- new bytecode instruction `POP`, removing the last value from the stack
- more documentation about ArkScript and its modules
- more tests for the io builtins
- added lines and code coloration in the error context
- added documentation about the compiler implementation
- added documentation about the virtual machine
- ArkScript now supports LTO if the compiler can do it
    - this is disabled in GCC 8 as [this causes a runtime crash](https://github.com/ArkScript-lang/Ark/pull/385#issuecomment-1163597951) due to an ABI breakage

### Changed
- fixed underline bug in the error context
- moved the frame counter of the VM to the ExecutionContext as this should be local to the context, not to the VM
- changing the way we count received arguments in arity / type errors for failed function call
- the CLI can now take a list of paths to the standard library, separated by ';'

## [3.2.0] - 2022-02-19
### Added
- running the modules tests in the CI
- new bytecode instruction `POP`, removing the last value from the stack
- the compiler can finally optimize tail calls
- suggesting symbols to the user when the compiler encounters an unbound symbol

### Changed
- the compiler can now remove unused values from the stack
- enhancing the compiler code
- debloating the parser methods argument's lists

## [3.1.3] - 2022-01-29
### Added
- adding an ExecutionContext to host the pointers (instruction, page, stack) and execution related structures (stack, locals, scopes), to ease the transition to a parallelized VM
    - the VM can have multiple independant context running on the same bytecode
- the VM now takes a reference to an `Ark::State` instead of a raw non-owning pointer
- adding `ARK_PROFILER_MIPS` to toggle instruction per second calculation
- adding new way to typecheck in builtins
- new CI build step now running valgrind to check for memory leaks
- new type checker (to be used by builtins)
- better type errors generation (with the list of arguments, if they are matching or not, and more)

### Changed
- splitting Utils.hpp into multiple files for easier maintenance and contextualisation
- reserving a default scope size of 3, which yields really good performance results compared to nothing being reserved
- upgrading the builtins error handling to use the `BetterTypeError`
- the VM now displays the debug info (ip, pp, sp) at the end of the backtrace instead of the beginning

### Removed
- `BetterTypeError` has been removed in favor of a type checker using templates and an error generator

### Deprecated
- deprecating `VM(State*)` in favor of `VM(State&)`

## [3.1.2] - 2021-11-02
### Added
- adding support for append_in_place, concat_in_place, pop_list and pop_list_in_place in the bytecode reader
- added `page_ptr(int)` in the compiler to replace `&page(int)`
- added literals `_u8` and `_u16`
- added table overflow detection in the compiler, to avoid creating unusable bytecode (checks if the symbols/values table is full or not)
- new Installer.iss (inno setup script) to generate a Windows installer
- new exceptions for type errors

### Changed
- using `doc_formatting.first_column` instead of `doc_formatting.start_column` when displaying the CLI help
- brand new cmake build system
- renaming `Ark/Config.hpp` to `Ark/Platform.hpp`
- refactored compiler handling of keywords
- removed `using Inst_t = uint8_t` in the compiler
- moved everything related to the AST in `Ark/Compiler/AST/`
- moved everything related to the macros in `Ark/Compiler/Macros/`
- renamed unclear file `CValue` to `ValTableElem`
- the parser is now an internal class
- the AST Optimizer was moved to `Compiler/AST`
- changed the ARKSCRIPT_PATH to be a collection of paths to look into, separated by `;`
- updating replxx to avoid a bug when compiling with clang

### Removed
- removed `ARK_SCOPE_DICHOTOMY` flag so that scopes don't use dichotomic search but a linear one, since it proved to be faster on small sets of values. This goes toward prioritizing small functions, and code being cut in multiple smaller scopes
- removing `download-arkscript.sh` from the repo
- removed `isFraction`, `isInteger`, `isFloat` from Ark/Utils.hpp (worked on strings and used regex)
- removed mpark variant to use standard variant
- `Ark::FeatureFunctionArityCheck` was removed, making arity checks mandatory

## [3.1.1] - 2021-09-19
### Added
- ArkDoc documentation for the builtins
- Now using clang-format to ensure the code is correctly formatted

### Changed
- the macro processor can now handle multiple macro definitions in a if-macro: `!{if true { !{a 1} !{b 2} }}` is finally working

### Deprecated
- `ark` command is now marked as deprecated, in favor of `arkscript`

## [3.1.0] - 2021-06-29
### Added
- lists are mutated in place if they are mutable, through `append!` and `concat!`
- instructions for `pop` and `pop!` were added, to replace `list:removeAt`

### Changed
- `list:removeAt` was deprecated

## [3.1.0] - 2021-06-29
### Added
- adding of new string function for manipulation of utf8 string (str:ord and str:chr)
- utf8 support for lexer
- `UserType::del()`, used only by the virtual machine to free memory
- a new unique stack based on a `std::array<Value, ARK_STACK_SIZE>`, the default stack size being 8192
- more profiling tests
- more options on the `display()` method of the bytecode reader, allowing us to selecto segment of bytecode instead of displaying everything
- added a new token type: `Spread` ; it handles `...identifier`, needed in macros
- the parser can now handle macros definitions
- macros are being handled right after the parsing, before the AST optimizer can run
    - if macros: `!{if compile-time-value then [optional else]}`
    - values macros: `!{name value}`
    - functions macros: `!{name (a b c ...args) body}`
- `sys:platform`, containing the current platform name
- updated the CLI so that we can slice the bytecode when displaying it
- the bytecode reader can now display
    - all the segments
    - only the values segment
    - only the symbols segment
    - only the code segment (all of them or a given one)
    - only the segments' titles and length
- verifying that we give enough arguments
- we can now import macros from other files
- undefining macros is now possible by using `!{undef macro_name}`
- `str:join` added in the standard library
- `str:split` can now take longer separators
- added `symcat` in macros to concatenate a symbol and a number/string/symbol to create a new one
- added `argcount` in macros to count (at compile time) the number of arguments of a function
- fixed a bug where `(bloc)` and `(print bloc)`, given a `!{bloc value}` macro, didn't give the same result (one was applied, the other was partial)
- new module to manipulate bits: `bitwise`
- enhanced standard library

### Changed
- updating doxyfile and some docstrings
- updating the download script
- enhancing examples
- creating a Scope allocates 4 pairs instead of 2, reducing the number of reallocations needed
- `tailOf` renamed to `tail` and `headOf` to `head` ; no need to keep the relics of the past
- `headOf` (now `head`) returns the real head of a container (List or String), the first element (nil if the list is empty, "" if the string is empty)
- the http module was updated to add `http:params:toList` and fix the `http:server:[method]` when passing a function
- fixing the compiler when we encounter get fields in lists
- updating the parser to support usually invalid constructions when they are in macros, to allow things like `!{defun (name args body) (let name (fun args body))}`
- updated the lexer to add UTF8 support and allow unconventional identifiers as long as they aren't keyword nor operators, so things like `->` now works
- fixing the code optimizer to avoid removing unused variables which are defined on function calls
- fixed the traceback generation on errors, it should now display the correct function names
- reorganizing the compiler code
- reorganizing the parser code to make it more maintainable
- adding `make_node<T>` and `make_node_list` internally to avoid repetitive code
- enhancing the parser `atom` method
- enhancing the way we choose the subparser to use in the parser
- avoid using `std::endl` if it's not useful
- CI was split into multiple files to ease maintenance
- moving ArkScript tests from `tests/*.ark` to `tests/arkscript/*.ark`
- fixed macros adding useless begin blocks, sometimes breaking code generation from macros
- moving std lib related tests into std/tests/
- lists are mutated in place if they are mutable, through `append` and `concat`
- fixed macro chaining
- fixed lexer, which wasn't adding the last token it read under some specific conditions

### Removed
- `~UserType`, since we are doing manual memory management now
- `Frame` were removed because they were giving bad performances
- `firstOf` was removed because it's basically a `(@ list 0)` and it was doing the job of `head`
- `Ark::Utils::toString`, our internal version of `std::to_string`
- use of static in the MacroProcessor and in the NodeType to string conversion function
- `Ark::Logger` was removed in favor of `std::cout/cerr` + `termcolor`

## [3.0.15] - 2020-12-27
### Added
- new submodule, plasma-umass/coz (a profiler)
- macros for profiling, enabled only if `ARK_PROFILE` is defined
- cmake flags using -D to turn on/off sys:exec and the coz profiler
- mpark variant is now the default used instead of the default STL variant (faster, better, stronger, and its creator is quite a god)
- new cmake flag, -DARK_SCOPE_DICHOTOMY=On|Off (default Off)
- using internal only references to constants and symbols to reduce the number of useless copies of the value type

### Changed
- updated standard library
- updated modules, adding hash
- updated the error handlers to avoid errors (sigsev) when handling errors (lexing, parsing, optimization and compilation error)
- better error message at runtime when a plugin can not be found
- fixes issue #203 (imports are ill-formed when given an absolute path)
- fixes issue #205 (search for the standard library folder in more common places)
- transitioning from C++ streams to printf
- replaced the thirdparty/ folder with a git submodule in thirdparties/
- now checking that a scope doesn't have our symbol before doing a `mut` operation (in dichotomic mode it was automatically handled, but not in linear mode)
- enhancing the cmake defines (`-DARK_XYZ`) and the code using them
- lighter Frame (from 40B to 32B), moved some unrelated logic from the frame to the virtual machine
- `(sys:exec)` now returns the stdout output of the given command

## [3.0.14] - 2020-11-26
### Added
- the parser can handle `(let|mut a b.c)` (bug fix)
- `f[ruv|no-ruv]` CLI switch to control the optimizer (ruv stands for remove unused variables)
- error message when we have too many parenthesis (at parse time)
- error message when using an operator not right after a `(`
- error message when we're capturing an unbound variable
- added `(sys:exit code)` as a builtin
- bytecode integrity checking through a sha256 in the header
- tests for `math:fibo` and `math:divs`
- added the ability to give scripts arguments, through `sys:args`

### Changed
- the parser checks if set is given a dot expression as an identifier (which is an error)
- the parser should take in account captured variables as well, otherwise some variables are optimized while they are captured, resulting in runtime errors
- better unbound variable error message
- (implementation) every constructor with a single argument is now marked as explicit
- REPL does not need to add extra surrounding {}
- the Ark::State (re)compiles a file even if there is a bytecode version available
- the parser is now stricter and gives better error messages when we give too many/not enough arguments to a keyword
- better handling of the code given to the REPL (adds new line)
- renamed the executable from `Ark` to `ark`
- now using Github Actions instead of Travis
- the parser can now detect when let/mut/set are fed too many arguments, and generate an error
- the compilater now handles `(set a b.c.d)`
- using a new plugin interface, more C-like

### Removed
- class `Ark::internal::Inst` which was used as a wrapper between `uint8_t` and `Instruction`
- worthless examples were removed
- removing `f[no-]aitap` since it wasn't used anymore in the code

## [3.0.13] - 2020-10-12
### Added
- string tests
- list tests
- range tests
- unbound variable checker at compile time (won't break on plugin symbols)

### Changed
- `list:find` returns -1 to stay consistent with `str:find`
- **hot fix** `(mut a 10) (let b 12) (set a b) (set a 11)`, the immutability was transfered from b to a
- converting `list`, `append` and `concat` to instructions
- instructions `LIST` `CONCAT` and `APPEND` added to replace the corresponding builtins

## [3.0.12] - 2020-09-06
### Added
- using a macro to define the default filename (when none is given, eg when loading bytecode files or from the REPL)
- `PLUGIN <const id>` instruction to load plugin dynamically and not when the VM boots up
- updated search paths for `(import "lib.ark")`, looking in ./, lib/std/ and lib/
- added a case to display NOT instructions in the bytecode reader
- `T& as<T>()` in usertype
- enhanced error message when calling a non-function object
- eliminating unused global scope variables in the compiler
- adding a new feature enabled by default: `FeatureRemoveUnusedVars` (not enabled for the REPL for obvious reasons)
- added replxx as a submodule
- added custom destructor to the user type, called when a scope is destroyed and when we use `(del obj)`
- added a GVL (global virtual machine lock) to be able to use the VM in a multithreaded context
- dockerfile + specific github action to build and push stable and nightly docker images, thanks to @yardenshoham
- added guards to the bytecode reader to stop reading if we're missing an entry point; now telling the user about it

### Changed
- updated the string module to benefit from the new `format` member function
- updated the logger to remove `fmt/format`
- changed the argument order for `Ark::State`
- renamed the cache directory `__arkscript__`
- operator `@` can now handle negative indexes to get elements from the end of the given container
- the standard library is now in another repository
- moved the modules to lib/ext
- the value of `CODE_SEGMENT_START` is again 0x03 (because we removed the plugin table)
- renamed `isDir?` to `dir?` for consistency
- the lexer is no longer using regexes but a char after char method
- an ArkScript program is no longer a single bloc, but can be composed of multiple bloc, thus we don't need to use a single big {} or (begin) bloc for all the program
- enhancing lexer and parser error messages
- else clause in if constructions is now optional
- updating error messages in the VM
- updated the repl to add auto completion, coloration and persistance by @PierrePharel
- moving the parser, lexer and AST node to Compiler/ from Parser/
- better import error messages at parsing
- format can now handle any value type
- updated the tests to use the new standard library, and testing every VM instruction and builtins (we didn't test everything before, this way we can be sure we don't break *anything* in the VM after each update)
- renaming builtins to add a namespace to them (math:, sys:, str:, list: and such)
- firstOf, tailOf and headOf now returns [] or "" instead of nil when they have nothing to do
- adding a brand new scoping system, lighter, more powerful
- `str:find` now returns the index where the substring was found
- `str:removeAt` was fixed to throw an error when the index is strictly equal to the length of the string (can not work since accessing elements in string is 0-index based)

### Removed
- removed `fmt/format` from our dependencies
- `PLUGIN_TABLE` was removed to use the `PLUGIN` instruction
- `not_()` from usertype
- removed Parser/Utf8Converter

## [3.0.11] - 2020-06-21
### Added
- member function `resolve(Args&& args...)` to Value, callable by plugins to resolve the value of a function called with specific arguments given by the plugin
- `(fill qu value)` create a list of `qu` `value`s
- `(setListAt list at new-value)` modify a list in place and return the new list value
- adding UTF-8 support in programs (experimental)
- more benchmarks
- on error, the VM now display the value of each variable in the current scope
- added thirdparty/madureira/String, to replace std::string in Ark::internal::Value which was heavy and slower than the new implementation
- minimizing the size of the usertype

### Changed
- UserType does not need to be given a manually defined type id but relies on `typeid(T)`
- performance boost of the VM by using pointers to avoid unecessary copies
- renaming `isNaN` to `NaN?`, `isInf` to `Inf?` for uniformisation (see `empty?`, `nil?`)
- renaming CLI feature options:
    - `-ffunction-arity-check` becomes `-ffac`, same for the `-fno-` version
    - `-fauthorize-invalid-token-after-paren` becomes `-faitap`, some for the `-fno-` version
- improving compiler performances by using const ref when passing nodes around
- renaming the FFI "builtins" because it's not a FFI but a set of functions using the VM API
- the VM should display a backtrace even if an unknown error occured
- transforming inline code from the vm into not inline code when possible to speed compilation, using macros instead of inline functions
- smaller value class
- smaller vm frames
- forked madureira/String and modified it for the needs of the project, added it as a submodule
- removed the VM pointer from the value class to make it lighter, now the VM is sending a pointer of itself to the C procedures
- removed const and type from value, now using a uint8_t to store those informations

### Removed
- removed NFT from the internal API to rely only on the value type

## [3.0.10] - 2020-02-09
### Added
- adding `sort` to sort a list
- added `\t`, `\n`, `\v` and `\r` escape codes (available in strings only)
- adding `listFiles` which returns a list of paths (strings)
- adding `(makeDir path)` and `(removeFiles ...)`
- added `(filter func list)` in `lib/Functional/Functional.ark`
- adding `puts`. Does the same thing as print but doesn't print a newline character
- added a msgpack module by @PierrePharel
- added a user type (to be defined in C++)
- adding a `not` operator
- adding an http module by @SuperFola

### Changed
- updated output of tests with number of passed tests
- updated REPL so that it doesn't try to compile blank lines or comments, by @rstefanic
- the option `-d|--debug` is now repeatable, to set a debug level. Can go from 1 (compilation information) to 3 (a lot of information).
- better precision when using `(time)`
- better tests presentation
- moved the modules to https://github.com/ArkScript-lang/modules

## [3.0.9] - 2019-10-27
### Added
- fixing segfault when the VM receives an empty code page (generated from an empty block)
- `(print (fun () ()))` will now print `Function @ 1` instead of just its page addr, `1`
- `(if true () ())` now returns nil (can be generalized to `() -> nil`)
- anonymous functions are now available ; eg: `((fun () (print "a")))`
- added `forEach` in library
- added `-c|--compile` option to the CLI so that we can only compile an ArkScript file instead of compiling and running it, by @DontBelieveMe
- added `min` and `max` in `lib/Math/Arithmetic.ark`, by @FrenchMasterSword
- added `reduce` in `lib/Functional/Reduce.ark`, by @FrenchMasterSword
- added `product` in `lib/List/Product.ark`, by @FrenchMasterSword

### Changed
- a quoted code (defered evaluation) isn't capturing anymore surrounding variables' values, thus increasing greatly performances
- lists are printed like `["string" 1 true]` now, instead of `( string 1 true )`
- updated `zip` so that it can work with lists of different sizes, by @FrenchMasterSword
- better cyclic includes detection
- better VM error message when redefining a variable through `let`

## [3.0.8] - 2019-10-22
### Added
- it's now possible to compare Values using `operator<`
- `reverseList` (added to the FFI) by @rinz13r
- a warning will now pop up during compilation if code appears to be ill-formed (eg number/string/list right after a `(`)
- option `-f(allow-invalid-token-after-paren|no-invalid-token-after-paren)` was added (default: OFF)

### Changed
- the internal API used to compare values has been updated to be much shorter
- the REPL can take into account the lib directory, by @rstefanic
- `isNaN` and `isInf` should work on any type but return false if they aren't numbers
- replacing Ark with ArkScript in source code and files (Ark being the shortname for ArkScript, as JS is the shortname for Javascript)
- `findInList` now returns `nil` when the object can not be found, otherwise it returns its index in the list

## [3.0.7] - 2019-10-15
### Added
- `cos`, `arccos`, `sin`, `arcsin`, `tan`, `arctan`
- `E` (exp 1), `Pi`, `Tau` (2 * Pi), `NaN`, `Inf`
- `isNaN`, `isInf`
- `exp`, `ln` (standard logarithm), `pow`, `sqrt`
- `ceil`, `round`, `floor`
- `zip`, `map`, `sum`
- REPL, made by @rstefanic

### Changed
- moved the lib files in subfolders to be more organized
- re-updating the import rules on the parser side to be able to import files in subfolders from the standard library
- updating naming convention of the modules

## [3.0.6-b] - 2019-10-09
### Added
- adding `lib/Functional.ark` to store `(compose f g)`

### Changed
- the VM now deletes all scopes except the global one in case of failure, when the persisting flag is set
- fixing plugin importation

## [3.0.6] - 2019-10-07
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

## [3.0.5] - 2019-10-04
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

## [3.0.4] - 2019-09-01
### Added
- with the option `-L|--lib` we can set the path to the ArkScript standard library
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

## [3.0.3] - 2019-08-23
### Added
- should be able to compare lists
- chained operators: `(+ 1 2 3)` is automatically expanded (at compile time) into `(+ (+ 1 2) 3)` by the compiler

### Changed
- some functions playing with list should also be able to play with Strings: `headof`, `tailof`, `firstof`, `len`, `empty?`, `@`
- `firstof` should segfault when the list/String is empty
- fixing type of `nil` to be `"Nil"` instead of `nil` when using `(type nil)`
- uniformised names of builtins: pascal case (impacted functions are `firstOf`, `headOf` and `tailOf`, as well as `hasField`)
- fixing bug with `writeFile` when sending a mode: the mode was also the content of the file, it no longer is

## [3.0.2] - 2019-08-22
### Added
- cmake options `ARK_BUILD_EXE` and `ARK_BUILD_BENCHMARK` to choose what to build
- when the VM crash, displaying stack trace
- added function `time` to the FFI (time in seconds since epoch)
- adding VM.doFile

### Changed
- updated the VM to be able to call functions defined in ArkScript from C++
- `del sym` set `sym` to `undefined` (internal value only, not the `undefined` of Javascript) instead of `nil`
- fixed imports
- fixed automatic compilation of not-up-to-date files

### Removed
- flag `-c|--compile` to force compilation was not useful

## [3.0.1] - 2019-07-25
### Added
- we can now call functions captured by closures, inside the scope of the closure, using the dot notation

### Changed
- the CLI is checking the timestamp of the file to know if it should recompile it or not
- the CLI knows if it should recompile the given file or not

## [3.0.0] - 2019
### Added
- adding `del` and `mut` keywords. Now `let` is for settings constants and `mut` for variables. Also it isn't possible to use `let` to define the same constant twice
- `google/benchmark` library for the benchmarks
- ArkScript version section in bytecode
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

## [2.2.0-dev] - 2019-06-01
### Added
- option in the CMakeLists.txt to use MPIR or not (defaults to no MPIR)
- information about the compilation options used for ArkScript in the CLI
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

## [2.1.0-dev] - 2019-05-05
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

## [2.0.0-dev] - 02-05-2020
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

## [1.2.2-dev] - 2019-05-02
### Added
- adding `import` keyword (handled by parser), throwing an error if a cyclic included is detected

### Changed
- CMakeLists.txt to add `install` rules: installing ArkScript in bin/ and the ArkScript standard library in share/.Ark/lib/
- updated documentation

## [1.2.1-dev] - 2019
### Added
- runtime typechecking
- exceptions (in the C++ ArkScript API)

### Changed
- updated the FFI to add the runtime typechecking
- micro optimization: using numbers as variable names internally, instead of strings

### Removed
- unnecessary destructors removed to let the compiler auto generate T(T&&) (to avoid implicitly using T(const T&))

## [1.2.0-dev] - 2019
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

## [1.1.0-dev] - 2019
### Added
- test.cpp to try to embed ArkScript into a C++ project
- updated the documentation
- the compiler can now return a read only version of the bytecode being executed
- the VM can take a bytecode or a filename
- *OOP* test with ArkScript using closures
- closures support
- Types.hpp (for the VM) to store the definitions of the NFT (Nil/True/False enum class) and the PageAddr_t
- Function.hpp to get a lambda from the interpreter and call it from C++ code

### Changed
- CMakeLists.txt, adding an option to chose between compiling main.cpp or test.cpp
- moved the VM FFI into include/Ark/VM

## [1.0.0-dev] - 2019
## Added
- beginning of the documentation
- compiler (ark code to ark bytecode)
- bytecode reader (human readable format)
- dozerg::HugeNumber to handle big numbers
- simple VM handling all the instructions, able to run an ark bytecode
- interpreter and VM FFI
- logger

## [0.1.0-dev] - 2019
### Added
- Node (to represent an AST node and a Node in the language)
- Environment to map variables and values
- Program executing ArkScript code from the AST
- standard library (builtin functions)
- Lexer and parser
- default CLI can handle the interpreter
- tests
- utils to play with files

## [0.0.1-dev] - 2019
### Added
- utils to play with strings and numbers
- default CLI (using clipp)
- CMakeLists to compile the project
- ryjen::format to format strings
