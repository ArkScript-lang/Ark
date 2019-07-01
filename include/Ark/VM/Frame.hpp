#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/FFI.hpp>

namespace Ark::internal
{
    /*
        A frame should hold:
        - its own stack
        - its own environment
        - a return address to a possible caller (if it's a function's frame)
    */
    class Frame
    {
    public:
        Frame(std::size_t length=16);
        Frame(const Frame&) = default;
        Frame(std::size_t length, std::size_t caller_addr, std::size_t caller_page_addr);

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

        inline Value& operator[](uint16_t key)
        {
            return m_environment[key];
        }

        inline bool find(uint16_t key) const
        {
            return !(m_environment[key] == FFI::nil);
        }

        std::size_t stackSize() const;

        std::size_t callerAddr() const;
        std::size_t callerPageAddr() const;

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);
    
    private:
        std::size_t m_addr, m_page_addr;
        std::vector<Value> m_stack;
        std::vector<Value> m_environment;
    };
}

#endif