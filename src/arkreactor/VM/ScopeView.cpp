#include <Ark/VM/ScopeView.hpp>

#include <limits>

namespace Ark::internal
{
    ScopeView::ScopeView(pair_t* storage, const std::size_t start) noexcept :
        m_storage(storage), m_start(start), m_size(0), m_min_id(std::numeric_limits<uint16_t>::max()), m_max_id(0)
    {}

    bool ScopeView::push_back(uint16_t id, Value&& val) noexcept
    {
        if (m_start + m_size >= ScopeStackSize) [[unlikely]]
            return false;

        if (id < m_min_id)
            m_min_id = id;
        if (id > m_max_id)
            m_max_id = id;

        m_storage[m_start + m_size] = std::make_pair(id, std::move(val));
        ++m_size;
        return true;
    }

    bool ScopeView::push_back(uint16_t id, const Value& val) noexcept
    {
        if (m_start + m_size >= ScopeStackSize) [[unlikely]]
            return false;

        if (id < m_min_id)
            m_min_id = id;
        if (id > m_max_id)
            m_max_id = id;

        m_storage[m_start + m_size] = std::make_pair(id, val);
        ++m_size;
        return true;
    }

    bool ScopeView::maybeHas(const uint16_t id) const noexcept
    {
        return m_min_id <= id && id <= m_max_id;
    }

    Value* ScopeView::operator[](const uint16_t id_to_look_for) noexcept
    {
        if (!maybeHas(id_to_look_for))
            return nullptr;

        for (std::size_t i = m_start; i < m_start + m_size; ++i)
        {
            auto& [id, value] = m_storage[i];
            if (id == id_to_look_for)
                return &value;
        }
        return nullptr;
    }

    const Value* ScopeView::operator[](const uint16_t id_to_look_for) const noexcept
    {
        if (!maybeHas(id_to_look_for))
            return nullptr;

        for (std::size_t i = m_start; i < m_start + m_size; ++i)
        {
            auto& [id, value] = m_storage[i];
            if (id == id_to_look_for)
                return &value;
        }
        return nullptr;
    }

    uint16_t ScopeView::idFromValue(const Value& val) const noexcept
    {
        for (std::size_t i = m_start; i < m_start + m_size; ++i)
        {
            const auto& [id, value] = m_storage[i];
            if (value == val)
                return id;
        }
        return std::numeric_limits<uint16_t>::max();
    }

    void ScopeView::reset() noexcept
    {
        m_size = 0;
        m_min_id = std::numeric_limits<uint16_t>::max();
        m_max_id = 0;
    }

    bool operator==(const ScopeView& A, const ScopeView& B) noexcept
    {
        // if we have two scopes with the same number of elements and starting at the same position,
        // they must be identical, as we have a single storage for all scopes
        return A.m_size == B.m_size && A.m_start == B.m_start;
    }
}
