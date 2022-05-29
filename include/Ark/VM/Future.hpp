/**
 * @file Future.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ARK_VM_FUTURE_HPP
#define ARK_VM_FUTURE_HPP

#include <future>
#include <vector>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/ExecutionContext.hpp>

namespace Ark::internal
{
    class Future
    {
    public:
        Future(ExecutionContext* context, VM* vm, std::vector<Value>& args);

        Value resolve();

    private:
        ExecutionContext* m_context;
        VM* m_vm;
        std::future<Value> m_value;
    };
}

#endif
