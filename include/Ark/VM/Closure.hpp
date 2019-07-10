#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    class Frame;

    class Closure
    {
    public:
        Closure();
        Closure(const std::shared_ptr<Frame>& frame_ptr, PageAddr_t pa);

        const std::shared_ptr<Frame>& frame() const;

        inline PageAddr_t pageAddr() const
        {
            return m_page_addr;
        }

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        std::shared_ptr<Frame> m_frame;
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_frame.get() == B.m_frame.get();
    }
}

#endif