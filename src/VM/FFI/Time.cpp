#include <Ark/VM/FFI.hpp>

#include <thread>

#undef abs
#include <chrono>

#include <Ark/VM/FFIErrors.inl>
#define FFI_Function(name) Value name(std::vector<Value>& n)

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
            throw std::runtime_error(TIME_SLEEP_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(TIME_SLEEP_TE0);
        
        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);
        
        return nil;
    }
}