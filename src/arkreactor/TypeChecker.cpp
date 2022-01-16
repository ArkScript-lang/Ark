#include <Ark/TypeChecker.hpp>

#include <limits>
#include <algorithm>
#include <termcolor/termcolor.hpp>

#include <Ark/Exceptions.hpp>

namespace Ark::internal::types
{
    // TODO create a function to generate the basic skeleton of an error message

    std::size_t typeChecker(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& provided_arguments)
    {
        for (std::size_t i = 0, end = contracts.size(); i < end; ++i)
        {
            const Contract& c = contracts[i];

            // not checking if we have only one variadic argument and if it's placed at the end only
            // this is something the developper should take care of themself,
            // otherwise checking for this would result in repeated performance hits

            if (c.arguments.empty() && provided_arguments.empty())
                return i;

            if (provided_arguments.size() == c.arguments.size() && !c.arguments.back().variadic)
            {
                // number of argument correct, no variadic at the end
                for (std::size_t j = 0, arg_end = c.arguments.size(); j < arg_end; ++j)
                {
                    const Contract::Typedef& td = c.arguments[j];
                    if (std::find(td.types.begin(), td.types.end(), provided_arguments[j].valueType()) == td.types.end())
                        throw TypeError("smh");  // TODO generate the message
                }

                return i;
            }
            else if (provided_arguments.size() >= c.arguments.size() - 1 && c.arguments.back().variadic)
            {
                // number of arguments is at least the number of required arguments (not counting variadic)
                // check the regular arguments first
                std::size_t j = 0;
                for (std::size_t arg_end = c.arguments.size() - 1; j < arg_end; ++j)
                {
                    const Contract::Typedef& td = c.arguments[j];
                    if (std::find(td.types.begin(), td.types.end(), provided_arguments[j].valueType()) == td.types.end())
                        throw TypeError("smh");  // TODO generate the message
                }

                const Contract::Typedef& var_td = c.arguments[j];
                // check the variadic arguments
                for (std::size_t arg_end = provided_arguments.size(); j < arg_end; ++j)
                {
                    if (std::find(var_td.types.begin(), var_td.types.end(), provided_arguments[j].valueType()) == var_td.types.end())
                        throw TypeError("smh");  // TODO generate the message
                }

                return i;
            }
        }

        // no match, the user most likely provided the wrong number of arguments
        std::cout << termcolor::green << funcname << termcolor::reset;

        if (contracts.size() == 1)
        {
            std::size_t expected_argc = contracts[0].arguments.size();
            std::cout << " expected " << expected_argc << " argument" << (expected_argc > 1 ? "s" : "")
                      << " but got " << provided_arguments.size() << "\n";
            // TODO display contract
        }
        else
        {}

        throw Error("smh");  // TODO generate the error
    }
}
