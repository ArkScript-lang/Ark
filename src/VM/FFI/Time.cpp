#include <Ark/VM/FFI.hpp>

#include <thread>

#undef abs
#include <chrono>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::Time
{
    FFI_Function(timeSinceEpoch)
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        return Value(static_cast<double>(microseconds.count()) / 1000000);
    }

    FFI_Function(sleep)
    {
        if (n.size() != 1)
            throw std::runtime_error("sleep can take only one argument, a duration (milliseconds)");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of sleep must be of type Number");
        
        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);
        
        return nil;
    }
}