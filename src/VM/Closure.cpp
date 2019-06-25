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

    void Closure::save(std::size_t frame_idx, uint16_t sym)
    {
        m_frame_idx = frame_idx;
        m_symbol = sym;
    }

    const std::shared_ptr<Frame>& Closure::frame() const
    {
        return m_frame;
    }

    PageAddr_t Closure::pageAddr() const
    {
        return m_page_addr;
    }

    std::size_t Closure::frameIndex() const
    {
        return m_frame_idx;
    }

    uint16_t Closure::symbol() const
    {
        return m_symbol;
    }
}