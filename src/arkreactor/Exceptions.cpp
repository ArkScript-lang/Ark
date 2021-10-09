#include <termcolor/termcolor.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    BetterTypeError::BetterTypeError(std::string_view func_name, std::size_t expected_argc, const std::vector<Value>& args) :
        Error(func_name, expected_argc, args)
    {
        std::cout << func_name << ": needs " << expected_argc << " argument(s), got " << args.size() << std::endl;
    }

    BetterTypeError& BetterTypeError::withArg(std::string_view arg_name, ValueType arg_type)
    {
        std::size_t expected_type = static_cast<std::size_t>(arg_type);

        // argument has been provided
        if (m_arg_index < m_args.size())
        {
            std::size_t provided_type = static_cast<std::size_t>(m_args[m_arg_index].valueType());
            if (provided_type != expected_type)
            {
                std::cout << termcolor::yellow;
            }

            std::cout << "  -> " << arg_name << " (" << types_to_str[expected_type] << ") was of type " << types_to_str[provided_type] << termcolor::reset << std::endl;
        }
        // argument was not provided
        else
        {
            std::cout << termcolor::yellow;
            std::cout << "  -> " << arg_name << " (" << types_to_str[expected_type] << ") was not provided" << termcolor::reset << std::endl;
        }

        m_arg_index++;
        return *this;
    }
}
