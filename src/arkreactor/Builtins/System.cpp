#include <Ark/Builtins/Builtins.hpp>

#include <stdio.h>
#include <memory>
#include <cstdlib>
#include <thread>

#ifdef _MSC_VER
#    define popen _popen
#    define pclose _pclose
#endif

#undef abs
#include <chrono>

#include <Ark/Constants.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::System
{
    namespace
    {
        struct close_file_deleter
        {
            int operator()(FILE* file) const noexcept
            {
                return pclose(file);
            }
        };
    }

    /**
     * @name sys:exec
     * @brief Execute a system specific command
     * @details Return the output of the command as a String, or nil if it was disabled in the ArkScript build
     * @param command the command to execute, as a String
     * =begin
     * (sys:exec "echo hello")
     * =end
     * @author https://github.com/SuperFola
     */
    Value system_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "sys:exec",
                { { types::Contract { { types::Typedef("command", ValueType::String) } } } },
                n);

#ifdef ARK_ENABLE_SYSTEM
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, close_file_deleter> pipe(popen(n[0].string().c_str(), "r"), close_file_deleter());
        if (!pipe)
            throw std::runtime_error("sys:exec: couldn't retrieve command output");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
        return Value(result);
#else
        return nil;
#endif  // ARK_ENABLE_SYSTEM
    }

    /**
     * @name sys:sleep
     * @brief Sleep for a given duration (in milliseconds)
     * @details Return nil
     * @param duration a Number representing a duration
     * =begin
     * (sys:sleep 1000)  # sleep for 1 second
     * =end
     * @author https://github.com/SuperFola
     */
    Value sleep(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            types::generateError(
                "sys:sleep",
                { { types::Contract { { types::Typedef("duration", ValueType::Number) } } } },
                n);

        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);

        return nil;
    }

    /**
     * @name sys:exit
     * @brief Reverse a given list and return a new one
     * @details Any code after this function call won't be executed
     * @param exitCode usually 0 for success and 1 for errors
     * =begin
     * (sys:exit 0)  # halt the virtual machine with given exit code (success)
     * =end
     * @author https://github.com/SuperFola
     */
    Value exit_(std::vector<Value>& n, VM* vm)
    {
        if (!types::check(n, ValueType::Number))
            types::generateError(
                "sys:exit",
                { { types::Contract { { types::Typedef("exitCode", ValueType::Number) } } } },
                n);

        vm->exit(static_cast<int>(n[0].number()));
        return nil;
    }
}
