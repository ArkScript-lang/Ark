/**
 * @file Future.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Internal object to resolve asynchronously a function call in ArkScript
 * @date 2022-05-28
 *
 * @copyright Copyright (c) 2022-2026
 *
 */

#ifndef ARK_VM_FUTURE_HPP
#define ARK_VM_FUTURE_HPP

#include <future>
#include <vector>

#include <Ark/VM/Value/Value.hpp>
#include <Ark/VM/ExecutionContext.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    class Future
    {
    public:
        /**
         * @brief Create a Future and immediately start it through std::async
         * @param context a dedicated context for the future to run on
         * @param vm non owning pointer to the VM
         * @param args list of (function, arguments...) to create the future
         */
        Future(ExecutionContext* context, VM* vm, std::vector<Value>& args);

        /**
         * @brief Await the future, blocking the thread it is run on
         * @return Value Nil if the future is invalid (has already been awaited), otherwise the value
         */
        Value resolve();

        static UserType::ControlFuncs ControlFunctions;

    private:
        std::future<Value> m_value;  ///< The actual thread
        VM* m_vm;                    ///< Non-owning pointer

        void deleteSelfViaVM();
    };
}

#endif
