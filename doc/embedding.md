# Embedding Ark

## Embedding the interpreter

```cpp
#include <iostream>
#include <string>

#include <Ark/Ark.hpp>

const char* g_code =
    "(begin\n"
    "    (let a 42)\n"
    "    (let b \"hello world\")\n"
    "    (let life [fun (x) {* x 2}])\n"
    ")\n"
    "";

int main()
{
    Ark::Lang::Program program;
    program.feed(g_code);
    program.execute();
    
    auto a = program.get<Ark::BigNum>("a");
    auto b = program.get<std::string>("b");
    auto life = program.get<Ark::Function>("life");
    
    std::cout << a << " " << b << " " << life(a) << std::endl;
    
    return 0;
}
```

### Creating a program

An `Ark::Lang::Program` is needed to run code. You can either feed it with a string, or with code from a file, like so:

```cpp
Ark::Lang::Program program;
program.feed(Ark::Utils::readFile("filename"));
```

### Running a program

Using `program.execute()` will the run the Ark code provided to the program, needed if you want to get values from it later on.

If you run it a second time, the environment won't be reset.

### Getting values from a program

```cpp
auto my_integer = program.get<Ark::BigNum>("my_ark_integer");
```

The types supported by `.get<>` are: Ark::BigNum, std::string and Function.

A Function can take Ark::BigNum and std::string (multiple arguments are supported).

### Registering a C++ function into an Ark program

```cpp
Node foo(const Nodes& n)
{
    std::cout << n.size() << std::endl;
    for (Node::Iterator it=n.begin(); it < n.end(); ++it)
    {
        std::cout << (*it) << std::endl;
    }
    return Ark::Lang::nil;
}

void main()
{
    Ark::Lang::Program program;
    program.feed(
        "(foo (list 5 12 \"hello world\"))"
    );
    program.loadFunction("foo", &foo);

    program.execute();
    /*
    Will print:
        3
        5
        12
        hello world
    */
}
```

## Embedding the virtual machine

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <Ark/Ark.hpp>

const char* g_code_vm =
    "(begin\n"
    "    (let a (do-stuff 42))\n"
    "    (if a (print \"ok\") (print \"nope\"))\n"
    ")\n"
    "";

Ark::VM::Value dostuff(const std::vector<Ark::VM::Value>& values)
{
    if (values.size() == 1 && values[0].isNumber() && values[0].number() == 42)
        return Ark::VM::Value(Ark::VM::NFT::True);
    return Ark::VM::Value(Ark::VM::NFT::False);
}

int main()
{
    Ark::VM::VM vm;
    Ark::Compiler::Compiler compiler;
    compiler.feed(g_code_vm);
    compiler.compile();
    vm.feed(compiler.bytecode());

    vm.loadFunction("do-stuff", &dostuff);

    vm.run();
    
    return 0;
}
```

### Creating a virtual machine and a compiler

An `Ark::Compiler::Compiler` is needed to compile Ark code to Ark bytecode for the VM. You can either feed it with a string, or with code from a file, like so:

```cpp
Ark::Compiler::Compiler compiler;
compiler.feed(Ark::Utils::readFile("filename"));
compile.compile();  // needed to toggle bytecode generation
```

To create a virtual machine:

```cpp
Ark::VM::VM vm;
vm.feed(compiler.bytecode());
// or
vm.feed(filename);  // the file must contain Ark bytecode
```

### Running a program on the virtual machine

Using the virtual machine, already fed with a bytecode, run `run`:

```cpp
vm.run();
```

### Registering a C++ function into an Ark VM

```cpp
using namespace Ark::VM;

Value dostuff(const std::vectorValue>& values)
{
    if (values.size() == 1 && values[0].isNumber() && values[0].number() == 42)
        return Value(NFT::True);
    return Value(NFT::False);
}

int main()
{
    VM vm;
    Ark::Compiler::Compiler compiler;
    compiler.feed(g_code_vm);
    compiler.compile();
    vm.feed(compiler.bytecode());

    vm.loadFunction("do-stuff", &dostuff);

    vm.run();
    /*
        Will print:
            ok
    */
    
    return 0;
}
```