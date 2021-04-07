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

* Indent with **4 spaces**
* Every brace (`{`, `}`) must be on its own line
* Conditions with a single statement (`if (condition) do_this();`) do not need to be enclosed in braces
* Put a space between `for`, `while`, `if` and `(...)`, around each `=` sign (wherever it is, even in for-loops)
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

## ArkScript coding guidelines

Check out the [ArkScript RFC 001](https://github.com/ArkScript-lang/rfc/blob/master/001-coding-guidelines.md). We are quite flexible about it, those are just general guidelines to produce a readable ArkScript code.