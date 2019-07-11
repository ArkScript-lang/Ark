#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>
#include <vector>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Constants.hpp>
#include <Ark/VM/FFI.hpp>

namespace Ark::internal
{
    /*
        A frame should hold:
        - its own stack
        - a return address to a possible caller (if it's a function's frame)
    */
    class Frame
    {
    public:
        Frame();
        Frame(const Frame&) = default;
        Frame(std::size_t caller_addr, std::size_t caller_page_addr);

        // stack related

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

        // getters-setters (misc)

        inline std::size_t stackSize() const
        {
            return m_i;
        }

        inline std::size_t callerAddr() const
        {
            return m_addr;
        }

        inline std::size_t callerPageAddr() const
        {
            return m_page_addr;
        }

        inline void setClosure(bool value)
        {
            m_is_closure = value;
        }
        
        inline bool isClosure() const
        {
            return m_is_closure;
        }

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);
    
    private:
        //              IP,          PP
        std::size_t m_addr, m_page_addr;

        std::vector<Value> m_stack;
        int8_t m_i;

        bool m_is_closure;
    };
}

#endif