# ArkScript  ![Latest version](https://img.shields.io/github/v/release/arkscript-lang/ark?include_prereleases&style=for-the-badge)

![Code size](https://img.shields.io/github/languages/code-size/arkscript-lang/ark?style=for-the-badge&logo=github)
![Downloads](https://img.shields.io/github/downloads/arkscript-lang/ark/total?color=%2324cc24&style=for-the-badge&logo=github)
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/ArkScript-lang/Ark/CMake?logo=cmake&style=for-the-badge)

<img align="right" src="images/ArkTransparent-by-mazz.png" width=200px>

* [Documentation](https://arkscript-lang.github.io/documentation.html)
* Discord server: [invite link](https://discord.gg/YT5yDwn), to discuss the specification of the language and receive help
* [Modules](https://github.com/ArkScript-lang/modules)

**Nota bene**: the project is referred as "Ark" and as "ArkScript". The official public name is "ArkScript" since "Ark" is already being used by [another language](https://github.com/ark-lang/ark)

## Key features

ArkScript is
* small: the compiler and the virtual machines fit under 8000 lines, but also small in terms of keywords (it has only 10)!
* a scripting language: it's very easy to embed it in your application. The builtin construction is quite easy to understand, so adding your own functions to the virtual machine is effortless
* portable: it produces a bytecode which is run by its virtual machine, the same way Java does it (with a smaller memory footprint)
* a functional language: every parameter is passed by value, everything is immutable unless you use `mut` to specify your need of mutability
* powerful: it can handle object-oriented programming in a very elegant way with its closures and explicit captures (see `examples/closures.ark`)
* promoting functionalities before performances: expressiveness often brings more productivity, even though performances aren't left behind
* easy to compile: it takes less than 200ms to compile and check a complex code with a lot of branches and sub-branches of 200 lines.
* a Lisp-like, but with fewer parentheses: `[...]` is expanded to `(list ...)` and `{}` to `(begin ...)`. More shorthands will come in the future.
* extensible: it is very easy to create a C++ module to use it in the language, adding functionalities

Also it has:
* macros: if/else, values, and functions
* a REPL with autocompletion and coloration
* a growing standard library, composed of ArkScript code (under `lib/std/`) and C++ (under `lib/ext/`)
* a lot of unit tests (but never enough), which are ran before every release to ensure everything work as intended
* docker images to try the language without compiling it:
    * [stable](https://hub.docker.com/r/arkscript/stable), images built after each release
    * [nightly](https://hub.docker.com/r/arkscript/nightly), image built after each commit

## Examples

### Fibonacci suite

```clojure
(let fibo (fun (n)
    (if (< n 2)
        n
        (+ (fibo (- n 1)) (fibo (- n 2))))))

(print (fibo 28))  # display 317811
```

## More or less game

```clojure
(import "random.arkm")
(import "Math.ark")

(let number (mod (abs (random)) 10000))

(mut value 0)
(mut tries 0)
(mut continue true)

(while continue {
    (set value (toNumber (input "Input a numeric value: ")))

    (if (< value number)
        # then
        (print "More!")
        # else
        (if (= value number)
            # then
            {
                (print "Bingo!")
                (set continue false)
            }
            # else
            (print "Less!")))

    (set tries (+ 1 tries))
})

(print "You won in " tries " tries")
```

More examples are available under the folder `examples/`.

## Installation

### Through the latest release

*Linux only*

**Important**: this method will add the folder where ArkScript will be downloaded to your PATH. The executable being named `ark` you can have **conflicts** with another existing program named `ark` as well, [a KDE archiving tool](https://linux.die.net/man/1/ark).

Save the following code as `install_arkscript.sh` and run `bash install_arkscript.sh`.

```bash
#!/usr/bin/env bash
set -euo pipefail
mkdir -p "${HOME}/.ark"
cd "${HOME}/.ark"
install_dir=`pwd`

current=`curl -s https://github.com/ArkScript-lang/Ark/releases/latest | egrep -o "tag/(?[^\"]+)" | cut -c 5- -`
url="https://github.com/ArkScript-lang/Ark/releases/download/$current/linux64.zip"
wget --quiet $url

if [ -f linux64.zip ]; then
    unzip -o linux64.zip
    rm linux64.zip
fi

# export arkscript path to your PATH variable to call it from everywhere
# export also ARKSCRIPT_PATH for arkscript to find its standard library
shellrc=""
path_update="export PATH=\"\$PATH:${install_dir}\""
SHELL=$(echo $SHELL | rev | cut -d'/' -f 1 | rev)
case $SHELL in
   "fish")
      shellrc="$HOME/.config/fish/config.fish"
      path_update="set PATH $install_dir \$PATH"
      ;;
   "zsh")
      shellrc="$HOME/.zshrc"
      ;;
   "bash")
      shellrc="$HOME/.bashrc"
      ;;
   *)
      echo "Unsupported shell: $SHELL. Please open an issue at <https://github.com/ArkScript-lang/Ark/issues/new> to request it."
      exit 1
      ;;
 esac

cat >> ${shellrc}<< EOF
$path_update
export ARKSCRIPT_PATH="${install_dir}"
EOF
echo "Don't forget to reload your shell configuration (\`source ${shellrc}\`) to refresh your path."
```

### Through docker

```bash
$ docker pull arkscript/stable:latest
```

## Contributing

* First, [fork](https://github.com/ArkScript-lang/Ark/fork) the repository
* Then, clone your fork: `git clone git@github.com:username/Ark.git`
* Create a branch for your feature: `git checkout -b feat-my-awesome-idea`
* When you're done, push it to your fork and submit a pull request!

Don't know what to work on? No worries, we have a [list of things to do](https://github.com/ArkScript-lang/Ark/issues) :wink:

### Related projects

We have other projects tightly related to ArkScript, which aren't necessarily C++ oriented:
* the [Request For Comments](https://github.com/ArkScript-lang/rfc), where we discuss new features for the language
* the [standard library](https://github.com/ArkScript-lang/std), written in ArkScript itself
* the [standard library modules](https://github.com/ArkScript-lang/modules), extending the capacities of the language, written in C++
* [ArkDoc](https://github.com/ArkScript-lang/ArkDoc), a documentation generator à la doxygen, for ArkScript, written in Ruby
* our [website](https://github.com/ArkScript-lang/arkscript-lang.github.io) written in HTML, CSS and JavaScript

### Our beloved contributors

Who worked on
* the standard library
    * [@SuperFola](https://github.com/SuperFola)
    * [@Natendrtfm](https://github.com/Natendrtfm)
    * [@rinz13r](https://github.com/rinz13r)
    * [@FrenchMasterSword](https://github.com/FrenchMasterSword)
    * [@rstefanic](https://github.com/rstefanic)
    * [@PierrePharel](https://github.com/PierrePharel)
    * [@Wafelack](https://github.com/Wafelack)
* the builtins
    * [@SuperFola](https://github.com/SuperFola)
    * [@rinz13r](https://github.com/rinz13r)
    * [@PierrePharel](https://github.com/PierrePharel)
* the REPL
    * [@rstefanic](https://github.com/rstefanic)
    * [@PierrePharel](https://github.com/PierrePharel)
* the CLI
    * [@SuperFola](https://github.com/SuperFola)
    * [@DontBelieveMe](https://github.com/DontBelieveMe)
    * [@PierrePharel](https://github.com/PierrePharel)
* the documentation
    * [@SuperFola](https://github.com/SuperFola)
    * [@OfficePop](https://github.com/OfficePop)
    * [@PierrePharel](https://github.com/PierrePharel)
* the language specification
    * [@SuperFola](https://github.com/SuperFola)
    * [@FrenchMasterSword](https://github.com/FrenchMasterSword)
    * [@PierrePharel](https://github.com/PierrePharel)
* the logo
    * [@mazzdevs](https://github.com/mazzlabs)
* the docker integration
    * [@yardenshoham](https://github.com/yardenshoham)
    * [@SuperFola](https://github.com/SuperFola)

### Contributing to the ArkScript standard library

See [Coding guidelines](https://github.com/ArkScript-lang/Ark/wiki/Coding-guidelines#coding-in-ark) if you want to write ArkScript for the library (see folder `lib/std/`).

For performance reasons, some functions might be written in C++, in `include/Ark/Builtins/Builtins.hpp` and `src/Builtins/`.

### Code structure

![ArkScript code structure](images/diagram.svg)

## Building

### Dependencies

* C++17
* CMake >= 3.12
* Visual Studio >= 11 (on Windows)
* On macOS versions prior to 10.15, `libc++` lacks `filesystem` in the standard library.
    * Install a newer compiler using [Homebrew](https://docs.brew.sh/): `brew install gcc && brew link gcc`
    * Pass compiler path to `cmake` in the build step: `-DCMAKE_CXX_COMPILER=/usr/local/bin/g++-9`

All the external libraries we use are already included in [thirdparties](https://github.com/ArkScript-lang/thirdparties).

### Through CMake

Different CMake switches are available to customize the build:
* `-DARK_BUILD_EXE` to generate an executable, defaults to Off, building a shared library only
* `-DARK_ENABLE_SYSTEM` to enable `sys:exec` (execute shell commands without restrictions), defaults to On
* `-DARK_PROFILER` to enable the [coz](https://github.com/plasma-umass/coz) profiler, defaults to Off
* `-DARK_PROFILER_COUNT` to count every creation/copy/move of the internal value type, defaults to Off
* `-DARK_SCOPE_DICHOTOMY` to activate the dichotomic mode of the scope, defaults to Off

```bash
# first, clone it
~$ git clone --depth=50 --branch=dev https://github.com/ArkScript-lang/Ark.git
~/Ark$ cd Ark
~/Ark$ git submodule update --init --recursive
# building Ark
~/Ark$ cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DARK_BUILD_EXE=On
~/Ark$ cmake --build build --config Release
# installing Ark (might need administrative privileges)
~/Ark$ cmake --install build --config Release
# running
~/Ark$ ark --help
DESCRIPTION
        ArkScript programming language

SYNOPSIS
        ark -h 
        ark -v 
        ark --dev-info 
        ark -e <expression> 
        ark -c <file> [-d] 
        ark -bcr <file> [-(a|st|vt|cs)] [-p <page>] [-s <start> <end>] 
        ark <file> [-d] [-L <lib_dir>] [-f(fac|no-fac)] [-f(ruv|no-ruv)] 

OPTIONS
        -h, --help                  Display this message
        -v, --version               Display ArkScript version and exit
        --dev-info                  Display development information and exit
        -e, --eval                  Evaluate ArkScript expression
        -c, --compile               Compile the given program to bytecode, but do not run
        -d, --debug...              Increase debug level (default: 0)
        -bcr, --bytecode-reader     Launch the bytecode reader
        -a, --all                   Display all the bytecode segments (default)
        -st, --symbols              Display only the symbols table
        -vt, --values               Display only the values table
        -cs, --code                 Display only the code segments
        -p, --page                  Set the bytecode reader code segment to display      
        -s, --slice                 Select a slice of instructions in the bytecode       
        -L, --lib                   Set the location of the ArkScript standard library   
        -f(fac|no-fac)              Toggle function arity checks (default: ON)
        -f(ruv|no-ruv)              Remove unused variables (default: ON)

LICENSE
        Mozilla Public License 2.0
```

## Performances

See https://github.com/ArkScript-lang/benchmarks

## Games

You can find a snake created in ArkScript in the folder examples/games/snake (run it from there, otherwise it won't find the font and the sprites ; you won't need to install the SFML).

![ArkSnake](images/ArkSnake.png)

Controls are the arrows (left, right, up and down), the game closes itself when you successfully collect the 3 apples.

## The donators

Huge thanks to those people for their donations to support the project:
* [TheCountVEVO](https://github.com/TheCountVEVO)
* [llexto](https://github.com/llexto)

## Credits

This project was inspired by [gameprogramingpatterns](http://gameprogrammingpatterns.com/bytecode.html) and [ofan lisp.cpp](https://gist.github.com/ofan/721464)

## Copyright and Licence information

Copyright (c) 2019-2021 Alexandre Plateau. All rights reserved.

This ArkScript distribution contains no GNU GPL code, which means it can be used in proprietary projects.
