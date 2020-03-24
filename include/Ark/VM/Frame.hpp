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

        inline Value* pop();
        inline void push(const Value& value);
        inline void push(Value&& value);

        // getters-setters (misc)

        inline std::size_t stackSize() const;
        inline std::size_t callerAddr() const;
        inline std::size_t callerPageAddr() const;
        inline std::size_t currentPageAddr() const;

        // related to scope deletion

        inline void incScopeCountToDelete();
        inline void resetScopeCountToDelete();
        inline uint8_t scopeCountToDelete() const;

        friend std::ostream& operator<<(std::ostream& os, const Frame& F);

    private:
        //              IP,          PP    EXC_PP
        std::size_t m_addr, m_page_addr, m_new_pp;

        std::vector<Value> m_stack;
        int16_t m_i;

        uint8_t m_scope_to_delete;
    };

    #include "inline/Frame.inl"
}

#endif