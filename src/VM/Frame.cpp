#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Frame::Frame() :
        m_addr(0),
        m_page_addr(0),
        m_locals_start(0)
    {
        m_stack.reserve(16);
    }

    Frame::Frame(std::size_t caller_addr, std::size_t caller_page_addr, std::size_t locals_start) :
        m_addr(caller_addr),
        m_page_addr(caller_page_addr),
        m_locals_start(locals_start)
    {
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

    std::size_t Frame::localsStart() const
    {
        return m_locals_start;
    }

    std::ostream& operator<<(std::ostream& os, const Frame& F)
    {
        os << "Frame";
        return os;
    }
}
