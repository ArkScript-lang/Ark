#include <Ark/Builtins/Builtins.hpp>

#include <iterator>
#include <algorithm>

#include <Ark/Builtins/BuiltinsErrors.inl>
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
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_REVERSE_ARITY);
        if (n.size() != 1)  // arity error
            throw TypeError(LIST_REVERSE_TE0);

        std::reverse(n[0].list().begin(), n[0].list().end());

        return n[0];
    }

    /**
     * @name list:find
     * @brief Search an element in a List
     * @details The original list is not modified
     * @param list the List to search in
     * @param el the element to search
     * =begin
     * (list:find [1 2 3] 1)  # 0
     * (list:find [1 2 3] 0)  # -1
     * =end
     * @author https://github.com/SuperFola
     */
    Value findInList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FIND_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_FIND_TE0);

        std::vector<Value>& l = n[0].list();
        for (Value::Iterator it = l.begin(), it_end = l.end(); it != it_end; ++it)
        {
            if (*it == n[1])
                return Value(static_cast<int>(std::distance<Value::Iterator>(l.begin(), it)));
        }

        return Value(-1);
    }

    /**
     * @name list:removeAt
     * @brief Remove an element in a List and return a new one
     * @details The original list is not modified
     * @param list the list to remove an element from
     * @param index the index of the element to remove (can be negative to search from the end)
     * =begin
     * (list:removeAt [1 2 3] 0)  # [2 3]
     * (list:removeAt [1 2 3] -1)  # [1 2]
     * =end
     * @author https://github.com/SuperFola
     */
    Value removeAtList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        static bool has_warned = false;
        if (!has_warned)
        {
            std::cout << "list:removeAt will be deprecated in ArkScript 4.0.0, consider using pop! or pop\n";
            has_warned = true;
        }

        if (n.size() != 2)
            throw std::runtime_error(LIST_RMAT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_RMAT_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw TypeError(LIST_RMAT_TE1);

        std::size_t idx = static_cast<std::size_t>(n[1].number());
        if (idx >= n[0].list().size())
            throw std::runtime_error(LIST_RMAT_OOR);

        n[0].list().erase(n[0].list().begin() + idx);
        return n[0];
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
     * (list:reverse [1 2 3])  # [3 2 1]
     * =end
     * @author https://github.com/SuperFola
     */
    Value sliceList(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 4)
            throw std::runtime_error(LIST_SLICE_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_SLICE_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw TypeError(LIST_SLICE_TE1);
        if (n[2].valueType() != ValueType::Number)
            throw TypeError(LIST_SLICE_TE2);
        if (n[3].valueType() != ValueType::Number)
            throw TypeError(LIST_SLICE_TE3);

        long step = static_cast<long>(n[3].number());
        if (step <= 0)
            throw std::runtime_error(LIST_SLICE_STEP);

        long start = static_cast<long>(n[1].number());
        long end = static_cast<long>(n[2].number());

        if (start > end)
            throw std::runtime_error(LIST_SLICE_ORDER);
        if (start < 0 || static_cast<std::size_t>(end) > n[0].list().size())
            throw std::runtime_error(LIST_SLICE_OOR);

        std::vector<Value> retlist;
        for (long i = start; i < end; i += step)
            retlist.push_back(n[0].list()[i]);

        return Value(std::move(retlist));
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
        if (n.size() != 1)
            throw std::runtime_error(LIST_SORT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_SORT_TE0);

        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    /**
     * @name list:fill
     * @brief Generate a List of n copies of an element
     * @param count the number of copies
     * @param el the element to copy
     * =begin
     * (list:fill 4 nil)  # [nil nil nil nil]
     * =end
     * @author https://github.com/SuperFola
     */
    Value fill(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FILL_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw TypeError(LIST_FILL_TE0);

        std::size_t c = static_cast<std::size_t>(n[0].number());
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
     * @param el the new element
     * =begin
     * (list:setAt [1 2 3] 0 5)  # [5 2 3]
     * =end
     * @author https://github.com/SuperFola
     */
    Value setListAt(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 3)
            throw std::runtime_error(LIST_SETAT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw TypeError(LIST_SETAT_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw TypeError(LIST_SETAT_TE1);

        n[0].list()[static_cast<std::size_t>(n[1].number())] = n[2];
        return n[0];
    }
}
