@page guidelines_coding Coding guidelines

## Definitions

_guideline_: general rule applying to ArkScript code, which must be followed when contributing to the standard library and examples of the main repository
_standard library (aka lib)_: the files and functions in the `lib` folder
_builtins_: functions and constants defined through C++ code, available without having to import anything
_module_: C++ plugin for the ArkScript virtual machine, allowing use of C++ code (eg: the SFML)

## Precisions

Indentation matters to us, programmers (but not to the compiler): 4 spaces or a single tab, but it should stay consistent accross a project/file.

The general rule of thumb is that a closing parenthesis should never be to the left of its matching opening parenthesis. All new lines should be a couple of spaces to the right of the opening parenthesis of the list they're in.

## Naming convention

Functions and constants (the ones in the lib and in the builtins) are named following the `first:second` convention, described in @ref guidelines_naming "the naming guidelines".

## Block definitions

* Indent the content of every `begin` block
* When using begin blocks in if (then, else), they should appear clearly as a block, each opening brace on its own line
* When using begin blocks in functions (body), they can appear as an indenpendant block (initial `(begin` or `{` on its own line) or not (initial `(begin` or `{` on the same line as the `fun` keyword)
* closing braces are stacked together, and never preceded by a newline
    * if the last instruction wasn't a function call but a variable, a space should be put between those two

Complete example:

~~~~{.clojure}
(let foo (fun (a) {
    (if (= a 2)
        {
            (print "a = 2")
            (egg (* 2 a))}
        {
            (print "a != 2")
            (egg 0)})}))

(let bar (fun (b c) {
    (let boop (+ b c))
    boop })
~~~~

## Standard library functions and constants' documentation

Each function and constant defined in the standard library should be documented, using [ArkDoc](https://github.com/ArkScript-lang/ArkDoc) format.
