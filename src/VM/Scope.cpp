#include <Ark/VM/Scope.hpp>

#ifdef ARK_SCOPE_DICHOTOMY
    #include <algorithm>
#endif

#define push_pair(id, val) m_data.emplace_back(std::pair<uint16_t, Value>(id, val))
#define insert_pair(place, id, val) m_data.insert(place, std::pair<uint16_t, Value>(id, val))

namespace Ark::internal
{
    Scope::Scope() noexcept
    {
    #ifndef ARK_SCOPE_DICHOTOMY
        // PERF costs a lot
        m_data.reserve(4);
    #endif
    }

    void Scope::push_back(uint16_t id, Value&& val) noexcept
    {
    #ifdef ARK_SCOPE_DICHOTOMY
        switch (m_data.size())
        {
            case 0:
                push_pair(std::move(id), std::move(val));
                break;

            case 1:
                if (m_data[0].first < id)
                    push_pair(std::move(id), std::move(val));
                else
                    insert_pair(m_data.begin(), std::move(id), std::move(val));
                break;

            default:
                auto lower = std::lower_bound(m_data.begin(), m_data.end(), id, [](const auto& lhs, uint16_t id) -> bool {
                    return lhs.first < id;
                });
                insert_pair(lower, std::move(id), std::move(val));
                break;
        }
    #else
        // PERF faster on ackermann, compared to the dichotomic version
        push_pair(std::move(id), std::move(val));
    #endif
    }

    void Scope::push_back(uint16_t id, const Value& val) noexcept
    {
    #ifdef ARK_SCOPE_DICHOTOMY
        switch (m_data.size())
        {
            case 0:
                push_pair(id, val);
                break;

            case 1:
                if (m_data[0].first < id)
                    push_pair(id, val);
                else
                    insert_pair(m_data.begin(), id, val);
                break;

            default:
                auto lower = std::lower_bound(m_data.begin(), m_data.end(), id, [](const auto& lhs, uint16_t id) -> bool {
                    return lhs.first < id;
                });
                insert_pair(lower, id, val);
                break;
        }
    #else
        push_pair(id, val);
    #endif
    }

    bool Scope::has(uint16_t id) noexcept
    {
        return operator[](id) != nullptr;
    }

    Value* Scope::operator[](uint16_t id) noexcept
    {
    #ifdef ARK_SCOPE_DICHOTOMY
        switch (m_data.size())
        {
            case 0:
                return nullptr;

            case 1:
                if (m_data[0].first == id)
                    return &m_data[0].second;
                return nullptr;

            default:
                auto lower = std::lower_bound(m_data.begin(), m_data.end(), id, [](const auto& lhs, uint16_t id) -> bool {
                    return lhs.first < id;
                });
                if (lower != m_data.end() && lower->first == id)
                    return &lower->second;
                return nullptr;
        }
    #else
        for (std::size_t i = 0, end = m_data.size(); i < end; ++i)
        {
            if (m_data[i].first == id)
                return &m_data[i].second;
        }
        return nullptr;
    #endif
    }

    uint16_t Scope::idFromValue(Value&& val) noexcept
    {
        for (std::size_t i = 0, end = m_data.size(); i < end; ++i)
        {
            if (m_data[i].second == val)
                return m_data[i].first;
        }
        return static_cast<uint16_t>(~0);
    }

    const std::size_t Scope::size() const noexcept
    {
        return m_data.size();
    }
}