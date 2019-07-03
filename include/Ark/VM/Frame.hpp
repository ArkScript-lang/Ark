#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>

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

        inline Value pop()
        {
            Value value = std::move(m_stack.back());
            m_stack.pop_back();

            return value;
        }

        inline void push(const Value& value)
        {
            m_stack.push_back(value);
        }

        std::size_t stackSize() const;

        std::size_t callerAddr() const;
        std::size_t callerPageAddr() const;
        std::size_t localsStart() const;

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);
    
    private:
        std::size_t m_addr, m_page_addr, m_locals_start;
        std::vector<Value> m_stack;
    };
}

#endif