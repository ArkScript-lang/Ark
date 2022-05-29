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
        // TODO add typechecking
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
        // TODO add typechecking
        Future& p = n[0].usertypeRef().as<Future>();
        Value res = p.resolve();
        // TODO find a way to remove the future from the VM

        return res;
    }
}
