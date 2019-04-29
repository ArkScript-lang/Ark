#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Frame.hpp>

namespace Ark
{
    namespace VM
    {
        Closure::Closure() :
            m_frame(nullptr),
            m_page_addr(0)
        {}

        Closure::Closure(Frame* frame_ptr, PageAddr_t pa) :
            m_page_addr(pa)
        {
            // copy frame
            m_frame = std::make_shared<Frame>(*frame_ptr);
        }

        void Closure::save(std::size_t frame_idx, const std::string& sym)
        {
            m_frame_idx = frame_idx;
            m_symbol = sym;
        }

        std::shared_ptr<Frame> Closure::frame() const
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

        const std::string& Closure::symbol() const
        {
            return m_symbol;
        }
    }
}