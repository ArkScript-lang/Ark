/**
 * @file Frame.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief A frame is created each time a function is called, containing its own stack and the return address
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

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
        Frame() noexcept;

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
         */
        Frame(uint16_t caller_addr, uint16_t caller_page_addr) noexcept;

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
        inline void push(const Value& value) noexcept;

        /**
         * @brief Push a value on the stack of the frame
         * 
         * @param value the value to put on the stack
         */
        inline void push(Value&& value) noexcept;

        // getters-setters (misc)

        /**
         * @brief Get the stack size
         * 
         * @return std::size_t 
         */
        inline std::size_t stackSize() const noexcept;

        /**
         * @brief Get the caller address
         * 
         * @return uint16_t 
         */
        inline uint16_t callerAddr() const noexcept;

        /**
         * @brief Get the caller page address
         * 
         * @return uint16_t 
         */
        inline uint16_t callerPageAddr() const noexcept;

    private:
        uint16_t m_addr;        ///< Instruction pointer
        uint16_t m_page_addr;   ///< Page pointer
        int16_t m_i;

        std::vector<Value> m_stack;
    };

    #include "inline/Frame.inl"
}

#endif