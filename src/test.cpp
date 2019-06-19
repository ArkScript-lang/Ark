#include <iostream>
#include <string>
#include <vector>

#include <Ark/Ark.hpp>

const char* g_code =
    "(begin\n"
    "    (let a 42)\n"
    "    (let b \"hello world\")\n"
    "    (let life [fun (x) {* x 2}])\n"
    ")\n"
    "";

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
    // testing interpreter
    Ark::Lang::Program program;
    program.feed(g_code);
    program.execute();
    
    auto a = program.get<double>("a");
    auto b = program.get<std::string>("b");
    auto life = program.get<Ark::Function>("life");
    
    std::cout << a << " " << b << " " << life(a) << std::endl;

    // testing virtual machine
    Ark::VM::VM vm;
    Ark::Compiler::Compiler compiler;
    compiler.feed(g_code_vm);
    compiler.compile();
    vm.feed(compiler.bytecode());

    vm.loadFunction("do-stuff", &dostuff);

    vm.run();
    
    return 0;
}
