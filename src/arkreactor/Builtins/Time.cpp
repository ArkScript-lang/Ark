#include <Ark/Builtins/Builtins.hpp>

#undef abs
#include <chrono>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Time
{
    /**
     * @name time
     * @brief Return the time of the computer since epoch, in seconds, with at least milliseconds precision
     * =begin
     * (time)  # 1627134107.837558031082153
     * =end
     * @author https://github.com/SuperFola
     */
    Value timeSinceEpoch(std::vector<Value>& n [[maybe_unused]], VM* vm [[maybe_unused]])
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(epoch);
        return Value(static_cast<double>(microseconds.count()) / 1000000);
    }
}
