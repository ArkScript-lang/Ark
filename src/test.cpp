#include <iostream>
#include <string>

#define ARKLANG
#include <Ark/Lang/Program.hpp>

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
    
    int a = program["a"];
    float b = program["b"];
    std::string c = program["c"];
    Ark::Function life = program["life"];
    
    std::cout << a << " " << b << " " << c << " " << life(a) << std::endl;
    
    return 0;
}
