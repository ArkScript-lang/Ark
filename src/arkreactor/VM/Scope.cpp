#include <Ark/VM/Scope.hpp>

#include <limits>

namespace Ark::internal
{
    Scope::Scope() noexcept
    {}

    void Scope::push_back(uint16_t id, Value&& val) noexcept
    {
        m_data.emplace_back(std::pair<uint16_t, Value>(std::move(id), std::move(val)));
    }

    void Scope::push_back(uint16_t id, const Value& val) noexcept
    {
        m_data.emplace_back(std::pair<uint16_t, Value>(id, val));
    }

    bool Scope::has(uint16_t id) noexcept
    {
        return operator[](id) != nullptr;
    }

    Value* Scope::operator[](uint16_t id) noexcept
    {
        for (std::size_t i = 0, end = m_data.size(); i < end; ++i)
        {
            if (m_data[i].first == id)
                return &m_data[i].second;
        }
        return nullptr;
    }

    uint16_t Scope::idFromValue(const Value& val) const noexcept
    {
        for (std::size_t i = 0, end = m_data.size(); i < end; ++i)
        {
            if (m_data[i].second == val)
                return m_data[i].first;
        }
        return std::numeric_limits<uint16_t>::max();
    }

    std::size_t Scope::size() const noexcept
    {
        return m_data.size();
    }
}
