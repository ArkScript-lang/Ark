# ArkScript
### Current version: 3.0.9

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/fd5900d08a97487486c43079c06e19ce)](https://app.codacy.com/app/folaefolc/Ark?utm_source=github.com&utm_medium=referral&utm_content=SuperFola/Ark&utm_campaign=Badge_Grade_Settings)
[![Build Status](https://travis-ci.org/SuperFola/Ark.svg?branch=rework)](https://travis-ci.org/SuperFola/Ark)

<img align="right" src="images/ArkTransparent-by-mazz.png" width=200px>

* Documentation: [Wiki](https://github.com/SuperFola/Ark/wiki)
* Discord server: [invite link](https://discord.gg/YT5yDwn), to discuss the specification of the language and receive help

**Nota bene**: the project is referred as "Ark" and as "ArkScript". The official public name is "ArkScript" since "Ark" is already being used by [another language](https://github.com/ark-lang/ark)

## Key features

ArkScript is
* small: the compiler, and the virtual machines fit under 5000 lines, but also small in term of keywords (it has only 10)!
* a scripting language: it's very easy to embed it in your application. The FFI is quite easy to understand, so adding your own functions to the virtual machine is effortless
* portable: it produces a bytecode which is run by its virtual machine, like Java but without the `OutOfMemoryException`
* a functional language: every parameters are passed by value, everything is immutable unless you use `mut` to define a mutable variable
* powerful: it can handle object oriented programming in a very elegant way with its closures and explicit captures (see examples/church-encoding)
* promoting functionalities before performances: expressiveness often brings more productivity, but performances aren't bad at all
* easy to compile: it takes less than 200ms to compile and check a complex code with a lot of branches and sub-branches of 200 lines.
* a Lisp-like, but with less parentheses: `[...]` is expanded to `(list ...)` and `{}` to `(begin ...)`. More shorthands will come in the future.

Also:
* it has a REPL
* it has a growing standard library, composed of ArkScript (under `lib/`) and C++ (under `modules/`)
* it has a lot of unit tests, which are ran before every release to ensure everything work as intended

## Examples

### Fibonacci suite

```clojure
{
    (let fibo (fun (n)
        (if (< n 2)
            n
            (+ (fibo (- n 1)) (fibo (- n 2))))))

    (print (fibo 28))  # display 317811
}
```

## More or less game

```clojure
{
    # more or less game
    (print "More or less game!")

    (import "librandom.so")
    (import "Arithmetic.ark")

    (let number (mod (abs (random)) 10000))
    (mut value 0)
    (mut essais 0)

    (mut continue true)

    (while continue {
        (set value (toNumber (input "Input a numeric value: ")))

        (if (< value number)
            # then
            (print "More!")
            # else
            (if (= value number)
                # then
                { (print "Bingo!") (set continue false) }
                # else
                (print "Less!")))

        (set essais (+ 1 essais))})

    (print "You won in " essais " tries")
}
```

More examples are available in the folder `examples/`.

## Contributing

* First, [fork](https://github.com/SuperFola/Ark/fork) the repository
* Then, clone your fork: `git clone git@github.com:username/Ark.git`
* Create a branch for your feature: `git checkout -b feat-my-awesome-idea`
* When you're done, push it to your fork and submit a pull request!

Don't know what to work on? No worries, we have a [list of things to do](https://github.com/SuperFola/Ark/projects) :wink:

### Our beloved contributors

Who worked on
* the standard library
    * [@SuperFola](https://github.com/SuperFola)
    * [@Natendrtfm](https://github.com/Natendrtfm)
    * [@rinz13r](https://github.com/rinz13r)
    * [@FrenchMasterSword](https://github.com/FrenchMasterSword)
    * [@rstefanic](https://github.com/rstefanic)
* the FFI
    * [@SuperFola](https://github.com/SuperFola)
    * [@rinz13r](https://github.com/rinz13r)
* the REPL
    * [@rstefanic](https://github.com/rstefanic)
* the CLI
    * [@SuperFola](https://github.com/SuperFola)
    * [@DontBelieveMe](https://github.com/DontBelieveMe)
* the documentation
    * [@SuperFola](https://github.com/SuperFola)
    * [@OfficePop](https://github.com/OfficePop)
* the language specification
    * [@SuperFola](https://github.com/SuperFola)
    * [@FrenchMasterSword](https://github.com/FrenchMasterSword)

### Contributing to the ArkScript standard library

See [Coding guidelines](https://github.com/SuperFola/Ark/wiki/Coding-guidelines#coding-in-ark) if you want to write ArkScript for the library (see folder `lib/`).

For performance reasons, some functions might be written in C++, in `include/Ark/VM/FFI.hpp` and `src/VM/FFI/`.

### Code structure

![ArkScript code structure](images/arkscript-code-structure.png)

## Dependencies

* C++17
* CMake >= 3.12
* Visual Studio >= 11 (on Windows)

Libs already included:
* [rj format](https://github.com/ryjen/format), MIT licence
* [CLIPP](https://github.com/muellan/clipp), MIT licence
* [termcolor](https://github.com/ikalnytskyi/termcolor), BSD (3-clause) licence
* [google/benchmark](https://github.com/google/benchmark), Apache 2.0 licence

## Building

```bash
# first, clone it
~$ git clone --depth=50 --branch=dev https://github.com/SuperFola/Ark.git
~/Ark$ cd Ark
~/Ark$ git submodule update --init --recursive
# building Ark
~/Ark$ cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=1
~/Ark$ cmake --build build
# installing Ark
# works on Linux and on Windows (might need administrative privileges)
~/Ark$ cmake --install build --config Release
# running
~/Ark$ Ark --help
DESCRIPTION
        ArkScript programming language

SYNOPSIS
        build/Ark -h, --help
        build/Ark --version
        build/Ark --dev-info
        build/Ark ((<file> [-c]) | -r) [-(d|bcr)] [-L <lib_dir>] [-f(function-arity-check|no-function-arity-check)] [-f(allow-invalid-token-after-paren|no-invalid-token-after-paren)]

OPTIONS
        -h, --help                  Display this message
        --version                   Display ArkScript version and exit
        --dev-info                  Display development information and exit
        -c, --compile               Compile the given program to bytecode, but do not run
        -r, --repl                  Run the ArkScript REPL
        -d, --debug...              Increase debug level (default: 0)
        -bcr, --bytecode-reader     Launch the bytecode reader
        -L, --lib                   Set the location of the ArkScript standard library

        -f(function-arity-check|no-function-arity-check)
                                    Toggle function arity checks (default: ON)

        -f(allow-invalid-token-after-paren|no-invalid-token-after-paren)
                                    Authorize invalid token after `(' (default: OFF). When ON, only display a warning

LICENSE
        Mozilla Public License 2.0
```

## Performances

The project was compiled on Linux Mint 18 x64, with g++ 8 and `-DNDEBUG -O3 -s`.

The test here is the Ackermann-Peter function with m=3 and n=6:

```
2019-10-08 19:46:02
Running benchmark/vm
Run on (4 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32K (x2)
  L1 Instruction 32K (x2)
  L2 Unified 256K (x2)
  L3 Unified 3072K (x1)
Load Average: 0.94, 1.25, 0.86
-------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations
-------------------------------------------------------------------
Ackermann_3_6_ark_mean          133 ms          133 ms           25
Ackermann_3_6_ark_median        133 ms          133 ms           25
Ackermann_3_6_ark_stddev      0.694 ms        0.694 ms           25

Ackermann_3_6_cpp_mean        0.370 ms        0.370 ms           25
Ackermann_3_6_cpp_median      0.368 ms        0.368 ms           25
Ackermann_3_6_cpp_stddev      0.004 ms        0.004 ms           25
```

Comparison with Java using OpenJDK 11.0.3 x64 (source code [here](benchmarks/Ackermann.java)):
```
Mean time: 651.28us
Median time: 622us
Stddev: 119.49792299450229us
```

Comparison with Python 3.6.7 (source code [here](benchmarks/ackermann.py)):
```
Mean time: 36.2155839279294ms
Median time: 35.600485280156136ms
Stddev: 0.8867313499129518ms
```

Comparison with Lua 5.1.5 (source code [here](benchmarks/ackermann.lua)):
```
Mean time: 13.244432 ms
Median time: 13.214 ms
Stddev: 0.30360220508642ms
```

## Games

You can find a snake created in ArkScript in the folder examples/games/snake (run it from there, otherwise it won't find the font and the sprites ; you won't need to install the SFML).

![ArkSnake](images/ArkSnake.png)

Controls are the arrows (left, right, up and down), the game closes itself when you successfully collect the 3 apples.

## Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)

## Copyright and Licence information

Copyright (c) 2019 Alexandre Plateau. All rights reserved.

This ArkScript distribution contains no GNU GPL code, which means it can be used in proprietary projects.
