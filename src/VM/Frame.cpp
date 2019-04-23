#include <Ark/VM/Frame.hpp>

namespace Ark
{
    namespace VM
    {
        Frame::Frame() :
            m_addr(0),
            m_page_addr(0)
        {}

        Frame::Frame(std::size_t caller_addr, std::size_t caller_page_addr) :
            m_addr(caller_addr),
            m_page_addr(caller_page_addr)
        {}

        Frame::~Frame()
        {}

        Value Frame::pop()
        {
            Value value(m_stack.back());
            m_stack.pop_back();

            return value;
        }

        void Frame::push(const Value& value)
        {
            m_stack.push_back(value);
        }

        Value& Frame::operator[](const std::string& key)
        {
            return m_env[key];
        }

        std::size_t Frame::returnAddr() const
        {
            return m_addr;
        }

        std::size_t Frame::callerPageAddr() const
        {
            return m_page_addr;
        }
    }
}