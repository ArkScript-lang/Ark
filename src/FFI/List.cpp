#include <Ark/FFI/FFI.hpp>

#include <iterator>
#include <algorithm>

#include <Ark/FFI/FFIErrors.inl>
#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI::List
{
    FFI_Function(append)
    {
        if (n.size() < 2)
            throw std::runtime_error(LIST_APPEND_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_APPEND_TE0);

        for (Value::Iterator it=n.begin()+1, it_end=n.end(); it != it_end; ++it)
            n[0].push_back(*it);
        return n[0];
    }

    FFI_Function(concat)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_CONCAT_ARITY);

        for (Value::Iterator it=n.begin()+1, it_end=n.end(); it != it_end; ++it)
        {
            if (it->valueType() != ValueType::List)
                throw Ark::TypeError(LIST_CONCAT_TE);

            for (Value::Iterator it2=it->const_list().begin(), it2_end=it->const_list().end(); it2 != it2_end; ++it2)
                n[0].push_back(*it2);
        }
        return n[0];
    }

    FFI_Function(list)
    {
        Value r(ValueType::List);
        for (Value::Iterator it=n.begin(), it_end=n.end(); it != it_end; ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(reverseList)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_REVERSE_ARITY);
        if (n.size() != 1)  // arity error
            throw Ark::TypeError(LIST_REVERSE_TE0);

        std::reverse(n[0].list().begin(), n[0].list().end());

        return n[0];
    }

    FFI_Function(findInList)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FIND_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_FIND_TE0);
        
        std::vector<Value>& l = n[0].list();
        for (Value::Iterator it=l.begin(), it_end=l.end(); it != it_end; ++it)
        {
            if (*it == n[1])
                return Value(static_cast<int>(std::distance<Value::Iterator>(l.begin(), it)));
        }

        return FFI::nil;
    }

    FFI_Function(removeAtList)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_RMAT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_RMAT_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_RMAT_TE1);

        std::size_t idx = static_cast<std::size_t>(n[1].number());
        if (idx < 0 || idx >= n[0].list().size())
            throw std::runtime_error(LIST_RMAT_OOR);

        n[0].list().erase(n[0].list().begin () + idx);
        return n[0];
    }

    FFI_Function(sliceList)
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
        for (std::size_t i=start; i < end; i += step)
            retlist.push_back(n[0].list()[i]);

        Value ret(std::move(retlist));
        return ret;
    }

    FFI_Function(sort_)
    {
        if (n.size() != 1)
            throw std::runtime_error(LIST_SORT_ARITY);
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError(LIST_SORT_TE0);
        
        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }

    FFI_Function(fill)
    {
        if (n.size() != 2)
            throw std::runtime_error(LIST_FILL_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(LIST_FILL_TE0);
        
        std::size_t c = static_cast<std::size_t>(n[0].number());
        std::vector<Value> l;
        for (std::size_t i=0; i < c; i++)
            l.push_back(n[1]);

        return Value(std::move(l));
    }

    FFI_Function(setListAt)
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
