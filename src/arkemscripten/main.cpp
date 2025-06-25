#include <Ark/Ark.hpp>
#include <emscripten.h>
#include <emscripten/bind.h>

#include <string>

extern "C" {
void __attribute__((noinline)) EMSCRIPTEN_KEEPALIVE run(const std::string& code)
{
    Ark::State state;
    state.doString(code);

    Ark::VM vm(state);
    vm.run();
}
}

EMSCRIPTEN_BINDINGS(arkscript)
{
    using namespace emscripten;
    function("run", &run);
}

int main()
{
    return 0;
}
