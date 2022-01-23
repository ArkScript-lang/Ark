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
        if (types.size() == AnyType.size())  // without more checks we'll assume they are the same
            return "any";

        std::string acc = "";

        for (std::size_t i = 0, end = types.size(); i < end; ++i)
        {
            if (i > 0)
                acc += ", ";
            acc += types_to_str[i];
        }
        return acc;
    }

    void displayContract(const Contract& contract, const std::vector<Value>& args)
    {
        for (std::size_t i = 0, end = contract.arguments.size(); i < end; ++i)
        {
            const Typedef& td = contract.arguments[i];
            std::string arg_str = typeListToString(td.types);

            std::cout << "  -> " << (td.variadic ? "variadic " : "")
                      << termcolor::green << td.name << termcolor::reset << " (" << arg_str << ") ";

            if (td.variadic && i < args.size())
            {
                // variadic argument in contract and enough provided arguments
                std::size_t bad_type = 0;
                for (std::size_t j = i, args_end = args.size(); j < args_end; ++j)
                {
                    if (td.types[j] != args[j].valueType())
                        bad_type++;
                }

                if (bad_type)
                    std::cout << termcolor::red << bad_type << termcolor::reset
                              << " argument" << (bad_type > 1 ? "s" : "") << " do not match";
            }
            else
            {
                // provided argument but wrong type
                if (i < args.size() && td.types[i] != args[i].valueType())
                    std::cout << "was of type " << termcolor::yellow << types_to_str[static_cast<std::size_t>(args[i].valueType())];
                // non-provided argument
                else if (i >= args.size())
                    std::cout << termcolor::red << "was not provided";
            }
            std::cout << termcolor::reset << "\n";
        }
    }


    void generateError(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args)
    {
        std::cout << termcolor::green << funcname << termcolor::reset << " expected ";

        // get expected arguments count
        std::size_t min_argc = std::numeric_limits<std::size_t>::max(), max_argc = 0;
        for (const Contract& c : contracts)
        {
            if (c.arguments.size() < min_argc)
                min_argc = c.arguments.size();
            if (c.arguments.size() > max_argc)
                max_argc = c.arguments.size();
        }

        if (min_argc != max_argc)
        {
            std::cout << "between "
                      << termcolor::yellow << min_argc << termcolor::reset
                      << " argument" << (min_argc > 1 ? "s" : "") << " and "
                      << termcolor::yellow << max_argc << termcolor::reset
                      << " argument" << (max_argc > 1 ? "s" : "");

            if (args.size() < min_argc || args.size() > max_argc)
                std::cout << " but got " << args.size() << "\n";
        }
        else
        {
            std::size_t expected_argc = contracts[0].arguments.size();
            std::cout << termcolor::yellow << expected_argc << termcolor::reset
                      << " argument" << (expected_argc > 1 ? "s" : "");

            if (args.size() != expected_argc)
                std::cout << " but got " << args.size() << "\n";
        }

        displayContract(contracts[0], args);
        for (std::size_t i = 1, end = contracts.size(); i < end; ++i)
        {
            std::cout << "Alternative " << (i + 1) << ":\n";
            displayContract(contracts[i], args);
        }

        throw Error("");
    }
}
