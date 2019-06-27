#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Frame::Frame(std::size_t length) :
        m_addr(0),
        m_page_addr(0)
    {
        auto nil = Value(NFT::Nil);

        m_environment.reserve(64);
        for (std::size_t i=0; i < length; ++i)
            m_environment.push_back(nil);
    }

    Frame::Frame(std::size_t length, std::size_t caller_addr, std::size_t caller_page_addr) :
        m_addr(caller_addr),
        m_page_addr(caller_page_addr)
    {
        auto nil = Value(NFT::Nil);
        for (std::size_t i=0; i < length; ++i)
            m_environment.push_back(nil);
    }

    Value Frame::pop()
    {
        Value value = std::move(m_stack.back());
        m_stack.pop_back();

        return value;
    }

    void Frame::push(const Value& value)
    {
        m_stack.push_back(value);
    }

    void Frame::setData(std::size_t caller_addr, std::size_t caller_page_addr)
    {
        m_addr = caller_addr;
        m_page_addr = caller_page_addr;
    }

    Value& Frame::operator[](uint16_t key)
    {
        return m_environment[key];
    }

    bool Frame::find(uint16_t key) const
    {
        return !(m_environment[key] == Value(NFT::Nil));
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
        for (std::size_t i=0; i < F.m_environment.size(); ++i)
        {
            os << (3 + i) << " => " << F.m_environment[i];
            if (i != F.m_environment.size() - 1)
                os << ", ";
        }
        return os;
    }
}
