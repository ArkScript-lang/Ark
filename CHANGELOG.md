# Change Log

## [Unreleased changes] - 2026-MM-DD
### Breaking changes

### Deprecated

### Added
- stdlib:
    - list:intersection
    - list:difference
    - sys:version, has the version of the ArkScript VM
- IR inliner, running after the AST lowerer, before the IR optimiser
- new debugger command `scopes`, printing the last `n` scopes (default: 5)

### Changed
- fix a bug related to recursive closures: once a closure was referenced in its own scope, we couldn't convert it to string or compare it against another closure/itself
- bytecode reader: the length of code segments is counted in instructions, not bytes
- the compiler can track the origin of an IR block, in preparation for inlining
- `POP_SCOPE` can take an argument to have a mode: by default, `0`, only pop the current scope ; if `1`, materialise the top of the stack if it's a reference
- the debugger does not trace the instructions of the code being run inside it (only instructions from the script are traced)

### Removed

## [4.7.1] - 2026-06-21
### Changed
- fixed a bug with `datetime:parse` and `datetime:parseAs` where some dates would be offset by a day

## [4.7.0] - 2026-06-18
### Added
- new builtin to extract the bytes of a string: builtin__string:bytes
- new builtins `builtin__time:asUTCDate` and `builtin__time:parseDate` to convert a timestamp to a date, and parse a string date as a timestamp
- standard library
  - math:fromBase to convert a number from a given base to base 10
  - string:surrogate? and string:privateUse? to check if a Unicode character is in one of those planes
  - list:findAfter to search for an element in a list after a given index
  - list:findIf to search for an element in a list using a predicate
  - list:search to search for a set of contiguous elements in a list
  - list:minMax to get the minimum and maximum values of a list of numbers in a single call
  - list:sorted? to check if a list is sorted
  - list:lowerBound, list:upperBound
  - list:binarySearch
  - list:shiftLeft, list:shiftRight
  - list:countOccurrences
  - std.Colours with CSS colours
  - datetime library
    - timezoneOffsets
    - utcOffsetMinutes
    - makeUTCTimestamp
    - timestamps: year0, year1970, year200
    - toUTCTimestamp
    - asUTCDate
    - plusSeconds, minusSeconds
    - plusMinutes, minusMinutes
    - plusHours, minusHours
    - plusDays, minusDays
    - plusWeeks, minusWeeks
    - plusMonths, minusMonths
    - plusYears, minusYears
    - atStartOfDay, atEndOfDay
    - today, yesterday, tomorrow
    - nextDay, previousDay
    - delta, asDelta
    - asSeconds
    - plusDelta, minusDelta
    - year, month, day, hour, minute, second, millisecond, dayOfWeek, dayOfYear
    - leapYear?
    - monthLength, yearLength
    - utcOffsetRepr
    - asISO8601
    - parse, parseAs
- new escape sequences: `\0xx` (octal escape sequences using 2 chars) and `\xyz` (hex escape sequences using 2 chars)
- ci: compile using visual studio 2022 and 2026

### Changed
- math:toBase handles 0 correctly
- math:countDigits returns 1 for 0
- builtin__list:find can take an optional third argument, to specify at which index to start looking from
- fixed formatting of function calls on multiple lines, unnecessary indents were added

## [4.6.0] - 2026-05-17
### Deprecations
- `std.Range` has been entirely deprecated and is scheduled for removal
- `bitwise` module is deprecated in favour of new builtins, included in `std.Math`

### Added
- standard library:
  - list:slice1
  - list:first
  - list:last
  - list:cumulativeSum
  - list:cumulativeProduct
  - list:zeros
  - list:ones
  - string:lpad
  - string:rpad
  - string:ascii?
  - string:first
  - string:last
  - string:codepoints
  - math:countOnes
  - math:countZeros
  - math:bitNot
  - math:bitAnd
  - math:bitOr
  - math:bitXor
  - math:lshift
  - math:rshift
  - math:bitCeil
  - math:bitFloor
  - math:bitWidth
  - math:countLeftZeros
  - math:countLeftOnes
  - math:countRightZeros
  - math:countRightOnes
  - math:circularLeftShift
  - math:circularRightShift
  - math:toBase
  - math:countDigits
- new experimental builtin `builtin__string:codepoints` returning the Unicode codepoints of a string as a list of numbers
- new bitwise builtins

### Changed
- `$repr` shows the correct representation for macros
- `$symcat` accepts symbols and strings as its first argument
- `math:pow` uses a more precise way of computing values when inputs are integers
- docker images use alpine 3.23 as a base instead of 3.21

## [4.5.2] - 2026-05-06
### Changed
- fix compiler segfault when there is no variable to mark as unreachable

## [4.5.1] - 2026-05-05
### Added
- special error message when trying to use a hidden symbol due to importing

### Changed
- avoid generating `LOAD_FAST_BY_INDEX` instructions when trying to fetch a potentially unreachable variable

## [4.5.0] - 2026-04-14
### Breaking changes
- removed `list:permutationsWithReplacement` (deprecated in 4.4.0)
- `list:permutations` now produces real permutations and not combinations (deprecated in 4.4.0)

### Added
- added new macro `$gensym`, to generate a unique symbol identifier to use in macros
- `append`, `concat`, and `pop` can be used as values
- new `ptr` command for the debugger, printing the VM pointers (ip, pp, sp)
- compile time arity check when performing a tail call
- `string:utf8len` to compute the number of codepoints in a string
- new `trace` command for the debugger, printing the last executed instructions
- ability to build ArkScript statically via CMake using `-DARK_STATIC=On`

