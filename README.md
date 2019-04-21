# Ark

<img align="right" src="images/Ark.png" width=200px>

Ark is a small programming language made in C++17, inspired by Lisp, using under 10 keywords.

```clojure
(begin
    (let create-human [fun (name age weight) {begin
        (let _name name)
        (let _age age)
        (let _weight weight)

        (let set-age [fun (new-age) (set _age new-age)])

        ' return value as higher-order function to manipulate the data above
        (fun (f) [begin
            (f _name _age _weight set-age)
        ])
    }])

    (let print-human-age [fun (d0 _age d1 d2) (print _age)])
    (let set-human-age [fun (new-age)
        (fun (d0 d1 d2 set-age) (set-age new-age))
    ])

    (let bob [create-human "Bob" 0 144])

    (bob print-human-age)  ' prints 0

    (bob (set-human-age 10))
    (bob print-human-age)  ' prints 10
)
```

* Ark is small, the interpreter, the compiler, and the virtual machines fit under 5000 lines
* Ark is a scripting language, easily embedded in your application. The FFI is quite easy to understand, so adding your own functions to the interpreter or the virtual machine is effortless

## Goals

Ark was meant to be a toy language, but it grew into something that I could qualify as big, now aiming video games as a scripting language, and mathematics as it can handle very big numbers without problems. Even if the language is inspired by Lisp, it's trying to convey a better image than "Lost in Stupid Parentheses", by providing `()[]{}`, to help making a more readable code.

## Features

The language already handles
* first class object
* higher-order function
* church encoding
* closures

## Dependencies (already included)

* C++17 compliant compiler
* CMake >= 3.8
* rj format (https://github.com/ryjen/format)
* CLIPP
* termcolor
* huge_number (by [daidodo](https://github.com/daidodo/huge-long-number))

## Tests

Running on Linux Mint 18, 64 bits.

```bash
# building
$ cmake -H. -Bbuild
$ cmake --build build
# running
$ ./build/Ark --help
SYNOPSIS
        build/Ark -h 
        build/Ark --version 
        build/Ark bcr <file> 
        build/Ark repl [-d] [-t] 
        build/Ark (interpreter|compile|vm) <file> [-d] [-t] 

OPTIONS
        -h, --help                  Display this help message
        --version                   Display Ark lang version and exit
        bcr                         Run the bytecode reader on the given file
        repl                        Start a Read-Eval-Print-Loop
        interpreter                 Start the interpreter with the given Ark source file
        compile                     Start the compiler to generate a bytecode file from the given Ark source file
        vm                          Start the virtual machine with the given bytecode file
        -d, --debug                 Enable debug mode
        -t, --time                  The task is timed

LICENSE
        Mozilla Public License 2.0
```

## Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)