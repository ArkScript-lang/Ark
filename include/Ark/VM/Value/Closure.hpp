/**
 * @file Closure.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value type, handling closures
 * @date 2024-04-21
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef VM_VALUE_CLOSURE_HPP
#define VM_VALUE_CLOSURE_HPP

#include <memory>
#include <string>

#include <Ark/Utils/Platform.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    using PageAddr_t = uint16_t;

    class ClosureScope;

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
         * @param scope the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(const ClosureScope& scope, PageAddr_t pa) noexcept;

        /**
         * @brief Construct a new Closure object
         * @param scope_ptr a shared pointer to the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(const std::shared_ptr<ClosureScope>& scope_ptr, PageAddr_t pa) noexcept;

        [[nodiscard]] const ClosureScope& scope() const noexcept { return *m_scope; }
        [[nodiscard]] ClosureScope& refScope() const noexcept { return *m_scope; }
        [[nodiscard]] const std::shared_ptr<ClosureScope>& scopePtr() const { return m_scope; }

        /**
         *
         * @return PageAddr_t the bytecode page address this closure refers to
         */
        [[nodiscard]] PageAddr_t pageAddr() const { return m_page_addr; }

        /**
         * @brief Used when generating error messages in the VM, to see if a symbol might have been wrongly fully qualified
         *
         * @param end
         * @param vm
         * @return true if the closure has a field which is the end of 'end'
         */
        [[nodiscard]] bool hasFieldEndingWith(const std::string& end, const VM& vm) const;

        /**
         * @brief Print the closure to a string
         *
         * @param vm
         */
        std::string toString(VM& vm) const noexcept;

        friend ARK_API bool operator==(const Closure& A, const Closure& B) noexcept;
        friend ARK_API_INLINE bool operator<(const Closure& A, const Closure& B) noexcept;

    private:
        std::shared_ptr<ClosureScope> m_scope;
        // keep track of the code page number, in case we need it later
        PageAddr_t m_page_addr;
    };

    inline bool operator<(const Closure& A, const Closure& B) noexcept
    {
        return A.m_page_addr < B.m_page_addr;
    }
}

#endif
