#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>
#include <vector>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark::internal
{
    /**
     * @brief Frame management
     * 
     * A frame should hold:
     *  - its own stack
     *  - a return address to a possible caller (if it's a function's frame)
     */
    class Frame
    {
    public:
        /**
         * @brief Construct a new Frame object
         * 
         */
        Frame();

        /**
         * @brief Construct a new Frame object from another one
         * 
         */
        Frame(const Frame&) = default;

        /**
         * @brief Construct a new Frame object
         * 
         * @param caller_addr the address of the caller, to return to it afterward
         * @param caller_page_addr the page address of the caller
         * @param new_pp the new page address where we're going
         */
        Frame(uint16_t caller_addr, uint16_t caller_page_addr, uint16_t new_pp);

        // stack related

        /**
         * @brief Pop a value from the stack of the frame
         * 
         * @return Value* the value popped
         */
        inline Value* pop();

        /**
         * @brief Push a value on the stack of the frame
         * 
         * @param value the value to put on the stack
         */
        inline void push(const Value& value);

        /**
         * @brief Push a value on the stack of the frame
         * 
         * @param value the value to put on the stack
         */
        inline void push(Value&& value);

        // getters-setters (misc)

        /**
         * @brief Get the stack size
         * 
         * @return std::size_t 
         */
        inline std::size_t stackSize() const;

        /**
         * @brief Get the caller address
         * 
         * @return uint16_t 
         */
        inline uint16_t callerAddr() const;

        /**
         * @brief Get the caller page address
         * 
         * @return uint16_t 
         */
        inline uint16_t callerPageAddr() const;

        /**
         * @brief Get the current page address
         * 
         * @return uint16_t 
         */
        inline uint16_t currentPageAddr() const;

        // related to scope deletion

        /**
         * @brief Increment the number of scopes linked to this frame
         * 
         * Needed for the primitive garbage collecting system.
         * 
         */
        inline void incScopeCountToDelete();

        /**
         * @brief Reset the number of scopes linked to this frame
         * 
         * Needed for the primitive garbage collecting system.
         * 
         */
        inline void resetScopeCountToDelete();

        /**
         * @brief Get the number of scopes linked to this frame
         * 
         * Needed for the primitive garbage collecting system.
         * 
         * @return uint8_t 
         */
        inline uint8_t scopeCountToDelete() const;

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);

    private:
        //              IP,          PP    EXC_PP
        uint16_t m_addr, m_page_addr, m_new_pp;

        std::vector<Value> m_stack;
        int16_t m_i;

        uint8_t m_scope_to_delete;
    };

    #include "inline/Frame.inl"
}

#endif