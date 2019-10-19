#include <Ark/VM/FFI.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::Array
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
            throw Ark::TypeError("First argument of reverse must be a list");

        if (n.size () != 1) // arity error
            throw Ark::TypeError ("reverse takes only 1 argument");

        Value r(std::move(n[0]));
        auto & l = r.list ();
        std::reverse (l.begin (), l.end ());

        return r;
    }

    FFI_Function(findInList)
    {
        if (n.size () != 2) {
            throw std::runtime_error ("findInList takes 2 arguments");
        }

        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of findInList must be a list");
        
        Value r (std::move (n[0]));
        auto & l = r.list ();
        for (Value::Iterator it=l.begin(); it != l.end(); ++it)
        {
            if (*it == n[1]) {
                return trueSym;
            }
        }

        return falseSym;
    }

    FFI_Function(removeAtList)
    {
        if (n.size () != 2)
            throw std::runtime_error ("removeAtList takes 2 arguments");

        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of removeAtList must be a list");
        if (n[1].valueType () != ValueType::Number)
            throw Ark::TypeError("Second argument of removeAtList must be a Number");            

        Value r (std::move (n[0]));
        Value id (std::move (n[1]));
        auto l = r.list ();
        if (id.number () < 0 || id.number () >= l.size ())
            throw std::runtime_error ("List index out of bounds");

        l.erase (l.begin () + id.number ());
        return Value (std::move (l));
    }
}
