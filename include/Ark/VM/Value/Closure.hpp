/**
 * @file Closure.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value type, handling closures
 * @version 0.2
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef VM_VALUE_CLOSURE_HPP
#define VM_VALUE_CLOSURE_HPP

#include <memory>
#include <vector>
#include <iostream>

#include <Ark/Platform.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    class Scope;

    using PageAddr_t = uint16_t;

    /**
     * @brief Closure management
     *
     */
    class Closure
    {
    public:
        /**
         * @brief Construct a new Closure object
         *
         */
        Closure() noexcept;

        /**
         * @brief Construct a new Closure object
         *
         * @param scope the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(Scope&& scope, PageAddr_t pa) noexcept;

        /**
         * @brief Construct a new Closure object
         *
         * @param scope the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(const Scope& scope, PageAddr_t pa) noexcept;

        /**
         * @brief Construct a new Closure object
         * @param scope_ptr a shared pointer to the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(const std::shared_ptr<Scope>& scope_ptr, PageAddr_t pa) noexcept;

        /**
         * @brief Return the scope held by the object
         *
         * @return const Scope&
         */
        [[nodiscard]] inline const Scope& scope() const noexcept { return *m_scope.get(); }

        /**
         * @brief Return a reference to the scope held by the object
         *
         * @return Scope&
         */
        [[nodiscard]] inline Scope& refScope() noexcept { return *m_scope.get(); }

        /**
         * @brief Return a reference to the shared pointer representing the scope
         * @details The scope has to be kept alive somewhere or all its variables will be destroyed.
         *
         * @return const std::shared_ptr<Scope>&
         */
        [[nodiscard]] inline const std::shared_ptr<Scope>& scopePtr() const { return m_scope; }

        /**
         * @brief Return the page address of the object
         *
         * @return PageAddr_t
         */
        [[nodiscard]] inline PageAddr_t pageAddr() const { return m_page_addr; }

        /**
         * @brief Print the closure to a string
         *
         * @param os
         * @param vm
         */
        void toString(std::ostream& os, VM& vm) const noexcept;

        friend ARK_API bool operator==(const Closure& A, const Closure& B) noexcept;
        friend ARK_API_INLINE bool operator<(const Closure& A, const Closure& B) noexcept;

    private:
        std::shared_ptr<Scope> m_scope;
        // keep track of the code page number, in case we need it later
        PageAddr_t m_page_addr;
    };

    inline bool operator<(const Closure& A, const Closure& B) noexcept
    {
        return A.m_page_addr < B.m_page_addr;
    }
}

#endif
