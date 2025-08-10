#include <Ark/Module.hpp>

#include <Ark/VM/DefaultValues.hpp>

// cppcheck-suppress [constParameterReference, constParameterPointer]
Ark::Value foo(std::vector<Ark::Value>& n, Ark::VM* vm [[maybe_unused]])
{
    if (n.size() == 1)
        return n[0];
    return Ark::Nil;
}

ARK_API Ark::mapping* getFunctionsMapping()
{
    static Ark::mapping map[] = {
        { "testmodule:foo", foo },
        { nullptr, nullptr }
    };

    return map;
}
