#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Frame::Frame(std::size_t length) :
        m_addr(0),
        m_page_addr(0)
    {
        m_environment.reserve(length);
        for (std::size_t i=0; i < length; ++i)
            m_environment.push_back(FFI::nil);
        
        m_stack.reserve(16);
    }

    Frame::Frame(std::size_t length, std::size_t caller_addr, std::size_t caller_page_addr) :
        m_addr(caller_addr),
        m_page_addr(caller_page_addr)
    {
        m_environment.reserve(length);
        for (std::size_t i=0; i < length; ++i)
            m_environment.push_back(FFI::nil);
        
        m_stack.reserve(16);
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
