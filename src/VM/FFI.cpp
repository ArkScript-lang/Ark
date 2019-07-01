#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <Ark/Log.hpp>
#undef abs
#include <cmath>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym = Value(NFT::False);
    extern const Value trueSym  = Value(NFT::True);
    extern const Value nil      = Value(NFT::Nil);

    extern const std::vector<std::pair<std::string, Value>> builtins = {
        { "false",  falseSym },
        { "true",   trueSym },
        { "nil",    nil },
        { "append", Value(&append) },
        { "concat", Value(&concat) },
        { "list",   Value(&list) },
        { "print",  Value(&print) },
        { "input",  Value(&input) }
    };

    extern const std::vector<std::string> operators = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstof", "tailof", "headof",
        "nil?", "assert",
        "toNumber", "toString",
        "@", "and", "or", "mod",
    };

    // ------------------------------

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

    FFI_Function(print)
    {
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            std::cout << (*it) << " ";
        std::cout << std::endl;

        return nil;
    }

    FFI_Function(input)
    {
        if (n.size() == 1)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError("Argument of input must be of type String");
            std::cout << n[0].string();
        }

        std::string line = "";
        std::getline(std::cin, line);

        return Value(line);
    }
}
