#include <Ark/Builtins/Builtins.hpp>

#include <utility>
#include <algorithm>
#include <fmt/core.h>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::List
{
    /**
     * @name list:reverse
     * @brief Reverse a given list and return a new one
     * @details The original list is not modified
     * @param list the list to reverse
     * =begin
     * (list:reverse [1 2 3])  # [3 2 1]
     * =end
     * @author https://github.com/SuperFola
     */
    Value reverseList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List))
            types::generateError(
                "list:reverse",
                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                n);

        std::reverse(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    /**
     * @name list:find
     * @brief Search an element in a List
     * @details The original list is not modified
     * @param list the List to search in
     * @param value the element to search
     * =begin
     * (list:find [1 2 3] 1)  # 0
     * (list:find [1 2 3] 0)  # -1
     * =end
     * @author https://github.com/SuperFola
     */
    Value findInList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Any))
            types::generateError(
                "list:find",
                { { types::Contract { { types::Typedef("list", ValueType::List), types::Typedef("value", ValueType::Any) } } } },
                n);

        std::vector<Value>& l = n[0].list();
        for (auto it = l.begin(), it_end = l.end(); it != it_end; ++it)
        {
            if (*it == n[1])  // FIXME cast
                return Value(static_cast<int>(std::distance<Value::Iterator>(l.begin(), it)));
        }

        return Value(-1);
    }

    /**
     * @name list:slice
     * @brief Get a slice from a List
     * @details The original list is not modified
     * @param list the list to reverse
     * @param start included, must be positive
     * @param end not included, must be positive and smaller than the list
     * @param step must be greater than 0
     * =begin
     * (list:slice [1 2 3 4 5] 1 4 2)  # [2 4]
     * =end
     * @author https://github.com/SuperFola
     */
    Value sliceList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Number, ValueType::Number, ValueType::Number))
            types::generateError(
                "list:slice",
                { { types::Contract { { types::Typedef("list", ValueType::List),
                                        types::Typedef("start", ValueType::Number),
                                        types::Typedef("end", ValueType::Number),
                                        types::Typedef("step", ValueType::Number) } } } },
                n);

        long step = static_cast<long>(n[3].number());
        if (step <= 0)
            throw std::runtime_error("list:slice: step can not be null or negative");

        auto start = static_cast<long>(n[1].number());
        auto end = static_cast<long>(n[2].number());

        if (start > end)
            throw std::runtime_error(fmt::format("list:slice: start position ({}) must be less or equal to the end position ({})", start, end));
        if (start < 0)
            throw std::runtime_error(fmt::format("list:slice: start index {} can not be less than 0", start));
        if (std::cmp_greater(end, n[0].list().size()))
            throw std::runtime_error(fmt::format("list:slice: end index {} out of range (length: {})", end, n[0].list().size()));

        std::vector<Value> list;
        for (auto i = static_cast<std::size_t>(start); std::cmp_less(i, end); i += static_cast<std::size_t>(step))
            list.push_back(n[0].list()[i]);

        return Value(std::move(list));
    }

    /**
     * @name list:sort
     * @brief Sort a List and return a new one
     * @details The original list is not modified
     * @param list the list to sort
     * =begin
     * (list:sort [4 2 3])  # [1 2 4]
     * =end
     * @author https://github.com/SuperFola
     */
    Value sort_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List))
            types::generateError(
                "list:sort",
                { { types::Contract { { types::Typedef("list", ValueType::List) } } } },
                n);

        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    /**
     * @name list:fill
     * @brief Generate a List of n copies of an element
     * @param count the number of copies
     * @param value the element to copy
     * =begin
     * (list:fill 4 nil)  # [nil nil nil nil]
     * =end
     * @author https://github.com/SuperFola
     */
    Value fill(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number, ValueType::Any))
            types::generateError(
                "list:fill",
                { { types::Contract { { types::Typedef("size", ValueType::Number),
                                        types::Typedef("value", ValueType::Any) } } } },
                n);

        auto c = static_cast<std::size_t>(n[0].number());
        std::vector<Value> l;
        for (std::size_t i = 0; i < c; i++)
            l.push_back(n[1]);

        return Value(std::move(l));
    }

    /**
     * @name list:setAt
     * @brief Modify a given list and return a new one
     * @details The original list is not modified
     * @param list the list to modify
     * @param index the index of the element to modify
     * @param value the new element
     * =begin
     * (list:setAt [1 2 3] 0 5)  # [5 2 3]
     * =end
     * @author https://github.com/SuperFola
     */
    Value setListAt(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Number, ValueType::Any))
            types::generateError(
                "list:setAt",
                { { types::Contract { { types::Typedef("list", ValueType::List),
                                        types::Typedef("index", ValueType::Number),
                                        types::Typedef("value", ValueType::Any) } } } },
                n);

        n[0].list()[static_cast<std::size_t>(n[1].number())] = n[2];
        return n[0];
    }
}
