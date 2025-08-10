#include <Ark/VM/Value/ClosureScope.hpp>

#include <Ark/VM/ScopeView.hpp>

namespace Ark::internal
{
    void ClosureScope::push_back(const uint16_t id, Value&& val)
    {
        m_data.emplace_back(id, std::move(val));
    }

    void ClosureScope::push_back(const uint16_t id, const Value& val)
    {
        m_data.emplace_back(id, val);
    }

    Value* ClosureScope::operator[](const uint16_t id_to_look_for)
    {
        for (auto& [id, value] : m_data)
        {
            if (id == id_to_look_for)
                return &value;
        }
        return nullptr;
    }

    void ClosureScope::mergeRefInto(ScopeView& other)
    {
        for (auto& [id, value] : m_data)
        {
            if (value.valueType() == ValueType::Reference)
                other.pushBack(id, value);
            else
                other.pushBack(id, Value(&value));
        }
    }

    bool operator==(const ClosureScope& A, const ClosureScope& B) noexcept
    {
        return A.m_data == B.m_data;
    }
}
