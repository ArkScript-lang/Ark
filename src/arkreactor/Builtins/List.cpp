#include <Ark/Builtins/Builtins.hpp>

#include <utility>
#include <algorithm>
#include <fmt/core.h>

#include <Ark/TypeChecker.hpp>

namespace Ark::internal::Builtins::List
{
    Value reverseList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List))
            throw types::TypeCheckingError(
                "list:reverse",
                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                n);

        std::ranges::reverse(n[0].list());
        return n[0];
    }

    Value findInList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Any))
            throw types::TypeCheckingError(
                "list:find",
                { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("value", ValueType::Any) } } } },
                n);

        if (const auto it = std::ranges::find(n[0].list(), n[1]); it != n[0].list().end())
            return Value(static_cast<int>(std::distance<decltype(n[0].list().begin())>(n[0].list().begin(), it)));
        return Value(-1);
    }

    Value sort_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List))
            throw types::TypeCheckingError(
                "list:sort",
                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                n);

        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    Value fill(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number, ValueType::Any))
            throw types::TypeCheckingError(
                "list:fill",
                { { types::Contract { { types::Typedef("size", ValueType::Number),
                                        types::Typedef("value", ValueType::Any) } } } },
                n);

        const auto c = static_cast<std::size_t>(n[0].number());
        std::vector<Value> l;
        l.reserve(c);
        for (std::size_t i = 0; i < c; i++)
            l.push_back(n[1]);

        return Value(std::move(l));
    }

    Value setListAt(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Number, ValueType::Any))
            throw types::TypeCheckingError(
                "list:setAt",
                { { types::Contract { { types::Typedef("list", ValueType::List),
                                        types::Typedef("index", ValueType::Number),
                                        types::Typedef("value", ValueType::Any) } } } },
                n);

        auto& list = n[0].list();

        const std::size_t size = list.size();
        long idx = static_cast<long>(n[1].number());
        idx = idx < 0 ? static_cast<long>(size) + idx : idx;
        if (std::cmp_greater_equal(idx, size))
            throw std::runtime_error(
                fmt::format("IndexError: list:setAt index ({}) out of range (list size: {})", idx, size));

        list[static_cast<std::size_t>(idx)] = n[2];
        return n[0];
    }
}