### Changed
- all paths inside `if` should return a value, when used as an expression. If an `else` branch is missing, `nil` will be returned
- new compile time error when trying to use `append!`, `concat!`, `pop!`, `@=` and `@@=` as values
- arguments in tail calls are loaded by value and not by reference
- `string:ord` checks that it get only 1 utf8 character

## [4.4.1] - 2026-03-19
### Breaking changes
- in function calls, the function to call is now always evaluated first
- in function calls, the arguments are now evaluated from left to right

### Added
- the bytecode reader can print the argument of a `PUSH_RETURN_ADDRESS` instruction as a hex number
- new super instruction `CALL_SYMBOL_BY_INDEX` to optimise a `LOAD_FAST_BY_INDEX` followed by a `CALL`

### Changed
- instruction counter in the bytecode reader are displayed in hex, and count each instruction instead of each byte
- `let` / `mut` / `set` push a copy of their value when used as expression (instead of an internal reference)

### Removed
- removed a nearly never emitted `GET_CURRENT_PAGE_ADDR` instruction, since it's now always optimised with `CALL` into a `CALL_CURRENT_PAGE` instruction
- removed `list:size`, `dict:size`, `dict:contains`, `math:even` and `math:odd` since they were deprecated since ArkScript 4.2.0

## [4.4.0] - 2026-03-13
### Deprecations
- `list:permutations` is deprecated in favour of `list:combinations`
- `list:permutationsWithReplacement` is deprecated in favour of `list:combinationsWithReplacement`

### Added
- new debugger commands: `stack <n>` and `locals <n>` to print the values on the stack and in the current locals scope
- custom format specifiers for lists:
  - `:n` to remove surrounding brackets,
  - `:c` / `:nc` to use `, ` as a separator instead of ` `,
  - `:l` / `:nl` to use `\n` as a separator,
  - `:?s` to format as an escaped quoted string,
  - `:s` to format as a quoted string
- `format` can use format specifiers for integers: `b`, `#b`, `B`, `#B`, `c`, `d`, `o`, `x`, `#x`, `X`, and `#X` if the argument is an integer
- display a warning to `stderr` when using a deprecated function/value (checks for `@deprecated` inside the attached comment of functions / values)

### Changed
- `pop!` can return the removed value
- `@=` and `@@=` return the inserted value
- `append!` and `concat!` return the modified list
- `let`, `mut` and `set` can return the assigned value
- fix formatter: when the condition of a `while` loop is on multiple lines, add the right amount of indentation before it

## [4.3.3] - 2026-03-01
### Changed
- runtime type checking errors are on stderr instead of stdout
- runtime exceptions are on stderr instead of stdout

## [4.3.2] - 2026-02-28
### Changed
- VM error outputs are on stderr instead of stdout

## [4.3.1] - 2026-02-28
### Added
- new `TAIL_CALL_SELF` instruction to take care of tail calls in functions: jumps to address 0 in the current page, and reset the scope

### Changed
- error outputs are on stderr instead of stdout

## [4.3.0] - 2026-02-26
### Breaking change
- in macros, `len`, `empty?`, `head`, `tail`, `@` have been renamed to `$len`, `$empty?`, `$head`, `$tail` and `$at`. Those versions only work inside macros too, inside of having a weird dichotomy where they sometimes got applied and sometimes not

### Added
- `apply` function: `(apply func [args...])`, to call a function with a set of arguments stored in a list. Works with functions, closures and builtins
- `+`, `-`, `*`, `/` and many other operators can now be passed around, like builtins. This now works: `(list:reduce [1 2 3] +)`, where before we would get a compile time error about a "freestanding operator '+'"
- `builtin__slice` builtin, for strings and lists: `(builtin__slice data start end [step=1])` ; **this is an experimentation and may be removed in future versions**
- arguments of builtin macros are properly type-checked and will now raise runtime errors if the type is incorrect
- `-fno-cache` cli option to disable the creation of the bytecode cache folder `__arkscript__`
- in the CLI, `file` can be `-` to read code from stdin

### Changed
- when using the cli flag `-fdump-ir`, the IR is dumped in the cache folder

## [4.2.0] - 2026-02-04
### Breaking changes
- `assert` is no longer an instruction but a builtin
- when comparing values of different types using `<`, `>`, `<=`, `>=` and `=`, the result will always be `false` (it used to rely on the type index)

