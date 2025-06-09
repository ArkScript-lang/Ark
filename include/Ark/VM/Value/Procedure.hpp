/**
 * @file Procedure.hpp
 * @author Justin Andreas Lacoste (me@justin.cx)
 * @brief Wrapper object user-defined functions
 * @date 2025-06-09
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_VM_PROCEDURE_HPP
#define ARK_VM_PROCEDURE_HPP

#include <functional>
#include <type_traits>
#include <vector>

namespace Ark
{
    class Value;
    class VM;

    /**
     * @brief Storage class to hold custom functions
     */
    class ARK_API Procedure
    {
    public:
        using PointerType = Value (*)(std::vector<Value>&, VM*);
        using CallbackType = std::function<Value(std::vector<Value>&, VM*)>;

        /**
         * @brief Create new Procedure from a plain C-style function pointer taking std::vector<Value>& and VM*.
         *
         * @param functor the function to store
         */
        template <typename T,
                  typename = std::enable_if_t<std::is_pointer_v<T> && std::is_convertible_v<T, PointerType>>>
        Procedure(T functor) noexcept :
            m_procedure(functor) {}

        Procedure(const CallbackType&) noexcept;
        Procedure(CallbackType&&) noexcept;

        Value operator()(std::vector<Value>&, VM*) const;

        bool operator<(const Procedure& other) const noexcept;
        bool operator==(const Procedure& other) const noexcept;

    private:
        CallbackType m_procedure;
    };
}

#endif
