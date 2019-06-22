#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>

#include <Ark/VM/Types.hpp>

namespace Ark
{
    namespace internal
    {
        class Frame;

        class Closure
        {
        public:
            Closure();
            Closure(std::shared_ptr<Frame> frame_ptr, PageAddr_t pa);

            void save(std::size_t frame_idx, uint16_t sym);

            std::shared_ptr<Frame> frame() const;
            PageAddr_t pageAddr() const;
            std::size_t frameIndex() const;
            uint16_t symbol() const;

            friend inline bool operator==(const Closure& A, const Closure& B);
        
        private:
            std::shared_ptr<Frame> m_frame;
            PageAddr_t m_page_addr;

            std::size_t m_frame_idx;
            uint16_t m_symbol;
        };

        inline bool operator==(const Closure& A, const Closure& B)
        {
            return false;
        }
    }
}

#endif