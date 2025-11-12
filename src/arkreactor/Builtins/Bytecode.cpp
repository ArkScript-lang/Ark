#include <Ark/Builtins/Builtins.hpp>

#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/VM.hpp>

#include <Ark/VM/DefaultValues.hpp>
#include <Ark/TypeChecker.hpp>

namespace Ark::internal::Builtins::Bytecode
{
    /**
     * @name disassemble
     * @brief Prints the bytecode of a given function
     * @param f function to disassemble
     * =begin
     * (let foo (fun () { (let a 1) (print a) })
     * (disassemble foo)
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value disassemble(std::vector<Value>& n, VM* vm)
    {
        if (!types::check(n, ValueType::PageAddr))
            throw types::TypeCheckingError(
                "disassemble",
                { { types::Contract { { types::Typedef("f", ValueType::PageAddr) } } } },
                n);

        BytecodeReader bcr;
        bcr.feed(vm->bytecode());
        bcr.display(BytecodeSegment::Code, std::nullopt, std::nullopt, n[0].pageAddr());

        return Nil;
    }
}
