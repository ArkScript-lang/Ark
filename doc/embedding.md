# Embedding Ark

## Example

```cpp
#include <iostream>
#include <string>

#define ARKLANG
#include <Ark/Ark.hpp>

const char* g_code =
    "(begin\n"
    "    (def a 42)\n"
    "    (def b 1.67)\n"
    "    (def c \"hello world\")\n"
    "    (def life [fun (x) {* x 2}])\n"
    ")\n"
    "";

int main()
{
    Ark::Lang::Program program;
    program.feed(g_code);
    program.execute();
    
    auto a = program.get<int>("a");
    auto b = program.get<float>("b");
    auto c = program.get<std::string>("c");
    auto life = program.get<Ark::Function>("life");
    
    std::cout << a << " " << b << " " << c << " " << life(a) << std::endl;
    
    return 0;
}
```

## More details

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
auto my_integer = program.get<int>("my_ark_integer");
```

The types supported by `.get<>` are: int, float, std::string and Function.

A Function can take int, float, double (converted to double), and std::string (multiple arguments are supported).

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