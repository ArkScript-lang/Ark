#include <Ark/VM/FFI.hpp>

#include <iterator>
#include <algorithm>

#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI::List
{
    FFI_Function(append)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of append must be a list");

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
            n[0].push_back(*it);
        return n[0];
    }

    FFI_Function(concat)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of concat should be a list");

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() != ValueType::List)
                throw Ark::TypeError("Arguments of concat must be lists");

            for (Value::Iterator it2=it->const_list().begin(); it2 != it->const_list().end(); ++it2)
                n[0].push_back(*it2);
        }
        return n[0];
    }

    FFI_Function(list)
    {
        Value r(ValueType::List);
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(reverseList)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of reverseList must be a list");
        if (n.size() != 1)  // arity error
            throw Ark::TypeError("reverseList takes only 1 argument");

        std::reverse(n[0].list().begin(), n[0].list().end());

        return n[0];
    }

    FFI_Function(findInList)
    {
        if (n.size() != 2)
            throw std::runtime_error("findInList takes 2 arguments: a list and a value to find in it");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of findInList must be a list");
        
        std::vector<Value>& l = n[0].list();
        for (Value::Iterator it=l.begin(); it != l.end(); ++it)
        {
            if (*it == n[1])
                return Value(static_cast<int>(std::distance<Value::Iterator>(l.begin(), it)));
        }

        return FFI::nil;
    }

    FFI_Function(removeAtList)
    {
        if (n.size() != 2)
            throw std::runtime_error("removeAtList takes 2 arguments: a list and an index");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of removeAtList must be a list");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Second argument of removeAtList must be a Number");

        std::size_t idx = static_cast<std::size_t>(n[1].number());
        if (idx < 0 || idx >= n[0].list().size())
            throw std::runtime_error("List index out of range");

        n[0].list().erase(n[0].list().begin () + idx);
        return n[0];
    }

    FFI_Function(sliceList)
    {
        if (n.size () != 4)
            throw std::runtime_error("sliceList takes 4 arguments: a list, a start position, an end position, and a step");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of sliceList must be a list");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Second argument of sliceList must be a Number");
        if (n[2].valueType() != ValueType::Number)
            throw Ark::TypeError("Third argument of sliceList must be a Number");
        if (n[3].valueType() != ValueType::Number)
            throw Ark::TypeError("Fourth argument of sliceList must be a Number");

        long step = static_cast<long>(n[3].number());
        if (step == 0)
            throw std::runtime_error ("Step can't be 0");

        long start = static_cast<long>(n[1].number());
        long end = static_cast<long>(n[2].number());

        if (start > end)
            throw std::runtime_error("Start position must be less or equal to end position");
        if (start < 0 || end > n[0].list().size())
            throw std::runtime_error("Slice indices out of range");

        std::vector<Value> retlist;
        for (std::size_t i=start; i < end; i += step)
            retlist.push_back(n[0].list()[i]);

        Value ret(std::move(retlist));
        return ret;
    }

    FFI_Function(sort_)
    {
        if (n.size() != 1)
            throw std::runtime_error("sort takes 1 argument: a list");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of sort should be a list");
        
        std::sort(n[0].list().begin(), n[0].list().end());
        return n[0];
    }
}
