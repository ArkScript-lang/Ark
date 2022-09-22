#include <Ark/VM/Scope.hpp>

#include <limits>

namespace Ark::internal
{
    Scope::Scope() noexcept
    {
        m_data.reserve(3);
    }

    void Scope::push_back(uint16_t id, Value&& val) noexcept
    {
        m_data.emplace_back(std::move(id), std::move(val));
    }

    void Scope::push_back(uint16_t id, const Value& val) noexcept
    {
        m_data.emplace_back(id, val);
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

    const Value* Scope::operator[](uint16_t id) const noexcept
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

    bool operator==(const Scope& A, const Scope& B) noexcept
    {
        const std::size_t size = A.size();
        if (size != B.size())
            return false;

        for (std::size_t i = 0; i < size; ++i)
        {
            const Value* b_value = B[A.m_data[i].first];

            if (b_value == nullptr || *b_value != A.m_data[i].second)
                return false;
        }

        return true;
    }
}
