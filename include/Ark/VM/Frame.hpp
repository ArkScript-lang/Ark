#ifndef ark_vm_frame
#define ark_vm_frame

#include <iostream>
#include <cinttypes>
#include <vector>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Constants.hpp>
#include <Ark/FFI/FFI.hpp>

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
        Frame(std::size_t caller_addr, std::size_t caller_page_addr, std::size_t new_pp);

        // stack related

        inline Value* pop()
        {
            m_i--;
            return &m_stack[m_i];
        }

        inline void push(const Value& value)
        {
            m_stack[m_i] = value;
            m_i++;

            if (m_i == m_stack.size())
                m_stack.emplace_back(FFI::undefined);
        }

        inline void push(Value&& value)
        {
            m_stack[m_i] = std::move(value);
            m_i++;

            if (m_i == m_stack.size())
                m_stack.emplace_back(FFI::undefined);
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

        inline std::size_t currentPageAddr() const
        {
            return m_new_pp;
        }

        // related to scope deletion

        inline void incScopeCountToDelete()
        {
            m_scope_to_delete++;
        }

        inline void resetScopeCountToDelete()
        {
            m_scope_to_delete = 0;
        }
        
        inline uint8_t scopeCountToDelete() const
        {
            return m_scope_to_delete;
        }

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);
    
    private:
        //              IP,          PP    EXC_PP
        std::size_t m_addr, m_page_addr, m_new_pp;

        std::vector<Value> m_stack;
        int16_t m_i;

        uint8_t m_scope_to_delete;
    };
}

#endif