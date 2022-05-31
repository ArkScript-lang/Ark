/**
 * @file TypeChecker.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief 
 * @version 0.3
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INCLUDE_ARK_TYPECHECKER_HPP
#define INCLUDE_ARK_TYPECHECKER_HPP

#include <limits>
#include <string>
#include <vector>
#include <type_traits>

#define NOMINMAX
#include <Ark/VM/Value.hpp>

#ifdef max
#    undef max
#endif

namespace Ark::types
{
    namespace details
    {
        template <typename T, typename... Ts>
        using AllSame = std::enable_if_t<std::conjunction_v<std::is_same<T, Ts>...>>;

        template <int I>
        bool checkN([[maybe_unused]] const std::vector<Value>& args)
        {
            return true;
        }

        template <int I, typename T, typename... Ts>
        bool checkN(const std::vector<Value>& args, T type, Ts... xs)
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
    bool check(const std::vector<Value>& args, Ts... types)
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

        Typedef(std::string_view name, ValueType type, bool variadic = false) :
            name(name), types { { type } }, variadic(variadic)
        {}

        Typedef(std::string_view name, const std::vector<ValueType>& types, bool variadic = false) :
            name(name), types(types), variadic(variadic)
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
    ARK_API void generateError [[noreturn]] (std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args);
}

#endif
