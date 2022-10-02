#include <Ark/TypeChecker.hpp>

#include <limits>
#include <numeric>
#include <algorithm>
#include <fmt/color.h>

#include <Ark/Exceptions.hpp>

namespace Ark::types
{
    std::string typeListToString(const std::vector<ValueType>& types)
    {
        if (types.size() == 1 && types[0] == ValueType::Any)
            return "any";

        std::string acc = "";

        for (std::size_t i = 0, end = types.size(); i < end; ++i)
        {
            if (i > 0)
                acc += ", ";
            acc += types_to_str[static_cast<std::size_t>(types[i])];
        }
        return acc;
    }

    void displayContract(const Contract& contract, const std::vector<Value>& args)
    {
        auto displayArg = [](const Typedef& td, bool correct) {
            std::string arg_str = typeListToString(td.types);

            fmt::print(
                "  -> {}{} ({}) ",
                (td.variadic ? "variadic " : ""),
                fmt::styled(td.name, fmt::fg(correct ? fmt::color::green : fmt::color::magenta)),
                arg_str);
        };

        for (std::size_t i = 0, end = contract.arguments.size(); i < end; ++i)
        {
            const Typedef& td = contract.arguments[i];

            if (td.variadic && i < args.size())
            {
                // variadic argument in contract and enough provided arguments
                std::size_t bad_type = 0;
                for (std::size_t j = i, args_end = args.size(); j < args_end; ++j)
                {
                    if (td.types[0] != ValueType::Any && std::find(td.types.begin(), td.types.end(), args[j].valueType()) == td.types.end())
                        bad_type++;
                }

                if (bad_type)
                {
                    displayArg(td, false);
                    fmt::print("{} argument{} do not match", fmt::styled(bad_type, fmt::fg(fmt::color::red)), (bad_type > 1 ? "s" : ""));
                }
                else
                    displayArg(td, true);
            }
            else
            {
                // provided argument but wrong type
                if (i < args.size() && td.types[0] != ValueType::Any && std::find(td.types.begin(), td.types.end(), args[i].valueType()) == td.types.end())
                {
                    displayArg(td, false);
                    fmt::print("was of type {}", fmt::styled(types_to_str[static_cast<std::size_t>(args[i].valueType())], fmt::fg(fmt::color::red)));
                }
                // non-provided argument
                else if (i >= args.size())
                {
                    displayArg(td, false);
                    fmt::print(fmt::fg(fmt::color::red), "was not provided");
                }
                else
                    displayArg(td, true);
            }
            fmt::print("\n");
        }
    }

    [[noreturn]] void generateError(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args)
    {
        fmt::print("Function {} expected ", fmt::styled(funcname, fmt::fg(fmt::color::blue)));

        std::vector<Value> sanitizedArgs;
        std::copy_if(args.begin(), args.end(), std::back_inserter(sanitizedArgs), [](const Value& value) -> bool {
            return value.valueType() != ValueType::Undefined;
        });

        // get expected arguments count
        std::size_t min_argc = std::numeric_limits<std::size_t>::max(), max_argc = 0;
        for (const Contract& c : contracts)
        {
            if (c.arguments.size() < min_argc)
                min_argc = c.arguments.size();
            if (c.arguments.size() > max_argc)
                max_argc = c.arguments.size();
        }

        bool correct_argcount = true;

        if (min_argc != max_argc)
        {
            fmt::print(
                "between {} argument{} and {} argument{}\n",
                fmt::styled(min_argc, fmt::fg(fmt::color::yellow)),
                (min_argc > 1 ? "s" : ""),
                fmt::styled(max_argc, fmt::fg(fmt::color::yellow)),
                (max_argc > 1 ? "s" : ""));

            if (sanitizedArgs.size() < min_argc || sanitizedArgs.size() > max_argc)
                correct_argcount = false;
        }
        else
        {
            fmt::print("{} argument{}\n", fmt::styled(min_argc, fmt::fg(fmt::color::yellow)), (min_argc > 1 ? "s" : ""));

            if (sanitizedArgs.size() != min_argc)
                correct_argcount = false;
        }

        if (!correct_argcount)
            fmt::print(" but got {}\n", fmt::styled(sanitizedArgs.size(), fmt::fg(fmt::color::red)));

        displayContract(contracts[0], sanitizedArgs);
        for (std::size_t i = 1, end = contracts.size(); i < end; ++i)
        {
            fmt::print("Alternative {}:\n", i + 1);
            displayContract(contracts[i], sanitizedArgs);
        }

        throw TypeError("");
    }
}
