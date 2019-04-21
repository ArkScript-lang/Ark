#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>

#include <Ark/VM/Types.hpp>

namespace Ark
{
    namespace VM
    {
        class Frame;

        class Closure
        {
        public:
            Closure();
            Closure(Frame* frame_ptr, PageAddr_t pa);
            ~Closure();

            std::shared_ptr<Frame> frame() const;
            PageAddr_t pageAddr() const;
        
        private:
            std::shared_ptr<Frame> m_frame;
            PageAddr_t m_page_addr;
        };
    }
}

#endif