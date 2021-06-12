#include <Ark/Builtins/Builtins.hpp>

#include <iterator>
#include <algorithm>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::List
{
    Value reverseList(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_REVERSE_ARITY);
        if (n.size() != 1)  // arity error
            throw Ark::TypeError(LIST_REVERSE_TE0);

        std::reverse(n[0].list().begin(), n[0].list().end());

        return n[0];
    }

    Value findInList(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FIND_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_FIND_TE0);

        std::vector<Value>& l = n[0].list();
        for (Value::Iterator it = l.begin(), it_end = l.end(); it != it_end; ++it)
        {
            if (*it == n[1])
                return Value(static_cast<int>(std::distance<Value::Iterator>(l.begin(), it)));
        }

        return Value(-1);
    }

    Value removeAtList(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_RMAT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_RMAT_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_RMAT_TE1);

        std::size_t idx = static_cast<std::size_t>(n[1].number());
        if (idx >= n[0].list().size())
            throw std::runtime_error(LIST_RMAT_OOR);

        n[0].list().erase(n[0].list().begin () + idx);
        return n[0];
    }

    Value sliceList(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size () != 4)
            throw std::runtime_error(LIST_SLICE_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_SLICE_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_SLICE_TE1);
        if (n[2].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_SLICE_TE2);
        if (n[3].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_SLICE_TE3);

        long step = static_cast<long>(n[3].number());
        if (step == 0)
            throw std::runtime_error(LIST_SLICE_STEP);

        long start = static_cast<long>(n[1].number());
        long end = static_cast<long>(n[2].number());

        if (start > end)
            throw std::runtime_error(LIST_SLICE_ORDER);
        if (start < 0 || end > n[0].list().size())
            throw std::runtime_error(LIST_SLICE_OOR);

        std::vector<Value> retlist;
        for (std::size_t i = start; i < end; i += step)
            retlist.push_back(n[0].list()[i]);

        return Value(std::move(retlist));
    }

    Value sort_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(LIST_SORT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_SORT_TE0);

        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    Value fill(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FILL_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_FILL_TE0);
        
        std::size_t c = static_cast<std::size_t>(n[0].number());
        std::vector<Value> l;
        for (std::size_t i = 0; i < c; i++)
            l.push_back(n[1]);

        return Value(std::move(l));
    }

    Value setListAt(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 3)
            throw std::runtime_error(LIST_SETAT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_SETAT_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_SETAT_TE1);

        n[0].list()[static_cast<std::size_t>(n[1].number())] = n[2];
        return n[0];
    }
}
