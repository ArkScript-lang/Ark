# Ark

[![Build Status](https://travis-ci.org/SuperFola/Ark.svg?branch=rework)](https://travis-ci.org/SuperFola/Ark)

<img align="right" src="images/Ark.png" width=200px>

Documentation: [doc/main.md](doc/main.md)

## Key features

* Ark is small: the compiler, and the virtual machines fit under 5000 lines, but also small in term of keywords (it has only 10)!
* Ark is a scripting language: it's very easy to embed it in your application. The FFI is quite easy to understand, so adding your own functions to the interpreter or the virtual machine is effortless
* Ark can run everywhere: it produces a bytecode which is run by its virtual machine, like Java but without the `OutOfMemoryException`
* Ark is a functional language: every parameters are by passed by value, everything is immutable unless you use `mut` to define a mutable variable
* Ark can handle object oriented programming using closures in a very elegant way (see example below)
* Ark handles first class objects, thus it has higher-order functions
* Ark is fast: the compiler takes less than 200µs to compile and check a complex code with a lot of branches and sub-branches of 200 lines.
* Ark is a Lisp-like, but with less parentheses: `[...]` is expanded to `(list ...)` and `{}` to `(begin ...)`. More shorthands will come in the future.

## Example

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

## Contributing

* First, [fork](https://github.com/SuperFola/Ark/fork) the repository
* Then, clone your fork: `git clone git@github.com:username/Ark.git`
* Create a branch for your feature: `git checkout -b feat-my-awesome-idea`
* When you're done, push it to your fork and submit a pull request!

Don't know what to work on? No worries, we have a [list of things to do](https://github.com/SuperFola/Ark/projects) :wink:

## Notes

* The execution of some programs is slower than expected, due to some operators being used like functions, but we are working on it, a brand new FFI is coming soon!
* The language is a bit limited in term of what it can do, because we are working on performance improvement and some brand new features, but adding builtin functions and operators is easy, so we will work on that as soon as possible

## Dependencies

* C++17
* CMake >= 3.8
* Visual Studio >= 11 (on Windows)

Libs already included:
* [rj format](https://github.com/ryjen/format), MIT licence
* [CLIPP](https://github.com/muellan/clipp), MIT licence
* [termcolor](https://github.com/ikalnytskyi/termcolor), BSD (3-clause) licence
* [google/benchmark](https://github.com/google/benchmark), Apache 2.0 licence

## Building

```bash
# first, clone it
~$ git clone --depth=50 --branch=rework https://github.com/SuperFola/Ark.git
~/Ark$ cd Ark
~/Ark$ git submodule update --init --recursive
# building Ark
~/Ark$ cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
~/Ark$ cmake --build build
# installing Ark
~/Ark$ cd build && sudo make install && cd ..
# running
~/Ark$ Ark --help
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

## Performances

The project was compiled on Linux Mint 18 x64, with g++ 8 and `-DNDEBUG -O4 -s`.

The test here is the Ackermann-Peter function with m=3 and n=6:

```
2019-06-30 17:21:33
Running benchmark/vm
Run on (4 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32K (x2)
  L1 Instruction 32K (x2)
  L2 Unified 256K (x2)
  L3 Unified 3072K (x1)
Load Average: 0.84, 0.64, 0.62
-------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations
-------------------------------------------------------------------
Ackermann_3_6_ark_mean          160 ms          160 ms           25
Ackermann_3_6_ark_median        159 ms          159 ms           25
Ackermann_3_6_ark_stddev       8.75 ms         8.75 ms           25

Ackermann_3_6_cpp_mean        0.339 ms        0.339 ms           25
Ackermann_3_6_cpp_median      0.337 ms        0.337 ms           25
Ackermann_3_6_cpp_stddev      0.007 ms        0.007 ms           25
```

Comparison with Java using OpenJDK 11.0.3 x64 (source code [here](benchmarks/Ackermann.java)):
```
Mean time: 651.28us
Mean time: 622us
Stddev: 119.49792299450229us
```

## Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)

## Copyright and Licence information

Copyright (c) 2019 Alexandre Plateau. All rights reserved.

This Ark distribution contains no GNU GPL code, which means it can be used in proprietary projects.
