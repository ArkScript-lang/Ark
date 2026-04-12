#include <Ark/VM/ScopeView.hpp>

#include <cassert>

namespace Ark::internal
{
    void ScopeView::insertFront(const std::vector<pair_t>& values) noexcept
    {
        const std::size_t offset_by = values.size();
        // If there is one day a bug with bad references, this can be caused by this code,
        // called when inserting plugins variables in a scope (because we invalidate said
        // references by moving them to another slot inside m_storage).
        for (std::size_t i = 0; i < m_size; ++i)
        {
            // This is a weak attempt to prevent / notice the bug before it goes in production,
            // if you hit this assertion read the comments carefully!
            assert(m_storage[m_start + m_size - i - 1].second.valueType() != ValueType::Reference && "References can not be moved around!");
            m_storage[m_start + m_size - i + offset_by - 1] = m_storage[m_start + m_size - i - 1];
        }

        std::size_t i = 0;
        for (const pair_t& pair : values)
        {
            const uint16_t id = pair.first;
            if (id < m_min_id)
                m_min_id = id;
            if (id > m_max_id)
                m_max_id = id;

            m_storage[m_start + i] = pair;
            ++i;
        }

        m_size += offset_by;
    }

    uint16_t ScopeView::idFromValue(const Value& val) const noexcept
    {
        for (std::size_t i = m_start; i < m_start + m_size; ++i)
        {
            const auto& [id, value] = m_storage[i];
            if (value == val)
                return id;
        }
        return MaxValue16Bits;
    }

    bool operator==(const ScopeView& A, const ScopeView& B) noexcept
    {
        // if we have two scopes with the same number of elements and starting at the same position,
        // they must be identical, as we have a single storage for all scopes
        return A.m_size == B.m_size && A.m_start == B.m_start;
    }
}
