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

        Closure::~Closure()
        {}

        std::shared_ptr<Frame> Closure::frame() const
        {
            return m_frame;
        }

        PageAddr_t Closure::pageAddr() const
        {
            return m_page_addr;
        }
    }
}