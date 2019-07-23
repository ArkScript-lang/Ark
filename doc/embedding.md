# Embedding the Ark Virtual Machine

## Example by code

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

using namespace Ark::internal;

Value dostuff(const std::vector<Value>& values)
{
    if (values.size() == 1 && values[0].isNumber() && values[0].number() == 42)
        return Value(NFT::True);
    return Value(NFT::False);
}

int main()
{
    Ark::VM vm;
    Ark::Compiler compiler;
    compiler.feed(g_code_vm);
    compiler.compile();
    vm.feed(compiler.bytecode());

    vm.loadFunction("do-stuff", &dostuff);

    vm.run();
    
    return 0;
}
```

### Creating a virtual machine and a compiler

An `Ark::Compiler` is needed to compile Ark code to Ark bytecode for the VM. You can either feed it with a string, or with code from a file, like so:

```cpp
Ark::Compiler compiler;
compiler.feed(Ark::Utils::readFile("filename"));
compile.compile();  // needed to toggle bytecode generation
```

To create a virtual machine:

```cpp
// debug mode on
Ark::VM_debug vm;
// debug mode off
Ark::VM vm;
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
using namespace Ark;

Value dostuff(const std::vector<Value>& values)
{
    if (values.size() == 1 && values[0].isNumber() && values[0].number() == 42)
        return Value(NFT::True);
    return Value(NFT::False);
}

int main()
{
    VM vm;
    Compiler compiler;
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
