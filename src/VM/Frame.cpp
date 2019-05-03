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

        void Frame::copyEnvironmentTo(Frame& other)
        {
            for (auto kv : m_environment)
                other.m_environment[kv.first] = kv.second;
        }

        Value Frame::pop()
        {
            Value value = m_stack.back();
            m_stack.pop_back();

            return value;
        }

        void Frame::push(const Value& value)
        {
            m_stack.push_back(value);
        }

        Value& Frame::operator[](uint16_t key)
        {
            return m_environment[key];
        }

        bool Frame::find(uint16_t key) const
        {
            return m_environment.find(key) != m_environment.end();
        }

        std::size_t Frame::stackSize() const
        {
            return m_stack.size();
        }

        std::size_t Frame::callerAddr() const
        {
            return m_addr;
        }

        std::size_t Frame::callerPageAddr() const
        {
            return m_page_addr;
        }

        std::ostream& operator<<(std::ostream& os, const Frame& F)
        {
            std::size_t i = 0;
            for (auto kv : F.m_environment)
            {
                os << static_cast<int>(kv.first) << " => " << kv.second;
                if (i != F.m_environment.size() - 1)
                    os << ", ";
            }
            return os;
        }
    }
}