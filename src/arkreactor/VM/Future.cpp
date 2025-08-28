#include <Ark/VM/Future.hpp>

#include <Ark/VM/VM.hpp>
#include <Ark/VM/DefaultValues.hpp>

namespace Ark::internal
{
    // cppcheck-suppress constParameterReference
    Future::Future(ExecutionContext* context, VM* vm, std::vector<Value>& args)
    {
        ControlFunctions.ostream_func = [](std::ostream& os, const UserType& user) -> std::ostream& {
            os << "Future@" << user.data();
            return os;
        };
        ControlFunctions.deleter = [vm](void* data) {
            vm->deleteFuture(static_cast<Future*>(data));
        };

        m_value = std::async(
            std::launch::async,
            [vm, context, args]() mutable {
                const Value res = vm->resolve(context, args);
                // We need to mark the context as free to use as soon as possible,
                // because if we do it in `Future::resolve`, it will never be marked as free
                // if the future is not awaited, even though it can already be reused.
                // The value is returned as a copy by VM::resolve, and not a reference,
                // thus it is okay to get rid of the context.
                vm->deleteContext(context);
                return res;
            });
    }

    Value Future::resolve()
    {
        if (!m_value.valid())
            return Nil;

        m_value.wait();
        Value res = m_value.get();
        return res;
    }
}
