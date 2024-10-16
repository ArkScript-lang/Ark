/**
 * @file TypeChecker.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief
 * @version 1.1
 * @date 2022-01-16
 *
 * @copyright Copyright (c) 2022-2024
 *
 */

#ifndef INCLUDE_ARK_TYPECHECKER_HPP
#define INCLUDE_ARK_TYPECHECKER_HPP

#include <string>
#include <vector>

#include <Ark/VM/Value.hpp>

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
        std::string_view name;
        std::vector<ValueType> types;
        bool variadic;

        Typedef(const std::string_view& type_name, const ValueType type, const bool is_variadic = false) :
            name(type_name), types { type }, variadic(is_variadic)
        {}

        Typedef(const std::string_view& type_name, const std::vector<ValueType>& type_list, const bool is_variadic = false) :
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
     */
    ARK_API void generateError [[noreturn]] (const std::string_view& funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args);
}

#endif
