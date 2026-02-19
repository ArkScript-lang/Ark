#include <Ark/Builtins/Builtins.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins
{
    /**
     * @name slice
     * @brief Slice a list or string given a start, an end, and an optional step size
     * @param container list or string
     * @param start number, included
     * @param end number, excluded
     * @param step number, default 1
     * =begin
     * (let d (dict "key" "value" 5 12))
     * (print d)  # {key: value, 5: 12}
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value slice(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() < 3 || n.size() > 4 ||
            (n[0].valueType() != ValueType::List && n[0].valueType() != ValueType::String) ||
            (n[1].valueType() != ValueType::Number && n[1].valueType() != ValueType::Nil) ||
            (n[2].valueType() != ValueType::Number && n[2].valueType() != ValueType::Nil) ||
            (n.size() == 4 && n[3].valueType() != ValueType::Number))
            throw types::TypeCheckingError(
                "slice",
                { { types::Contract {
                        { types::Typedef("container", { ValueType::List, ValueType::String }),
                          types::Typedef("start", ValueType::Number),
                          types::Typedef("end", ValueType::Number) } },
                    types::Contract {
                        { types::Typedef("container", { ValueType::List, ValueType::String }),
                          types::Typedef("start", ValueType::Number),
                          types::Typedef("end", ValueType::Number),
                          types::Typedef("step", ValueType::Number) } } } },
                n);

        const bool is_list = n[0].valueType() == ValueType::List;
        const std::size_t container_size = is_list ? n[0].constList().size() : n[0].string().size();

        const long start = n[1].valueType() == ValueType::Number ? static_cast<long>(n[1].number()) : 0;
        const std::size_t i = static_cast<std::size_t>(start < 0 ? static_cast<long>(container_size) + start : start);

        if (i >= container_size)
            VM::throwVMError(
                ErrorKind::Index,
                fmt::format("{} out of range {} (length {})", start, n[0].toString(*vm, /* show_as_code= */ true), container_size));

        const long end = n[2].valueType() == ValueType::Number ? static_cast<long>(n[2].number()) : static_cast<long>(container_size);
        const std::size_t j = std::min(container_size, static_cast<std::size_t>(end < 0 ? static_cast<long>(container_size) + end : end));

        const long step = n.size() == 4 ? static_cast<long>(n[3].number()) : 1L;
        if (step == 0)
            throw Error("slice: a step of 0 is illegal");

        std::size_t a = step > 0 ? i : j - 1;
        const std::size_t b = step > 0 ? j : i;
        const std::size_t increments = static_cast<std::size_t>(std::abs(step));

        if (is_list)
        {
            Value output(ValueType::List);
            while ((step > 0 && a < b) || (step < 0 && a >= b))
            {
                output.push_back(n[0].constList()[a]);

                if (step > 0)
                    a += increments;
                else if (a >= increments)
                    a -= increments;
                else
                    break;  // step < 0 and 'increments' is bigger than 'a'
            }
            return output;
        }

        std::string output;
        while ((step > 0 && a < b) || (step < 0 && a >= b))
        {
            output += n[0].string()[a];

            if (step > 0)
                a += increments;
            else if (a >= increments)
                a -= increments;
            else
                break;  // step < 0 and 'increments' is bigger than 'a'
        }
        return Value(output);
    }
}
