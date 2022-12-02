/**
 * @file Scope.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The virtual machine scope system
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef ARK_VM_SCOPE_HPP
#define ARK_VM_SCOPE_HPP

#include <vector>
#include <utility>
#include <cinttypes>

#include <Ark/Platform.hpp>
#include <Ark/VM/Value.hpp>

namespace Ark::internal
{
    /**
     * @brief A class to handle the VM scope more efficiently
     * 
     */
    class Scope
    {
    public:
        /**
         * @brief Construct a new Scope object
         * 
         */
        Scope() noexcept;

        /**
         * @brief Put a value in the scope
         * 
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        void push_back(uint16_t id, Value&& val) noexcept;

        /**
         * @brief Put a value in the scope
         * 
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        void push_back(uint16_t id, const Value& val) noexcept;

        /**
         * @brief Check if the scope has a specific symbol in memory
         * 
         * @param id The id of the symbol
         * @return true On success
         * @return false Otherwise
         */
        bool has(uint16_t id) noexcept;

        /**
         * @brief Get a value from its symbol id
         * 
         * @param id 
         * @return Value* Returns nullptr if the value can not be found
         */
        Value* operator[](uint16_t id) noexcept;

        /**
         * @brief Get a value from its symbol id
         * 
         * @param id 
         * @return const Value* Returns nullptr if the value can not be found
         */
        const Value* operator[](uint16_t id) const noexcept;

        /**
         * @brief Get the id of a variable based on its value ; used for debug only
         * 
         * @param val 
         * @return uint16_t 
         */
        uint16_t idFromValue(const Value& val) const noexcept;

        /**
         * @brief Return the size of the scope
         * 
         * @return const std::size_t 
         */
        std::size_t size() const noexcept;

        friend ARK_API bool operator==(const Scope& A, const Scope& B) noexcept;

        friend class Ark::VM;
        friend class Ark::internal::Closure;

    private:
        std::vector<std::pair<uint16_t, Value>> m_data;
        uint16_t m_min_id;
        uint16_t m_max_id;
    };
}

#endif
