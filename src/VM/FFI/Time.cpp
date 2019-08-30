#include <Ark/VM/FFI.hpp>

#include <thread>

#undef abs
#include <chrono>

namespace Ark::internal::FFI
{
    FFI_Function(timeSinceEpoch)
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return Value(static_cast<double>(milliseconds.count()) / 1000);
    }

    FFI_Function(sleep)
    {
        if (n.size() != 1)
            throw std::runtime_error("sleep can take only one argument, a duration (milliseconds)");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of sleep must be of type Number");
        
        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);
        
        return nil;
    }
}