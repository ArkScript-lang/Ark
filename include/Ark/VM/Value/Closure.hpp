/**
 * @file Closure.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Subtype of the value type, handling closures
 * @version 0.1
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

    /**
     * @brief Scope handling
     * 
     * A scope is defined as a shared pointer to a list of local variables
     *  because a Closure could continue to leave when the local variables list
     *  has been closed by the virtual machine
     */
    using Scope_t = std::shared_ptr<Scope>;
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
         * @param scope_ptr the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(Scope_t&& scope_ptr, PageAddr_t pa) noexcept;

        /**
         * @brief Construct a new Closure object
         * 
         * @param scope_ptr the scope of the function turned into a closure
         * @param pa the current page address of the function turned into a closure
         */
        Closure(const Scope_t& scope_ptr, PageAddr_t pa) noexcept;

        /**
         * @brief Return the scope held by the object
         * 
         * @return const Scope_t& 
         */
        const Scope_t& scope() const noexcept;

        /**
         * @brief Return a reference to the scpoe held by the object
         * 
         * @return Scope_t& 
         */
        Scope_t& refScope() noexcept;

        /**
         * @brief Return the page address of the object
         * 
         * @return PageAddr_t 
         */
        inline PageAddr_t pageAddr() const { return m_page_addr; }

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
        Scope_t m_scope;
        // keep track of the code page number, in case we need it later
        PageAddr_t m_page_addr;
    };

    inline bool operator<(const Closure& A, const Closure& B) noexcept
    {
        return A.m_page_addr < B.m_page_addr;
    }
}

#endif
