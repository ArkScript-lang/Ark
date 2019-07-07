#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>
#include <array>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Constants.hpp>

namespace Ark::internal
{
    /*
        A frame should hold:
        - its own stack
        - a return address to a possible caller (if it's a function's frame)
        - a counter of the var stack to know how many variables
            we should delete when returning
    */
    class Frame
    {
    public:
        Frame();
        Frame(const Frame&) = default;
        Frame(std::size_t caller_addr, std::size_t caller_page_addr, std::size_t locals_start);

        inline Value&& pop()
        {
            m_i--;
            return std::move(m_stack[m_i]);
        }

        inline void push(const Value& value)
        {
            m_stack[m_i] = value;
            m_i++;
        }

        inline void push(Value&& value)
        {
            m_stack[m_i] = std::move(value);
            m_i++;
        }

        std::size_t stackSize();

        std::size_t callerAddr() const;
        std::size_t callerPageAddr() const;
        std::size_t localsStart() const;

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);
    
    private:
        std::size_t m_addr, m_page_addr, m_locals_start;

        std::array<Value, ARK_MAX_STACK_SIZE> m_stack;
        int m_i;
    };
}

#endif