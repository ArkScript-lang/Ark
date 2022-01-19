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
#define NOMINMAX
#include <Ark/VM/Value.hpp>

#ifdef max
    #undef max
#endif

namespace Ark::internal::types
{
    /**
     * @brief A type definition within a contract
     * 
     */
    struct Typedef
    {
        std::string_view name;
        uint32_t types;
        bool variadic;

        Typedef(std::string_view name, ValueType type, bool variadic = false) :
            name(name), types(1 << static_cast<uint32_t>(type)), variadic(variadic)
        {}

        Typedef(std::string_view name, uint32_t types, bool variadic = false) :
            name(name), types(types), variadic(variadic)
        {}
    };

    /**
     * @brief A contract is a list of typed arguments that a function can follow
     * 
     */
    struct Contract
    {
        std::vector<Typedef> arguments;
    };

    /**
     * @brief Define all the types we can use in contracts
     * 
     */
    constexpr uint32_t AnyType = std::numeric_limits<uint32_t>::max();

    /**
     * @brief Applies type checks and arity check to the provided argument, following a set of contracts the function can follow
     * 
     * @param funcname 
     * @param contracts 
     * @param provided_arguments 
     * @return std::size_t the index of the contract used
     */
    std::size_t checker(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& provided_arguments);

    inline std::size_t checker(std::string_view funcname, const Contract& contract, const std::vector<Value>& provided_arguments)
    {
        return checker(funcname, { { contract } }, provided_arguments);
    }
}

#endif
