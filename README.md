# Ark

<img align="right" src="images/Ark.png">

This is a small programming language, made just for fun, in C++17.

# Dependencies (already included)

* C++17 compliant compiler
* CMake >= 3.8
* rj format (https://github.com/ryjen/format)
* CLIPP
* termcolor
* huge_number (by [daidodo](https://github.com/daidodo/huge-long-number))

# Tests

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

# Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)