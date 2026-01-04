/**
 * @file Procedure.hpp
 * @author Justin Andreas Lacoste (me@justin.cx)
 * @brief Wrapper object for user-defined functions
 * @date 2025-06-09
 *
 * @copyright Copyright (c) 2025-2026
 *
 */

#ifndef ARK_VM_PROCEDURE_HPP
#define ARK_VM_PROCEDURE_HPP

#include <functional>
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

        ///
        /// Due to clang (sometimes) rejecting forward declared types
        /// in templates (`Value` causes issues), we have to implement
        /// the constructor for the actual `CallbackType` using SFINAE
        /// and a templated constructor, such that when clang
        /// encounters the constructor, it knows the actual
        /// declaration of `Value`.
        ///
        /**
         * @brief Create a new procedure.
         */
        template <typename T>
        // cppcheck-suppress noExplicitConstructor ; we explicitly want implicit conversion to Procedure
        Procedure(T&& cb) :
            m_procedure(cb)
        {}

        /**
         * @brief Create a new procedure from a stateless C function pointer.
         */
        Procedure(PointerType c_ptr);  // cppcheck-suppress noExplicitConstructor ; we explicitly want implicit conversion to Procedure

        Value operator()(std::vector<Value>&, VM*) const;

        bool operator<(const Procedure& other) const noexcept;
        bool operator==(const Procedure& other) const noexcept;

        friend struct std::hash<Ark::Procedure>;

    private:
        CallbackType m_procedure;
    };
}

template <>
struct std::hash<Ark::Procedure>
{
    [[nodiscard]] std::size_t operator()(const Ark::Procedure& s) const noexcept
    {
        return std::hash<const void*> {}(static_cast<const void*>(&s.m_procedure));
    }
};

#endif
