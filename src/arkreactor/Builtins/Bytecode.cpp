#include <Ark/Builtins/Builtins.hpp>

#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/VM.hpp>

#include <Ark/Utils/Files.hpp>
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
        if (!types::check(n, ValueType::PageAddr) && !types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "disassemble",
                { { types::Contract { { types::Typedef("f", ValueType::PageAddr) } },
                    types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

        BytecodeReader bcr;
        if (n[0].valueType() == ValueType::PageAddr)
        {
            bcr.feed(vm->bytecode());
            bcr.display(BytecodeSegment::Code, std::nullopt, std::nullopt, n[0].pageAddr());
        }
        else if (n[0].valueType() == ValueType::String)
        {
            const std::string filename = n[0].string();
            if (!Utils::fileExists(filename))
                throw Error(fmt::format("`disassemble': can not read file {}", filename));

            bcr.feed(filename);
            bcr.display();
        }

        return Nil;
    }
}
