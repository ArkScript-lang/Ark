#include <Ark/Exceptions.hpp>

#include <numeric>
#include <termcolor/termcolor.hpp>

#include <Ark/Utils.hpp>

namespace Ark
{
    BetterTypeError::BetterTypeError(std::string_view func_name, std::size_t expected_argc, const std::vector<Value>& args) :
        Error(), m_funcname(func_name), m_arg_index(0), m_expected_argc(expected_argc), m_args(args)
    {
        std::cout << func_name << ": needs " << expected_argc;
        if (expected_argc <= 1)
            std::cout << " argument,";
        else
            std::cout << " arguments,";
        std::cout << (args.size() == expected_argc ? "" : " but") << " got " << args.size() << std::endl;
    }

    BetterTypeError& BetterTypeError::withArg(std::string_view arg_name, const std::vector<ValueType>& arg_types)
    {
        std::string arg_str;
        std::vector<std::string> args_str;

        for (const ValueType& t : arg_types)
        {
            std::size_t expected_type = static_cast<std::size_t>(t);
            args_str.push_back(types_to_str[expected_type]);
        }

        if (arg_types.empty())
            arg_str = "any";
        else
            arg_str = std::accumulate(
                std::next(args_str.begin()),
                args_str.end(),
                args_str[0],
                [](const std::string& a, const std::string& b) -> std::string {
                    return a + ", " + b;
                });

        // argument has been provided
        if (m_arg_index < m_args.size())
        {
            std::size_t provided_type = static_cast<std::size_t>(m_args[m_arg_index].valueType());

            if (std::find(arg_types.begin(), arg_types.end(), m_args[m_arg_index].valueType()) == arg_types.end())
                std::cout << termcolor::yellow;

            std::cout << "  -> " << arg_name << " (" << arg_str << ") was of type " << types_to_str[provided_type] << termcolor::reset << std::endl;
        }
        // argument was not provided
        else
        {
            std::cout << termcolor::yellow;
            std::cout << "  -> " << arg_name << " (" << arg_str << ") was not provided" << termcolor::reset << std::endl;
        }

        m_arg_index++;
        return *this;
    }

    BetterTypeError& BetterTypeError::withArg(std::string_view arg_name, ValueType arg_type)
    {
        return withArg(arg_name, std::vector<ValueType> { arg_type });
    }
}
