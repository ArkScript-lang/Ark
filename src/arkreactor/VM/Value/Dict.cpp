#include <Ark/VM/Value/Dict.hpp>

#include <Ark/VM/DefaultValues.hpp>

#include <ranges>

namespace Ark::internal
{
    void Dict::set(const Value& key, const Value& value)
    {
        m_dict.insert_or_assign(key, value);
    }

    const Value& Dict::get(const Value& key)
    {
        if (const auto it = m_dict.find(key); it != m_dict.end())
            return it->second;
        return Nil;
    }

    bool Dict::contains(const Value& key) const
    {
        return m_dict.contains(key);
    }

    void Dict::remove(const Value& key)
    {
        m_dict.erase(key);
    }

    Value::List_t Dict::keys()
    {
        Value::List_t keys;
        keys.reserve(m_dict.size());
        std::ranges::copy(std::ranges::views::keys(m_dict), std::back_inserter(keys));

        return keys;
    }

    std::size_t Dict::size() const
    {
        return m_dict.size();
    }

    std::string Dict::toString(VM& vm) const
    {
        std::string out = "{";

        std::size_t i = 0;
        for (const auto& [key, value] : m_dict)
        {
            out += key.toString(vm) + ": " + value.toString(vm);

            if (i + 1 != m_dict.size())
                out += ", ";
            ++i;
        }

        return out + "}";
    }
}
