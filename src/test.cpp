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
