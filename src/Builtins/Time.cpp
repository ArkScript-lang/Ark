#include <Ark/Builtins/Builtins.hpp>

#undef abs
#include <chrono>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Time
{
    Value timeSinceEpoch(std::vector<Value>& n, Ark::VM* vm)
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        return Value(static_cast<double>(microseconds.count()) / 1000000);
    }
}