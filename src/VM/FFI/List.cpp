#include <Ark/VM/FFI.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::List
{
    FFI_Function(append)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of append must be a list");

        Value r(std::move(n[0]));
        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(concat)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of concat should be a list");

        Value r(std::move(n[0]));
        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() != ValueType::List)
                throw Ark::TypeError("Arguments of concat must be lists");

            for (Value::Iterator it2=it->const_list().begin(); it2 != it->const_list().end(); ++it2)
                r.push_back(*it2);
        }
        return r;
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

        Value r(std::move(n[0]));
        auto& l = r.list();
        std::reverse(l.begin(), l.end());

        return r;
    }

    FFI_Function(findInList)
    {
        if (n.size() != 2)
            throw std::runtime_error("findInList takes 2 arguments: a list and a value to find in it");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of findInList must be a list");
        
        Value r(std::move(n[0]));
        std::vector<Value>& l = r.list();
        for (Value::Iterator it=l.begin(); it != l.end(); ++it)
        {
            if (*it == n[1])
                return trueSym;
        }

        return falseSym;
    }

    FFI_Function(removeAtList)
    {
        if (n.size() != 2)
            throw std::runtime_error("removeAtList takes 2 arguments: a list and an index");
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of removeAtList must be a list");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Second argument of removeAtList must be a Number");

        Value r(std::move(n[0]));
        std::size_t idx = static_cast<std::size_t>(n[1].number());
        std::vector<Value>& l = r.list();
        if (idx < 0 || idx >= l.size())
            throw std::runtime_error("List index out of range");

        l.erase(l.begin () + idx);
        return r;
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

        Value r(std::move(n[0]));
        long start = static_cast<long>(n[1].number());
        long end = static_cast<long>(n[2].number());

        auto l = r.list();
        if (start > end)
            throw std::runtime_error("Start position must be less or equal to end position");

        if (start < 0 || end > l.size())
            throw std::runtime_error("Slice indices out of range");

        std::vector<Value> retlist;
        for (std::size_t i=start; i < end; i += step)
            retlist.push_back(l[i]);

        Value ret(std::move(retlist));
        return ret;
    }
}
