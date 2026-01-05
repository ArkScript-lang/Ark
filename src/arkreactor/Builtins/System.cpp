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

    // cppcheck-suppress constParameterReference
    Value system_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "sys:exec",
                { { types::Contract { { types::Typedef("command", ValueType::String) } } } },
                n);

#if defined(ARK_ENABLE_SYSTEM) && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        std::array<char, 128> buffer {};
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

    // cppcheck-suppress constParameterReference
    Value sleep(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "sys:sleep",
                { { types::Contract { { types::Typedef("duration", ValueType::Number) } } } },
                n);

        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);

        return nil;
    }

    // cppcheck-suppress constParameterReference
    Value exit_(std::vector<Value>& n, VM* vm)
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "sys:exit",
                { { types::Contract { { types::Typedef("exitCode", ValueType::Number) } } } },
                n);

        vm->exit(static_cast<int>(n[0].number()));
        return nil;
    }

    // cppcheck-suppress constParameterReference
    Value assert_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Any, ValueType::String))
            throw types::TypeCheckingError(
                "assert",
                { { types::Contract { { types::Typedef("expr", ValueType::Any), types::Typedef("message", ValueType::String) } } } },
                n);

        if (n[0] == Builtins::falseSym)
            throw AssertionFailed(n[1].stringRef());

        return nil;
    }
}
