#include <Ark/Builtins/Builtins.hpp>

#include <thread>

#undef abs
#include <chrono>

#include <cstdlib>
#include <Ark/Constants.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::System
{
    Value system_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(SYS_SYS_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(SYS_SYS_TE0);

        #if ARK_ENABLE_SYSTEM != 0
            int output = std::system(n[0].string().c_str());
            return Value(output);
        #endif  // ARK_ENABLE_SYSTEM

        return nil;
    }

    Value sleep(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(SYS_SLEEP_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(SYS_SLEEP_TE0);

        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);

        return nil;
    }

    Value exit_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(SYS_EXIT_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(SYS_EXIT_TE0);

        vm->exit(static_cast<int>(n[0].number()));

        return nil;
    }
}