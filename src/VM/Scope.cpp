#include <Ark/VM/Scope.hpp>

#include <algorithm>

#define push_pair(id, val) m_data.push_back(std::pair<uint16_t, Value>(id, val))
#define insert_pair(place, id, val) m_data.insert(place, std::pair<uint16_t, Value>(id, val))

namespace Ark::internal
{
    Scope::Scope()
    {}

    void Scope::push_back(uint16_t id, Value&& val)
    {
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
    }

    void Scope::push_back(uint16_t id, const Value& val)
    {
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
    }

    bool Scope::has(uint16_t id)
    {
        return operator[](id) != nullptr;
    }

    Value* Scope::operator[](uint16_t id)
    {
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
    }

    uint16_t Scope::idFromValue(Value&& val)
    {
        for (uint16_t i=0; i < m_data.size(); ++i)
        {
            if (m_data[i].second == val)
                return i;
        }
        return static_cast<uint16_t>(~0);
    }

    const std::size_t Scope::size() const
    {
        return m_data.size();
    }
}