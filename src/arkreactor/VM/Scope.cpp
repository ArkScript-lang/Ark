#include <Ark/VM/Scope.hpp>

#include <limits>

namespace Ark::internal
{
    Scope::Scope() noexcept :
        m_min_id(std::numeric_limits<uint16_t>::max()), m_max_id(0)
    {
        m_data.reserve(3);
    }

    void Scope::mergeRefInto(Ark::internal::Scope& other)
    {
        for (auto& [id, val] : m_data)
        {
            if (val.valueType() == ValueType::Reference)
                other.push_back(id, val);
            else
                other.push_back(id, Value(&val));
        }
    }

    void Scope::push_back(uint16_t id, Value&& val) noexcept
    {
        if (id < m_min_id)
            m_min_id = id;
        if (id > m_max_id)
            m_max_id = id;

        m_data.emplace_back(id, std::move(val));
    }

    void Scope::push_back(uint16_t id, const Value& val) noexcept
    {
        if (id < m_min_id)
            m_min_id = id;
        if (id > m_max_id)
            m_max_id = id;

        m_data.emplace_back(id, val);
    }

    bool Scope::has(uint16_t id) noexcept
    {
        return m_min_id <= id && id <= m_max_id && operator[](id) != nullptr;
    }

    Value* Scope::operator[](uint16_t id) noexcept
    {
        for (auto& val : m_data)
        {
            if (val.first == id)
                return &val.second;
        }
        return nullptr;
    }

    const Value* Scope::operator[](uint16_t id) const noexcept
    {
        for (const auto& val : m_data)
        {
            if (val.first == id)
                return &val.second;
        }
        return nullptr;
    }

    uint16_t Scope::idFromValue(const Value& val) const noexcept
    {
        for (const auto& [id, data] : m_data)
        {
            if (data == val)
                return id;
        }
        return std::numeric_limits<uint16_t>::max();
    }

    std::size_t Scope::size() const noexcept
    {
        return m_data.size();
    }

    bool operator==(const Scope& A, const Scope& B) noexcept
    {
        const std::size_t size = A.size();
        if (size != B.size())
            return false;
        if (A.m_min_id != B.m_min_id || A.m_max_id != B.m_max_id)
            return false;

        // assuming we have the same closure page address, the element should be in the same order
        for (std::size_t i = 0; i < size; ++i)
        {
            if (A.m_data[i] != B.m_data[i])
                return false;
        }

        return true;
    }
}
