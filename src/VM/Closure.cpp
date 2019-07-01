#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_frame(nullptr),
        m_page_addr(0)
    {}

    Closure::Closure(const std::shared_ptr<Frame>& frame_ptr, PageAddr_t pa) :
        m_frame(frame_ptr),
        m_page_addr(pa)
    {}

    const std::shared_ptr<Frame>& Closure::frame() const
    {
        return m_frame;
    }

    PageAddr_t Closure::pageAddr() const
    {
        return m_page_addr;
    }
}