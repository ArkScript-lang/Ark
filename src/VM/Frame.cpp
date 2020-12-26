#include <Ark/VM/Frame.hpp>

namespace Ark::internal
{
    Frame::Frame() noexcept :
        m_addr(0), m_page_addr(0), m_new_pp(0),
        // PERF costs a lot
        m_stack(4, Value(ValueType::Undefined)),
        m_i(0), m_scope_to_delete(0)
    {}

    Frame::Frame(uint16_t caller_addr, uint16_t caller_page_addr, uint16_t new_pp) noexcept :
        m_addr(caller_addr), m_page_addr(caller_page_addr), m_new_pp(new_pp),
        // PERF costs a lot
        m_stack(4, Value(ValueType::Undefined)),
        m_i(0), m_scope_to_delete(0)
    {}

    std::ostream& operator<<(std::ostream& os, const Frame& F) noexcept
    {
        os << "Frame";
        return os;
    }
}
