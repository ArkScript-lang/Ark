/**
 * @file Scope.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The virtual machine scope system
 * @version 0.2
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_VM_SCOPE_HPP
#define ARK_VM_SCOPE_HPP

#include <vector>
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
         * @brief Merge values from this scope as refs in the other scope
         * @details This scope must be kept alive for the ref to be used
         * @param other
         */
        void mergeRefInto(Scope& other);

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
         * @param id_to_look_for
         * @return Value* Returns nullptr if the value can not be found
         */
        Value* operator[](uint16_t id_to_look_for) noexcept;

        /**
         * @brief Get a value from its symbol id
         *
         * @param id_to_look_for
         * @return const Value* Returns nullptr if the value can not be found
         */
        const Value* operator[](uint16_t id_to_look_for) const noexcept;

        /**
         * @brief Get the id of a variable based on its value ; used for debug only
         *
         * @param val
         * @return uint16_t
         */
        [[nodiscard]] uint16_t idFromValue(const Value& val) const noexcept;

        /**
         * @brief Return the size of the scope
         *
         * @return const std::size_t
         */
        [[nodiscard]] std::size_t size() const noexcept;

        friend ARK_API bool operator==(const Scope& A, const Scope& B) noexcept;

        friend class Ark::VM;
        friend class Ark::internal::Closure;

    private:
        std::vector<std::pair<uint16_t, Value>> m_data;
        uint16_t m_min_id;  ///< Minimum stored ID, used for a basic bloom filter
        uint16_t m_max_id;  ///< Maximum stored ID, used for a basic bloom filter
    };
}

#endif
