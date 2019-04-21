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
    
    auto a = program.get<dozerg::HugeNumber>("a");
    auto b = program.get<std::string>("b");
    auto life = program.get<Ark::Function>("life");
    
    std::cout << a << " " << b << " " << life(a) << std::endl;
    
    return 0;
}
