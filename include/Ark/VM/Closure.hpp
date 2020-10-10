#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>
#include <vector>
#include <iostream>

#include <Ark/VM/Types.hpp>

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
        Scope_t& scope_ref() noexcept;

        /**
         * @brief Return the page address of the object
         * 
         * @return PageAddr_t 
         */
        inline PageAddr_t pageAddr() const { return m_page_addr; }

        friend inline bool operator==(const Closure& A, const Closure& B) noexcept;
        friend inline bool operator<(const Closure& A, const Closure& B) noexcept;
        friend std::ostream& operator<<(std::ostream& os, const Closure& C) noexcept;
    
    private:
        Scope_t m_scope;
        // keep track of the code page number, in case we need it later
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B) noexcept
    {
        return A.m_scope == B.m_scope;
    }

    inline bool operator<(const Closure& A, const Closure& B) noexcept
    {
        return A.m_page_addr < B.m_page_addr;
    }
}

#endif