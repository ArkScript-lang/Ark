/**
 * @file TypeChecker.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief
 * @date 2022-01-16
 *
 * @copyright Copyright (c) 2022-2026
 *
 */

#ifndef INCLUDE_ARK_TYPECHECKER_HPP
#define INCLUDE_ARK_TYPECHECKER_HPP

#include <string>
#include <vector>
#include <ostream>
#include <sstream>

#include <Ark/Error/Exceptions.hpp>
#include <Ark/VM/Value/Value.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::types
{
    namespace details
    {
        template <typename T, typename... Ts>
        using AllSame = std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>;

        template <int I>
        [[nodiscard]] bool checkN(const std::vector<Value>& args)
        {
            return I >= args.size();
        }

        template <int I, typename T, typename... Ts>
        [[nodiscard]] bool checkN(const std::vector<Value>& args, T type, Ts... xs)
        {
            if (I >= args.size() || (type != ValueType::Any && args[I].valueType() != type))
                return false;
            return checkN<I + 1>(args, xs...);
        }
    }

    /**
     * @brief Helper to see if a builtin has been given a wanted set of types
     *
     * @tparam Ts Variadic argument list composed of ValueTypes
     * @param args arguments passed to the function
     * @param types accepted types
     * @return true if the contract is respected
     * @return false otherwise
     */
    template <typename... Ts, typename = details::AllSame<ValueType, Ts...>>
    [[nodiscard]] bool check(const std::vector<Value>& args, Ts... types)
    {
        if (sizeof...(types) != args.size())
            return false;
        return details::checkN<0>(args, types...);
    }

    /**
     * @brief A type definition within a contract
     *
     */
    struct ARK_API Typedef
    {
        std::string name;
        std::vector<ValueType> types;
        bool variadic;

        Typedef(const std::string& type_name, const ValueType type, const bool is_variadic = false) :
            name(type_name), types { type }, variadic(is_variadic)
        {}

        Typedef(const std::string& type_name, const std::vector<ValueType>& type_list, const bool is_variadic = false) :
            name(type_name), types(type_list), variadic(is_variadic)
        {}
    };

    /**
     * @brief A contract is a list of typed arguments that a function can follow
     *
     */
    struct ARK_API Contract
    {
        std::vector<Typedef> arguments;
    };

    /**
     * @brief Generate an error message based on a given set of types contracts provided argument list
     *
     * @param funcname ArkScript name of the function
     * @param contracts types contracts the function can follow
     * @param args provided argument list
     * @param vm reference to the VM used for pretty printing closures
     * @param os output stream, default to cerr
     * @param colorize enable output colorizing
     */
    ARK_API void generateError(
        const std::string_view& funcname,
        const std::vector<Contract>& contracts,
        const std::vector<Value>& args,
        VM& vm,
        std::ostream& os = std::cerr,
        bool colorize = true);

    class ARK_API TypeCheckingError final : public Error
    {
    public:
        TypeCheckingError(std::string&& funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args) :
            Error("TypeCheckingError"),
            m_funcname(std::move(funcname)),
            m_contracts(contracts),
            m_passed_args(args)
        {}

        [[nodiscard]] std::string details(const bool colorize, VM& vm) const override
        {
            std::stringstream stream;
            generateError(m_funcname, m_contracts, m_passed_args, vm, stream, colorize);
            return stream.str();
        }

    private:
        std::string m_funcname;
        std::vector<Contract> m_contracts;
        std::vector<Value> m_passed_args;
    };
}

#endif
