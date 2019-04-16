#ifndef ark_vm_frame
#define ark_vm_frame

#include <string>
#include <unordered_map>

#include <Ark/VM/Value.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>

namespace Ark
{
    namespace VM
    {
        using namespace Ark::Compiler;

        /*
            A frame should hold:
            - its own stack
            - its own environment
            - a return address to a possible caller (if it's a function's frame)
        */
        class Frame
        {
        public:
            Frame();
            Frame(std::size_t caller_addr, std::size_t caller_page_addr);
            ~Frame();

            Value pop();
            void push(const Value& value);

            Value& operator[](const std::string& key);

            std::size_t callerAddr() const;
            std::size_t callerPageAddr() const;
        
        private:
            std::size_t m_addr, m_page_addr;
            std::vector<Value> m_stack;
            std::unordered_map<std::string, Value> m_environment;
        };
    }
}

#endif