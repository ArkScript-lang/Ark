#include <Ark/TypeChecker.hpp>

#include <limits>
#include <numeric>
#include <algorithm>
#define NOMINMAX
#include <termcolor/termcolor.hpp>

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

            std::cout << "  -> " << (td.variadic ? "variadic " : "")
                      << (correct ? termcolor::green : termcolor::magenta) << td.name << termcolor::reset << " (" << arg_str << ") ";
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
                    std::cout << termcolor::red << bad_type << termcolor::reset
                              << " argument" << (bad_type > 1 ? "s" : "") << " do not match";
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
                    std::cout << "was of type " << termcolor::red << types_to_str[static_cast<std::size_t>(args[i].valueType())];
                }
                // non-provided argument
                else if (i >= args.size())
                {
                    displayArg(td, false);
                    std::cout << termcolor::red << "was not provided";
                }
                else
                    displayArg(td, true);
            }
            std::cout << termcolor::reset << "\n";
        }
    }

    [[noreturn]] void generateError(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args)
    {
        std::cout << "Function " << termcolor::blue << funcname << termcolor::reset << " expected ";

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
            std::cout << "between "
                      << termcolor::yellow << min_argc << termcolor::reset
                      << " argument" << (min_argc > 1 ? "s" : "") << " and "
                      << termcolor::yellow << max_argc << termcolor::reset
                      << " argument" << (max_argc > 1 ? "s" : "");

            if (sanitizedArgs.size() < min_argc || sanitizedArgs.size() > max_argc)
                correct_argcount = false;
        }
        else
        {
            std::cout << termcolor::yellow << min_argc << termcolor::reset
                      << " argument" << (min_argc > 1 ? "s" : "");

            if (sanitizedArgs.size() != min_argc)
                correct_argcount = false;
        }

        if (!correct_argcount)
            std::cout << " but got " << termcolor::red << sanitizedArgs.size();

        std::cout << termcolor::reset << "\n";

        displayContract(contracts[0], sanitizedArgs);
        for (std::size_t i = 1, end = contracts.size(); i < end; ++i)
        {
            std::cout << "Alternative " << (i + 1) << ":\n";
            displayContract(contracts[i], sanitizedArgs);
        }

        throw TypeError("");
    }
}
