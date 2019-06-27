# Ark

[![Build Status Travis](https://travis-ci.org/SuperFola/Ark.svg?branch=dev)](https://travis-ci.org/SuperFola/Ark)

<img align="right" src="images/Ark.png" width=200px>

Ark is a small programming language made in C++17, inspired by Lisp, using under 10 keywords.

```clojure
{
    (let create-human (fun (name age weight)
        # return value as higher order function to manipulate the data above
        # this will be our "constructor"
        (fun (f)
        {
            # all the setters must be defined in this scope
            (let set-age (fun (new-age) (set age new-age)))

            # and then we can call the function
            (f name age weight set-age)
        })
    ))

    # define function to play with the human more easily
    (let print-human-age (fun (_ age _ _) (print age)))
    (let set-human-age (fun (new-age)
        (fun (_ _ _ set-age) (set-age new-age))
    ))

    (let bob (create-human "Bob" 0 144))
    (let john (create-human "John" 12 15))

    (bob print-human-age)   # prints 0
    (bob (set-human-age 10))
    (bob print-human-age)   # prints 10

    (john print-human-age)  # prints 12
}
```

* Ark is small, the interpreter, the compiler, and the virtual machines fit under 5000 lines
* Ark is a scripting language, easily embedded in your application. The FFI is quite easy to understand, so adding your own functions to the interpreter or the virtual machine is effortless

## Goals

Ark was meant to be a toy language, but it grew into something that I could qualify as big, now aiming video games as a scripting language, and mathematics as it can handle very big numbers without problems. Even if the language is inspired by Lisp, it's trying to convey a better image than "Lost in Stupid Parentheses", by providing `[...]` (expand to `(list ...)`) and `{...}` (expand to `(begin ...)`), to help making a more readable code.

## Features

The language already handles
* first class object
* higher-order function
* church encoding
* closures

## Dependencies

* C++17 compliant compiler, the following have been tested
    * GCC 7.2
    * GCC 8.2
* CMake >= 3.8
* Python 3.6 (for the configure script)
* Visual Studio >= 11 (on Windows)

Libs already included:
* [rj format](https://github.com/ryjen/format), MIT licence
* [CLIPP](https://github.com/muellan/clipp), MIT licence
* [termcolor](https://github.com/ikalnytskyi/termcolor), BSD (3-clause) licence

## Building

```bash
# building Ark
$ cmake -H. -Bbuild
$ cmake --build build
# installing Ark
$ cd build && sudo make install && cd ..
# running
$ Ark --help
SYNOPSIS
        build/Ark -h 
        build/Ark --version 
        build/Ark --dev-info 
        build/Ark <file> [-c] [-o <out>] [-vm] [--count-fcalls] [-bcr] [-d] [-t] 

OPTIONS
        -h, --help                  Display this help message
        --version                   Display Ark lang version and exit
        --dev-info                  Display development informations and exit
        -c, --compile               Compile file
        -o, --output                Set the output filename for the compiler
        -vm                         Start the VM on the given file
        --count-fcalls              Count functions calls and display result at the end of the execution
        -bcr, --bytecode-reader     Launch the bytecode reader
        -d, --debug                 Trigger debug mode
        -t, --time                  Launch a timer

LICENSE
        Mozilla Public License 2.0
```

The project has been tested on
* Linux Mint 18, 64 bits (gcc)
* Lubuntu 18, 32 bits (gcc)
* Windows 10, 64 bits (MSVC)

## [Documentation](doc/main.md)

## Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)

## Copyright and Licence information

Copyright (c) 2019 Alexandre Plateau. All rights reserved.

This Ark distribution contains no GNU GPL code, which means it can be used in proprietary projects.