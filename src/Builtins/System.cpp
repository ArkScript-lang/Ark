#include <Ark/Builtins/Builtins.hpp>

#include <stdio.h>
#include <memory>
#include <cstdlib>
#include <thread>

#ifdef _MSC_VER
    #define popen _popen
    #define pclose _pclose
#endif

#undef abs
#include <chrono>

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

        #ifdef ARK_ENABLE_SYSTEM
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(n[0].string().c_str(), "r"), pclose);
            if (!pipe)
                throw std::runtime_error("sys:exec: couldn't retrieve command output");
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
                result += buffer.data();
            return Value(result);
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