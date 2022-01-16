#include <Ark/TypeChecker.hpp>

#include <limits>
#include <numeric>
#include <algorithm>
#define NOMINMAX
#include <termcolor/termcolor.hpp>

#include <Ark/Exceptions.hpp>

namespace Ark::internal::types
{
    std::string typeListToString(const std::vector<ValueType>& types)
    {
        if (types.size() == AnyType.size())  // without more checks we'll assume they are the same
            return "any";

        return std::accumulate(
            types.begin(),
            types.end(),
            std::string(""),
            [](const std::string& a, ValueType type) -> std::string {
                if (a.empty())
                    return types_to_str[static_cast<std::size_t>(type)];
                return a + ", " + types_to_str[static_cast<std::size_t>(type)];
            });
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
                    if (std::find(td.types.begin(), td.types.end(), args[j].valueType()) == td.types.end())
                        bad_type++;
                }

                if (bad_type)
                    std::cout << termcolor::red << bad_type << termcolor::reset
                              << " argument" << (bad_type > 1 ? "s" : "") << " do not match";
            }
            else
            {
                // provided argument but wrong type
                if (i < args.size() && std::find(td.types.begin(), td.types.end(), args[i].valueType()) == td.types.end())
                    std::cout << "was of type " << termcolor::yellow << types_to_str[static_cast<std::size_t>(args[i].valueType())];
                // non-provided argument
                else if (i >= args.size())
                    std::cout << termcolor::red << "was not provided";
            }
            std::cout << termcolor::reset << "\n";
        }
    }

    std::size_t checker(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& provided_arguments)
    {
        for (std::size_t i = 0, end = contracts.size(); i < end; ++i)
        {
            const Contract& c = contracts[i];

            // not checking if we have only one variadic argument and if it's placed at the end only
            // this is something the developper should take care of themself,
            // otherwise checking for this would result in repeated performance hits

            if (c.arguments.empty() && provided_arguments.empty())
                return i;

            if ((provided_arguments.size() == c.arguments.size() && !c.arguments.back().variadic) ||
                (provided_arguments.size() >= c.arguments.size() - 1 && c.arguments.back().variadic))
            {
                std::size_t j = 0;
                std::size_t arg_end = c.arguments.size();

                // need to check only the first arguments in case of a variadic type definition in the contract
                if (c.arguments.back().variadic)
                    arg_end--;

                for (; j < arg_end; ++j)
                {
                    const Typedef& td = c.arguments[j];
                    if (std::find(td.types.begin(), td.types.end(), provided_arguments[j].valueType()) == td.types.end())
                    {
                        std::cout << "TypeError\n";
                        displayContract(c, provided_arguments);
                        throw Error("");
                    }
                }

                // check the variadic part if any
                if (c.arguments.back().variadic)
                {
                    const Typedef& var_td = c.arguments[j];
                    // check the variadic arguments
                    for (std::size_t provided_end = provided_arguments.size(); j < provided_end; ++j)
                    {
                        if (std::find(var_td.types.begin(), var_td.types.end(), provided_arguments[j].valueType()) == var_td.types.end())
                        {
                            std::cout << "TypeError\n";
                            displayContract(c, provided_arguments);
                            throw Error("");
                        }
                    }
                }

                return i;
            }
        }

        // no match, the user most likely provided the wrong number of arguments
        std::cout << termcolor::green << funcname << termcolor::reset << " expected ";

        if (contracts.size() == 1)
        {
            std::size_t expected_argc = contracts[0].arguments.size();
            std::cout << termcolor::yellow << expected_argc << termcolor::reset
                      << " argument" << (expected_argc > 1 ? "s" : "");
        }
        else
        {
            std::size_t min_argc = std::numeric_limits<std::size_t>::max(), max_argc = 0;
            for (const Contract& c : contracts)
            {
                if (c.arguments.size() < min_argc)
                    min_argc = c.arguments.size();
                if (c.arguments.size() > max_argc)
                    max_argc = c.arguments.size();
            }

            std::cout << "between "
                      << termcolor::yellow << min_argc << termcolor::reset
                      << " argument" << (min_argc > 1 ? "s" : "") << " and "
                      << termcolor::yellow << max_argc << termcolor::reset
                      << " argument" << (max_argc > 1 ? "s" : "");
        }
        std::cout << " but got " << provided_arguments.size() << "\n";

        displayContract(contracts[0], provided_arguments);
        for (std::size_t i = 1, end = contracts.size(); i < end; ++i)
        {
            std::cout << "Alternative " << (i + 1) << ":\n";
            displayContract(contracts[i], provided_arguments);
        }

        throw Error("");
    }
}
