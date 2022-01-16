/**
 * @file TypeChecker.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INCLUDE_ARK_TYPECHECKER_HPP
#define INCLUDE_ARK_TYPECHECKER_HPP

#include <string>
#include <vector>
#include <Ark/VM/Value.hpp>

namespace Ark::internal::types
{
    /**
     * @brief A contract is a list of typed arguments that a function can follow
     * 
     */
    struct Contract
    {
        /**
         * @brief A type definition within a contract
         * 
         */
        struct Typedef
        {
            std::string_view name;
            std::vector<ValueType> types;
            bool variadic;

            Typedef(std::string_view name, ValueType type, bool variadic = false) :
                name(name), variadic(variadic)
            {
                types.emplace_back(type);
            }

            Typedef(std::string_view name, const std::vector<ValueType>& types, bool variadic = false) :
                name(name), types(types), variadic(variadic)
            {}
        };

        std::vector<Typedef> arguments;
    };

    /**
     * @brief Define all the types we can use in contracts
     * 
     */
    const std::vector<ValueType> AnyType {{
        ValueType::List,
        ValueType::Number,
        ValueType::String,
        ValueType::PageAddr,
        ValueType::CProc,
        ValueType::Closure,
        ValueType::User,
        ValueType::Nil,
        ValueType::True,
        ValueType::False
    }};

    /**
     * @brief Applies type checks and arity check to the provided argument, following a set of contracts the function can follow
     * 
     * @param funcname 
     * @param contracts 
     * @param provided_arguments 
     * @return std::size_t the index of the contract used
     */
    std::size_t typeChecker(std::string_view funcname, const std::vector<Contract>& contracts, const std::vector<Value>& provided_arguments);
}

#endif
