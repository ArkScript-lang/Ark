#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Frame::Frame() :
        m_addr(0), m_page_addr(0), m_new_pp(0),
        m_stack(ARK_MAX_STACK_SIZE), m_i(0),
        m_nil(FFI::nil), m_scope_to_delete(0)
    {}

    Frame::Frame(std::size_t caller_addr, std::size_t caller_page_addr, std::size_t new_pp) :
        m_addr(caller_addr), m_page_addr(caller_page_addr), m_new_pp(new_pp),
        m_stack(ARK_MAX_STACK_SIZE), m_i(0),
        m_nil(FFI::nil), m_scope_to_delete(0)
    {}

    std::ostream& operator<<(std::ostream& os, const Frame& F)
    {
        os << "Frame";
        return os;
    }
}
