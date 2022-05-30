#include <Ark/Builtins/Builtins.hpp>

#include <memory>
#include <thread>

#include <Ark/Constants.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Async
{
    /**
     * @name async
     * @brief Calls a function asynchronously with a given set of arguments
     * @details The function is started in a separate context, with no access to the others, preventing any concurrency problems.
     * @param func the function to call
     * @param args... the arguments of the function
     * =begin
     * (let foo (fun (a b) (+ a b)))
     * (async foo 1 2)
     * =end
     * @author https://github.com/SuperFola
     */
    Value async(std::vector<Value>& n, VM* vm)
    {
        if (n.size() == 0)
            types::generateError(
                "async", { { types::Contract { { types::Typedef("function", ValueType::PageAddr) } }, types::Contract { { types::Typedef("function", ValueType::CProc) } }, types::Contract { { types::Typedef("function", ValueType::Closure) } } } }, n);

        Future* future = vm->createFuture(n);
        return Value(UserType(future));
    }

    /**
     * @name await
     * @brief Blocks until the result becomes available
     * @details
     * @param future the future to wait for its result to be avaible
     * =begin
     * (let foo (fun (a b) (+ a b)))
     * (let async-foo (async foo 1 2))
     * (print (await async-foo))
     * =end
     * @author https://github.com/SuperFola
     */
    Value await(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::User) || !n[0].usertypeRef().is<Future>())
            types::generateError("await", { { types::Contract { { types::Typedef("future", ValueType::User) } } } }, n);

        Future& f = n[0].usertypeRef().as<Future>();
        Value res = f.resolve();
        vm->deleteFuture(&f);

        return res;
    }
}
