/**
 * @file ClosureScope.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value type, handling closures
 * @date 2025-03-17
 *
 * @copyright Copyright (c) 2025-2026
 *
 */

#ifndef ARK_VM_VALUE_CLOSURESCOPE_HPP
#define ARK_VM_VALUE_CLOSURESCOPE_HPP

#include <vector>
#include <utility>
#include <cinttypes>

#include <Ark/Utils/Platform.hpp>
#include <Ark/VM/Value.hpp>

namespace Ark::internal
{
    class ScopeView;

    /**
     * @brief A class to store fields captured by a closure
     */
    class ARK_API ClosureScope
    {
    public:
        /**
         * @brief Create a new ClosureScope
         */
        ClosureScope() noexcept = default;

        /**
         * @brief Put a value in the scope
         *
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        void push_back(uint16_t id, Value&& val);

        /**
         * @brief Put a value in the scope
         *
         * @param id The symbol id of the variable
         * @param val The value linked to the symbol
         */
        void push_back(uint16_t id, const Value& val);

        Value* operator[](uint16_t id_to_look_for);

        /**
         * @brief Merge values from this scope as refs in the other scope
         * @details This scope must be kept alive for the ref to be used
         * @param other
         */
        void mergeRefInto(ScopeView& other);

        friend class Closure;

        friend ARK_API bool operator==(const ClosureScope& A, const ClosureScope& B) noexcept;

    private:
        std::vector<std::pair<uint16_t, Value>> m_data;
    };
}

#endif  // ARK_VM_VALUE_CLOSURESCOPE_HPP