### Added
- added `BREAKPOINT` instruction
- breakpoints can be placed using `(breakpoint)` and `(breakpoint condition)`
- added a debugger that can be triggered on error or on breakpoint by passing `-fdebugger` to the CLI (see [the docs for the debugger](https://arkscript-lang.dev/docs/tutorials/debugging/))
- diagnostics can now be generated when using `State.doString`, using `Diagnostics::generateWithCode` (as the original code must be passed to the diagnostics generator)
- `empty?` can now take a dict, and returns `true` if it has no key/value pairs
- `len` can now work on dictionaries, counting the number of keys

### Changed
- changed the runpath of `arkscript` to look for `libArkReactor` under its (arkscript's) directory, {arkscript}/bin, {arkscript}/lib, and {arkscript}/../lib
- `and` and `or` require valid expressions, so `(or 1 (mut x 3))` is no longer valid code, as `(mut x 3)` doesn't return a value
- `(not (dict "a" 2))` now returns `false`, as `not` checks if the dict is empty

## [4.1.2] - 2026-01-09
### Added
- the repl prints the output of the last expression it ran
- new super instructions: `MUL_BY`, `MUL_BY_INDEX`, `MUL_SET_VAL` that can do multiplications (and optional storing in vars) in place
- new super instruction: `FUSED_MATH`, which can fuse 2 to 3 math operations in one go (ADD, SUB, MUL, DIV)
- new `LOAD_SYMBOL` instruction that avoids creating a reference

### Fixed
- the REPL doesn't color `import` in two colours (red for `imp__t` and blue for `___or_`), it keeps the first colour that matched (red for import here)
- page numbers are correctly counted when using the bytecode reader with '--only-names', instead of displaying `0` every time

### Changed
- quotes are added around strings in type errors
- `disassemble` can show a file bytecode
- `empty?` now accepts `nil` and returns `true` for this value
- the REPL adds `(repl:history)` and `(repl:save filename)` as builtins
- the REPL attempts to load a file from `ARKSCRIPT_REPL_STARTUP` environment variable, to preload code
- rename LOAD_SYMBOL and LOAD_SYMBOL_BY_INDEX to LOAD_FAST and LOAD_FAST_BY_INDEX to emphasise they load refs

## [4.1.1] - 2025-12-13
### Fixed
- the formatter was breaking functions' arguments list containing argument attributes on multiple lines for no reason
- the formatter was formatting begin nodes inside conditions badly, putting the `{` on the same line as the condition, making it hard to know if the condition had `then` and `else` nodes or a single multi nodes `then` node

### Changed
- long function calls are split on multiple lines

## [4.1.0] - 2025-12-12
### Breaking changes
- Function arguments are now immutable by default and an argument attribute `mut` must be added: `(fun (a b c) (set b 5))` -> `(fun (a (mut b) c) (set b 5))`

### Deprecated
- `dict:contains`, use `dict:contains?`
- `math:even`, use `math:even?`
- `math:odd`, use `math:odd?`

### Added
- new builtin `disassemble` to print the bytecode of a function
- new builtin `io:readFileLines` to read lines from a file as a list of strings

### Changed
- the formatter properly formats dictionaries (key-value pairs on their own line, always)
- renamed `dict:contains` to `dict:contains?` so that all functions returning booleans have `?` suffix ; added temporary alias `dict:contains`
- renamed `math:even` to `math:even?`, and `math:odd` to `math:odd?`
- `string:removeAt` can work with negative indexes

## [4.0.0] - 2025-09-12
### Added
- more tests for the io builtins
- added lines and code coloration in the error context
- new dependency: `fmtlib`
- added the padding/instruction/argumentation values when displaying instructions in the bytecode reader
- `$repr` macro to get a string representation of a given node
- added boost-ext/ut to write unit tests in C++
- basic ArkScript code formatter, available through the CLI: `arkscript -f|--format`
- comments are now tracked in the AST and attached to the nearest node below them
- `VM::forceReloadPlugins`, to be used by the REPL to force reload the plugins and be sure that their symbols are all define
- added `help`, `save` (save history to disk), `history` (print history), `reset` (reset vm and code) commands to the REPL
- REPL can now show when a code block isn't terminated (prompt changes from `>` to `:`)
- more controls available inside the REPL
- fuzzing step in the CI
- better error reporting on unknown import
- check on number of arguments passed to `type`
- warning when the formatter deletes comment(s) by mistake
- check on arguments passed to `list`, `concat`, `append` and friends to only push valid nodes (that produces a value)
- introduced `Ark::internal::Pass` to describe compiler passes: they all output an AST (parser, import solver, macro processor, and optimiser for now)
- add `-f(no-)importsolver`, `-f(no-)macroprocessor` and `-f(no-)optimizer` to toggle on and off those compiler passes
- added resolving `empty?` as a macro when possible
- added short-circuiting to `and` and `or` implementation
- added `--check` to the formatter as an option: returns 0 if the code is correctly formatted, 1 otherwise
- the name & scope resolution pass now checks for mutability errors
- compile time checks for mutability errors with `append!`, `concat!` and `pop!`
- new `MAKE_CLOSURE <page addr>` instruction, generated in place of a `LOAD_CONST` when a closure is made
- added `-fdump-ir` to dump the IR entities to a file named `{file}.ark.ir`
- added 11 super instructions and their implementation to the VM
- support for the glob import syntax and symbol import syntax
- modify list and return a copy `(string:setAt string index char)` (bound checked)
- added in place list mutation: `(@= list|string index new_value)`, `(@@= list|list<string> index1 index2 new_value|char)` (bound checked)
- compile time argument count check for `and` and `or`
- basic dead code elimination in the AST optimizer
- new operator `@@` to get elements in list of lists / list of strings
- new builtin `random`, returning a random number between INT_MIN and INT_MAX, or in a custom range
- `$as-is` to paste a node inside a maro without evaluating it further ; useful to stop recursive evaluation of nodes inside function macros
- `LOAD_SYMBOL_BY_INDEX` instruction, loading a local from the current scope by an index (0 being the last element added to the scope)
- `STORE_FROM_INDEX` and `SET_VAL_FROM_INDEX` instructions for parity with the super instructions not using load by index
- `INCREMENT_BY_INDEX` and `DECREMENT_BY_INDEX` instructions for parity with the super instructions not using load by index
- `STORE_TAIL_BY_INDEX`, `STORE_HEAD_BY_INDEX`, `SET_VAL_TAIL_BY_INDEX`, `SET_VAL_HEAD_BY_INDEX` super instructions added for parity with the super instructions not using load by index
- `RESET_SCOPE_JUMP` instruction emitted at the end of a while loop to reset a scope so that we can create multiple variables and use `LOAD_SYMBOL_BY_INDEX`
- instruction source location ; two new bytecode tables were added: one for filenames, another for (page pointer, instruction pointer, file id, line), allowing the VM to display better error messages when the source is available
- show source location when a runtime error is thrown in the VM
- `LT_CONST_JUMP_IF_FALSE` and `LT_SYM_JUMP_IF_FALSE` to compare a symbol to a const and a symbol to a symbol (respectively), then jump to an address if false (useful for while loops that check a simple `(< x n)` condition)
- `LT_CONST_JUMP_IF_TRUE`, counterpart of `LT_CONST_JUMP_IF_FALSE`
- `GT_CONST_JUMP_IF_TRUE`, counterpart of `LT_CONST_JUMP_IF_TRUE`
- `GT_CONST_JUMP_IF_FALSE`, counterpart of `LT_CONST_JUMP_IF_FALSE`
- `GT_SYM_JUMP_IF_FALSE`, counterpart of `LT_SYM_JUMP_IF_FALSE`
- `CALL_SYMBOL` super instruction to load and call a symbol in a single instruction
- `GET_FIELD_FROM_SYMBOL` and `GET_FIELD_FROM_SYMBOL_INDEX` super instructions to get a field from a closure and push it to the stack
- `EQ_CONST_JUMP_IF_TRUE` and `EQ_SYM_INDEX_JUMP_IF_TRUE` to compare a symbol to a const and a symbol to a symbol (respectively), then jump to an address if true (useful for conditions that check a simple `(= x n)` condition)
- `NEQ_CONST_JUMP_IF_TRUE` as a super instruction counterpart to `EQ_CONST_JUMP_IF_TRUE`
- `NEQ_SYM_JUMP_IF_FALSE`, counterpart of `LT_SYM_JUMP_IF_FALSE` for inequality
- `AT_SYM_SYM` and `AT_SYM_INDEX_SYM_INDEX` super instructions, to get an element from a list in a single instruction, avoiding 2 push and 2 pop
- `CHECK_TYPE_OF` and `CHECK_TYPE_OF_BY_INDEX` super instructions, to check the type of variable against a constant in a single instruction
- `INCREMENT_STORE` and `DECREMENT_STORE` super instructions, to update a value in place when incrementing/decrementing it by a set amount
- `APPEND_IN_PLACE_SYM` and `APPEND_IN_PLACE_SYM_INDEX` super instructions
- `PUSH_RETURN_ADDRESS` instruction now replaces the VM auto push of IP/PP
- remove the stack swapping by pushing arguments in the reverse order by which they are loaded
- wasm export: we can now run ArkScript code on the web!
- `GET_CURRENT_PAGE_ADDRESS` instruction to push the current page address to the stack
- `CALL_CURRENT_PAGE` super instruction, calling the current page with a given number of arguments (avoid loading a page address on the stack, then popping it to perform the call)
- new data type `Dict`, which can be created with `(dict "key" "value" ...)`, and manipulated with `dict:get`, `dict:add`, `dict:contains`, `dict:remove`, `dict:keys` and `dict:size`
- added program name under `builtin__sys:programName
- `STORE_LEN` super instruction, to load a symbol by index and store its length (if it's a string or list) in a new variable
- `AT_SYM_INDEX_CONST` super instruction, to load a value from a container using a constant as the index

### Changed
- instructions are on 4 bytes: 1 byte for the instruction, 1 byte of padding, 2 bytes for an immediate argument
- enhanced the bytecode reader and its command line interface
- added the padding/instruction/argumentation values when displaying instructions in the bytecode reader
- fixed underline bug in the error context
- the str:format functions now expects strings following this syntax: https://fmt.dev/12.0/syntax/
- more documentation about the compiler implementation
- more documentation about the virtual machine
- closures can be now be compared field per field: `(= closure1 closure2)` will work only if they have the same fields (name) and if the values match
- macros are now defined like `(macro name value)` / `(macro name (args args args) body)` / `($if cond then else)`
- upgraded from C++17 to C++20
- new parser, new syntax for imports: `(import package.sub.file)`
- allow nodes to be empty when dumping the AST to JSON
- macros can be declared inside a `begin` block within a cond macro and used in the scope surrounding the cond macro
- `arkscript --version` and `arkscript --help` now output ArkScript version with the commit hash
- `void Value::toString(std::ostream&, VM&)` now becomes `std::string Value::toString(VM&)`
- removed `Node::operator<<` to replace it with `Node::debugPrint`
- fixed a bug in the compiler where one could pass a non symbol to `let`, `mut` or `set`, resulting in a compiler crash
- fixed a bug in the macro processor where one could pass an unknown symbol to `argcount` and crash the processor
- fixed a bug in the compiler where one could pass something other than a list to `(fun)` as the argument block, resulting in a crash
- fixed a bug in the compiler generating not callable functions
- fixed a bug in the macro processor generating invalid `let` / `mut` / `set` nodes
- fixed a bug in the macro processor allowing out of bounds access with `(macro test (@ [1 2 3] -5))`
- fixed a bug in the vm which wrongfully allowed self concat in place: `(concat! lst lst)`
- fixed a bug in the compiler where one could "use" operators without calling them: `(print nil?)`
- fixed a bug in the compiler allowing the use of operators without any argument: `(+)`
- fixed a bug in the vm during error reporting when a non-function was used as a function
- refactored code inside the bytecode reader to promote code reuse
- fixed a bug in the compiler generating invalid `while` nodes
- fixed a bug when passing the wrong number of arguments to a function inside an async call was crashing the VM because the function couldn't be named
- fixed a bug in the compiler generating invalid `fun` nodes
- fixed a bug when generating `let`, `mut` or `set` nodes inside macros with an invalid node type
- fixed a bug when reading invalid UTF8 codepoints in the parser caused out of bounds reads
- fixed a bug with recursive macro, exhausting the stack space due to recursive evaluation
- futures can be awaited again, they will return nil on all the tries
- checking for reused argument name in macros during parsing
- enhanced comment after node handling in macros
- adding a hard limit on package names length (255 characters, to comply with posix limits)
- disallow passing invalid nodes as arguments to functions and operators
- checking for unevaluated spread inside macros
- checking for invalid symbols when defining a function through a macro
- added a max macro unification depth (256)
- added a max macro evaluation depth (256)
- introduced `internal::listInstructions` with the different instructions, to be used by the compiler and name resolution pass
- checking for forbidden variable/constant name in the name & scope resolution pass, to give errors to the user before compiling some weird code
- repl completion and colours are now generated automatically from the builtins, keywords & operators
- fixed formating of comments inside function declarations
- renamed the macros `symcat` and `argcount` to `$symcat` and `$argcount` for uniformity
- the `Ark::VM` class is now `final`
- the `STORE` instruction has been renamed `SET_VAL`
- the `STORE` instruction is emitted in place of the `LET` and `MUT` instructions, without any mutability checking now
- `io:writeFile` no longer takes a mode and has been split into `io:writeFile` and `io:appendToFile`
- instructions are now positioned like this: `inst byte1 byte2 byte3`
  - byte1 is 0 if the instruction takes a single argument on 16 bits, split on byte2 and byte3
  - if the instruction takes two arguments, they each have 12 bits ; the second one is on byte1 and upper half of byte2, the first on lower half of byte2 and then byte3
- ast-to-json dump now supports macros
- the parser can detect ill-formed macros (that are seen as function macros while being value macros)
- adding a `CALL_BUILTIN <builtin> <arg count>` super instruction
- fixed formatting of comments after the last symbol in an import node
- renamed `str:xyz` builtins to `string:xyz` for uniformity with the standard library
- `string:find` takes an optional third argument, startIndex (where to start the lookup from, default 0
- `list:setAt` can work with negative indexes, and is now bound checked
- re-enabled the AST optimizer, only used for the main `arkscript` executable (not enabled when embedding arkscript, so that one can grab variables from the VM)
- loops have their own scope: variables created inside a loop won't leak outside it
- upgraded `fmtlib` to 11.1.3-13
- allow capture in nested scope (before it was targeting only the current scope)
- `-bcr` option can be given a source file, it will then be compiled before its bytecode is shown
- magic numbers for tables start in bytecode files have been changed from 0x01, 0x02, 0x03 to 0xA1, 0xA2, 0xA3 (symbols, values, code) to make them stand out in hex editors
- magic numbers for value types in bytecode files have been changed from 0x01, 0x02, 0x03 to 0xF1, 0xF2, 0xF3 (number, string, function)
- numbers in the values table in bytecode files are no longer turned to string, but their IEEE754 representation is now encoded on 12 bytes (4 for the exponent, 8 for the mantissa)
- changed how scopes are stored inside the VM to enhance performances. All scope data are now contiguous!
- when possible, accessing variables from the current scope is compiled to a new instruction `LOAD_SYMBOL_BY_INDEX`, to avoid the sometimes expansive lookup by id
  - this works inside normal scopes (introduced by while loops) and functions scopes, but not for closures
- VM stack size is now 4096 instead of 8192
- `Ark::CodeError` now takes a `CodeErrorContext` to store the source (filename, line, column, expression) of an error
- renamed `string:format` to `format`
- `io:removeFiles` is now `io:removeFile` and works on a single file/path
- renamed almost all builtins to prefix them with `builtin__`, to have them proxied in the standard library (to be able to import and scope them properly)
- new super instruction `CALL_BUILTIN_WITHOUT_RETURN_ADDRESS` to optimise the proxied builtins, skipping the return address deletion
- the VM no longer store a reference to the current function being called in the newly created scope
- execution contexts can be reused for async calls if they are not active, to avoid constantly requesting memory and creating (heavy) contexts
  - if there is more than 5 contexts, the 6th one will be destroyed once it completes
- execution contexts are now marked as free to be reused (or deleted) once a value has been computed, without waiting for a call to `await`
- captures are not renamed any more by the NameResolutionPass (which used to fully qualify captured names when possible, which isn't desirable: when you capture `&foo`, you expect to be able to use `.foo` not `.module:foo`)
- when loading a module, its mappings are loaded in the current scope instead of the global scope
- argument order in the CLI changed: the file to run (and its optional script arguments) are now last, to be more consistent with all the other existing tooling (Python, Docker...)
- VM stack size has been upped to 4096 + 256, to have a buffer to be able to catch stack overflows without hindering performances too much
- we can not create a variable in a function, shadowing said function, to prevent weird bugs when trying to do recursion for example

### Removed
- removed unused `NodeType::Closure`
- removing the custom string, replacing it with std::string (the format engine of the custom string had a lot of memory leaks)
- `Utils::digPlaces` and `Utils::decPlaces` got removed as they were no longer needed
- removed deprecated code (`list:removeAt`, `ark` executable now replaced by `arkscript`)
- removed `VM::getUserPointer` and `VM::setUserPointer`
- removed `ARK_PROFILER_COUNT` define
- removed useless `\0` escape in strings
- removed `termcolor` dependency to rely on `fmt` for colouring outputs
- removed `and` and `or` instructions in favour of a better implementation to support short-circuiting
- removed `LET` and `MUT` instructions in favour of a single new `STORE` instruction
- removed `SAVE_ENV` instruction
- removed `Value VM::resolve(const Value* val, Args&&... args)`, which has been deprecated in ArkScript v3.4.0

## [3.5.0] - 2023-02-19
### Added
- added fuzzing tools and corpus for [AFL](https://github.com/AFLplusplus/AFLplusplus)
- added some tests for errors
- added recursion limit reached detection

### Changed
- plugins can be constructed from outside ArkScript lib/modules folder, easing the development process
- plugins loading now works as intended: look alongside the given file/bytecode file, then in the std lib folder
- new way to create modules, easier to use
- calling a non-callable anonymous object do not result in a segfault
- macro processor function registering now handles empty nodes
- added a fix to avoid crashes when capturing unbound variables
- checking if the given operator takes one or more arguments at compile time
- adding bound checking on operator @
- adding bound checking on operator @ when used in macros
- better arity check for macros
- fixed a bug in the macro processor where macros were deleted when they shouldn't
- fixed a bug where macro functions with no argument would crash the macro processor

## [3.4.0] - 2022-09-12
### Added
- added new `async` and `await` builtins
     - they can access the outer scope
- added methods to create and destroy an execution context and a future in the VM
- added new CLI option `--ast` to generate JSON from the generated abstract syntax tree
- added an AST to JSON compiler
- added warnings on unused functions/quotes and statements without any effect

### Changed
- printing a closure will now print its fields instead of `Closure<1432>`
- macros are always evaluated, even when they aren't any user defined macro
- `argcount` works on symbols and anonymous functions

### Deprecated
- deprecating `Value VM::resolve(const Value* val, Args&&... args)`

### Removed
- removed the `std::ostream& operator<<` of the Value, now using the `.toString(stream, vm reference)`
- removed the global VM lock
- removed coz and ARK_PROFILER

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
- the compiler can finally optimise tail calls
- suggesting symbols to the user when the compiler encounters an unbound symbol

### Changed
- the compiler can now remove unused values from the stack
- enhancing the compiler code
- removing bloat in the parser methods argument's lists

## [3.1.3] - 2022-01-29
### Added
- adding an ExecutionContext to host the pointers (instruction, page, stack) and execution related structures (stack, locals, scopes), to ease the transition to a parallelised VM
    - the VM can have multiple independent context running on the same bytecode
- the VM now takes a reference to an `Ark::State` instead of a raw non-owning pointer
- adding `ARK_PROFILER_MIPS` to toggle instruction per second calculation
- adding new way to typecheck in builtins
- new CI build step now running valgrind to check for memory leaks
- new type checker (to be used by builtins)
- better type errors generation (with the list of arguments, if they are matching or not, and more)

### Changed
- splitting Utils.hpp into multiple files for easier maintenance and contextualisation
- reserving a default scope size of 3, which yields good performance results compared to nothing being reserved
- upgrading the builtins error handling to use the `BetterTypeError`
- the VM now displays the debug info (ip, pp, sp) at the end of the backtrace instead of the beginning

### Removed
- `BetterTypeError` has been removed in favour of a type checker using templates and an error generator

### Deprecated
- deprecating `VM(State*)` in favour of `VM(State&)`

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
- updating `replxx` to avoid a bug when compiling with clang

### Removed
- removed `ARK_SCOPE_DICHOTOMY` flag so that scopes don't use dichotomy search but a linear one, since it proved to be faster on small sets of values. This goes toward prioritising small functions, and code being cut in multiple smaller scopes
- removing `download-arkscript.sh` from the repo
- removed `isFraction`, `isInteger`, `isFloat` from Ark/Utils.hpp (worked on strings and used regex)
- removed `mpark::variant` to use standard variant
- `Ark::FeatureFunctionArityCheck` was removed, making arity checks mandatory

## [3.1.1] - 2021-09-19
### Added
- ArkDoc documentation for the builtins
- Now using clang-format to ensure the code is correctly formatted

### Changed
- the macro processor can now handle multiple macro definitions in an if-macro: `!{if true { !{a 1} !{b 2} }}` is finally working

### Deprecated
- `ark` command is now marked as deprecated, in favour of `arkscript`

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
- undefined macros is now possible by using `!{undef macro_name}`
- `str:join` added in the standard library
- `str:split` can now take longer separators
- added `symcat` in macros to concatenate a symbol and a number/string/symbol to create a new one
- added `argcount` in macros to count (at compile time) the number of arguments of a function
- fixed a bug where `(bloc)` and `(print bloc)`, given a `!{bloc value}` macro, didn't give the same result (one was applied, the other was partial)
- new module to manipulate bits: `bitwise`
- enhanced standard library

### Changed
- updating `doxyfile` and some docstrings
- updating the download script
- enhancing examples
- creating a Scope allocates 4 pairs instead of 2, reducing the number of reallocations needed
- `tailOf` renamed to `tail` and `headOf` to `head` ; no need to keep the relics of the past
- `headOf` (now `head`) returns the real head of a container (List or String), the first element (nil if the list is empty, "" if the string is empty)
- the http module was updated to add `http:params:toList` and fix the `http:server:[method]` when passing a function
- fixing the compiler when we encounter get fields in lists
- updating the parser to support usually invalid constructions when they are in macros, to allow things like `!{defun (name args body) (let name (fun args body))}`
- updated the lexer to add UTF8 support and allow unconventional identifiers as long as they aren't keyword nor operators, so things like `->` now works
- fixing the code optimiser to avoid removing unused variables which are defined on function calls
- fixed the traceback generation on errors, it should now display the correct function names
- reorganising the compiler code
- reorganising the parser code to make it more maintainable
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
- `Ark::Logger` was removed in favour of `std::cout/cerr` + `termcolor`

## [3.0.15] - 2020-12-27
### Added
- new submodule, `plasma-umass/coz` (a profiler)
- macros for profiling, enabled only if `ARK_PROFILE` is defined
- cmake flags using -D to turn on/off sys:exec and the coz profiler
- `mpark::variant` is now the default used instead of the default STL variant (faster, better, stronger, and its creator is quite a god)
- new cmake flag, -DARK_SCOPE_DICHOTOMY=On|Off (default Off)
- using internal only references to constants and symbols to reduce the number of useless copies of the value type

### Changed
- updated standard library
- updated modules, adding hash
- updated the error handlers to avoid errors (sigsegv) when handling errors (lexing, parsing, optimisation and compilation error)
- better error message at runtime when a plugin can not be found
- fixes issue #203 (imports are ill-formed when given an absolute path)
- fixes issue #205 (search for the standard library folder in more common places)
- transitioning from C++ streams to printf
- replaced the `thirdparty/` folder with a git submodule in `thirdparties/`
- now checking that a scope doesn't have our symbol before doing a `mut` operation (in dichotomy mode it was automatically handled, but not in linear mode)
- enhancing the cmake defines (`-DARK_XYZ`) and the code using them
- lighter Frame (from 40B to 32B), moved some unrelated logic from the frame to the virtual machine
- `(sys:exec)` now returns the stdout output of the given command

## [3.0.14] - 2020-11-26
### Added
- the parser can handle `(let|mut a b.c)` (bug fix)
- `f[ruv|no-ruv]` CLI switch to control the optimiser (ruv stands for remove unused variables)
- error message when we have too many parenthesis (at parse time)
- error message when using an operator not right after a `(`
- error message when we're capturing an unbound variable
- added `(sys:exit code)` as a builtin
- bytecode integrity checking through a sha256 in the header
- tests for `math:fibo` and `math:divs`
- added the ability to give scripts arguments, through `sys:args`

### Changed
- the parser checks if set is given a dot expression as an identifier (which is an error)
- the parser should take in account captured variables as well, otherwise some variables are optimised while they are captured, resulting in runtime errors
- better unbound variable error message
- (implementation) every constructor with a single argument is now marked as explicit
- REPL does not need to add extra surrounding {}
- the Ark::State (re)compiles a file even if there is a bytecode version available
- the parser is now stricter and gives better error messages when we give too many/not enough arguments to a keyword
- better handling of the code given to the REPL (adds new line)
- renamed the executable from `Ark` to `ark`
- now using GitHub Actions instead of Travis
- the parser can now detect when let/mut/set are fed too many arguments, and generate an error
- the compiler now handles `(set a b.c.d)`
- using a new plugin interface, more C-like

### Removed
- class `Ark::internal::Inst` which was used as a wrapper between `uint8_t` and `Instruction`
- worthless examples were removed
- removing `f[no-]aitap` since it wasn't used any more in the code

## [3.0.13] - 2020-10-12
### Added
- string tests
- list tests
- range tests
- unbound variable checker at compile time (won't break on plugin symbols)

### Changed
- `list:find` returns -1 to stay consistent with `str:find`
- **hot fix** `(mut a 10) (let b 12) (set a b) (set a 11)`, the immutability was transferred from b to a
- converting `list`, `append` and `concat` to instructions
- instructions `LIST` `CONCAT` and `APPEND` added to replace the corresponding builtins

## [3.0.12] - 2020-09-06
### Added
- using a macro to define the default filename (when none is given, e.g. when loading bytecode files or from the REPL)
- `PLUGIN <const id>` instruction to load plugin dynamically and not when the VM boots up
- updated search paths for `(import "lib.ark")`, looking in ./, lib/std/ and lib/
- added a case to display NOT instructions in the bytecode reader
- `T& as<T>()` in usertype
- enhanced error message when calling a non-function object
- eliminating unused global scope variables in the compiler
- adding a new feature enabled by default: `FeatureRemoveUnusedVars` (not enabled for the REPL for obvious reasons)
- added `replxx` as a submodule
- added custom destructor to the user type, called when a scope is destroyed and when we use `(del obj)`
- added a GVL (global virtual machine lock) to be able to use the VM in a multithreaded context
- dockerfile + specific GitHub action to build and push stable and nightly docker images, thanks to @yardenshoham
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
- updated the repl to add auto-completion, coloration and persistence by @PierrePharel
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
- added `thirdparty/madureira/String`, to replace std::string in Ark::internal::Value which was heavy and slower than the new implementation
- minimising the size of the usertype

### Changed
- UserType does not need to be given a manually defined type id but relies on `typeid(T)`
- performance boost of the VM by using pointers to avoid unnecessary copies
- renaming `isNaN` to `NaN?`, `isInf` to `Inf?` for uniformisation (see `empty?`, `nil?`)
- renaming CLI feature options:
    - `-ffunction-arity-check` becomes `-ffac`, same for the `-fno-` version
    - `-fauthorize-invalid-token-after-paren` becomes `-faitap`, some for the `-fno-` version
- improving compiler performances by using const ref when passing nodes around
- renaming the FFI "builtins" because it's not an FFI but a set of functions using the VM API
- the VM should display a backtrace even if an unknown error occurred
- transforming inline code from the vm into not inline code when possible to speed compilation, using macros instead of inline functions
- smaller value class
- smaller vm frames
- forked `madureira/String` and modified it for the needs of the project, added it as a submodule
- removed the VM pointer from the value class to make it lighter, now the VM is sending a pointer of itself to the C procedures
- removed const and type from value, now using an uint8_t to store this information

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
- `(if true () ())` now returns nil (can be generalised to `() -> nil`)
- anonymous functions are now available ; eg: `((fun () (print "a")))`
- added `forEach` in library
- added `-c|--compile` option to the CLI so that we can only compile an ArkScript file instead of compiling and running it, by @DontBelieveMe
- added `min` and `max` in `lib/Math/Arithmetic.ark`, by @FrenchMasterSword
- added `reduce` in `lib/Functional/Reduce.ark`, by @FrenchMasterSword
- added `product` in `lib/List/Product.ark`, by @FrenchMasterSword

### Changed
- a quoted code (deferred evaluation) isn't capturing any more surrounding variables' values, thus increasing greatly performances
- lists are printed like `["string" 1 true]` now, instead of `( string 1 true )`
- updated `zip` so that it can work with lists of different sizes, by @FrenchMasterSword
- better cyclic includes detection
- better VM error message when redefining a variable through `let`

## [3.0.8] - 2019-10-22
### Added
- it's now possible to compare Values using `operator<`
- `reverseList` (added to the FFI) by @rinz13r
- a warning will now pop up during compilation if code appears to be ill-formed (e.g. number/string/list right after a `(`)
- option `-f(allow-invalid-token-after-paren|no-invalid-token-after-paren)` was added (default: OFF)

### Changed
- the internal API used to compare values has been updated to be much shorter
- the REPL can take into account the lib directory, by @rstefanic
- `isNaN` and `isInf` should work on any type but return false if they aren't numbers
- replacing Ark with ArkScript in source code and files (Ark being the shortname for ArkScript, as JS is the shortname for JavaScript)
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
- moved the lib files in subfolders to be more organised
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
- the parser can now recognise expressions like `((f f) x)`
- we can now create `Ark::Value` with floats

### Changed
- the `init()` internal method of the VM shouldn't stop when a bound function isn't used in the code, just ignore it
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
- updating CMakeLists.txt to avoid building unuseful stuff from Google benchmark
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
- using uniformed names of builtins: pascal case (impacted functions are `firstOf`, `headOf` and `tailOf`, as well as `hasField`)
- fixing bug with `writeFile` when sending a mode: the mode was also the content of the file, it no longer is

## [3.0.2] - 2019-08-22
### Added
- cmake options `ARK_BUILD_EXE` and `ARK_BUILD_BENCHMARK` to choose what to build
- when the VM crash, displaying stack trace
- added function `time` to the FFI (time in seconds since epoch)
- adding VM.doFile

### Changed
- updated the VM to be able to call functions defined in ArkScript from C++
- `del sym` set `sym` to `undefined` (internal value only, not the `undefined` of JavaScript) instead of `nil`
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
- adding `del` and `mut` keywords. Now `let` is for settings constants and `mut` for variables. Also, it isn't possible to use `let` to define the same constant twice
- `google/benchmark` library for the benchmarks
- ArkScript version section in bytecode
- timestamp (build date)
- major versions of the compiler and the virtual machine used must match, a compatibility across versions will be kept if they have the same major number
- many opcodes to handle the operators
- persist flag for the VM (if persist is false (default value), each time we call vm.run(), the frames will be reset)
- adding captures through functions arguments: `(fun (&captured std-argument) (...))`
- adding closure fields reading (readonly)

### Changed
- moved everything from the "folder namespaces" to a single `Ark::internal` namespace
- using `#` instead of `'` for the comments, using `'` to quote instead of `` ` ``
- the lexer is now detecting the type of the tokens it's playing with
- using `std::runtime_error`s instead of `exit(1)` when an error occurred
- the VM should throw an error if we try to use `set` on a constant
- we can avoid passing all the arguments to a function, they will just be undefined
- the CLI is now able to determine if it should compile & run, run from the arkscript cache or run the file as a bytecode file

### Removed
- Lexer::check, we should see if the program is correct when building the AST
- removed from the bytecode `NEW_ENV`

## [2.2.0-dev] - 2019-06-01
### Added
- option in the CMakeLists.txt to use `MPIR` or not (defaults to no `MPIR`)
- information about the compilation options used for ArkScript in the CLI
- we can now use `` ` `` to quote

### Changed
- using a vector instead of a map in the `Frame` to speed up things
- using double or `MPIR` depending on the compilation options
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
- the frames stack is handled differently, using shared pointers to avoid unnecessary copies of frame's environments, it improves execution speed by *a lot*
- new CLI
- handling floating pointer numbers and rational numbers

## [2.0.0-dev] - 02-05-2020
### Added
- configure.py script, to download, build and install `mpir` 3.0.0
- builtins functions: input, toNumber, toString
- **breaking change** adding `PLUGIN_TABLE_START` with a value of 3 in the compiler/VM
- adding plugins management

### Changed
- split `lib/Exceptions.ark` into `lib/Exceptions.ark` and `lib/Either.ark`
- renamed FindGMP FindMPIR, and we're now searching for `MPIR` and linking with it
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
- CMakeLists.txt to add `install` rules: installing ArkScript in `bin/` and the ArkScript standard library in `share/.Ark/lib/`
- updated documentation

## [1.2.1-dev] - 2019
### Added
- runtime typechecking
- exceptions (in the C++ ArkScript API)

### Changed
- updated the FFI to add the runtime typechecking
- micro optimisation: using numbers as variable names internally, instead of strings

### Removed
- unnecessary destructors removed to let the compiler auto generate `T(T&&)` (to avoid implicitly using `T(const T&)`)

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
- `dozerg::HugeNumber`, it was too slow

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
### Added
- beginning of the documentation
- compiler (ark code to ark bytecode)
- bytecode reader (human-readable format)
- `dozerg::HugeNumber` to handle big numbers
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
- `ryjen::format` to format strings
