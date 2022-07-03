#include <Ark/VM/Future.hpp>

#include <Ark/VM/VM.hpp>

namespace Ark::internal
{
    Future::Future(ExecutionContext* context, VM* vm, std::vector<Value>& args) :
        m_context(context), m_vm(vm)
    {
        m_value = std::async(std::launch::async, [vm, context, args]() mutable {
            return vm->resolve(context, args);
        });
    }

    Value Future::resolve()
    {
        m_value.wait();
        Value res = m_value.get();

        m_vm->deleteContext(m_context);
        m_context = nullptr;

        return res;
    }
}
