/**
 * @file Closure.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value type, handling closures
 * @version 1.1
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef VM_VALUE_CLOSURE_HPP
#define VM_VALUE_CLOSURE_HPP

#include <memory>
#include <iostream>
#include <string>

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

        [[nodiscard]] const Scope& scope() const noexcept { return *m_scope; }
        [[nodiscard]] Scope& refScope() const noexcept { return *m_scope; }
        [[nodiscard]] const std::shared_ptr<Scope>& scopePtr() const { return m_scope; }

        [[nodiscard]] PageAddr_t pageAddr() const { return m_page_addr; }

        /**
         * @brief Print the closure to a string
         *
         * @param vm
         */
        std::string toString(VM& vm) const noexcept;

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
