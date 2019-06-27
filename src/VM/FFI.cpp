#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <Ark/Log.hpp>
#undef abs
#include <cmath>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const std::vector<std::string> builtins = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstof", "tailof", "append", "concat", "list", "nil?",
        "print", "assert", "input",
        "toNumber", "toString",
        "@", "and", "or", "headof",
        "mod"
    };

    const Value falseSym = Value(NFT::False);
    const Value trueSym  = Value(NFT::True);
    const Value nil      = Value(NFT::Nil);

    // ------------------------------

    FFI_Function(add)
    {
        if (n[0].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of + should be Numbers");
        if (n[1].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of + should be Numbers");
        
        auto i = n[0].number();
        return Value(n[0].number() + n[1].number());
    }

    FFI_Function(sub)
    {
        if (n[0].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of - should be Numbers");
        if (n[1].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of - should be Numbers");
        
        return Value(n[0].number() - n[1].number());
    }

    FFI_Function(mul)
    {
        if (n[0].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of * should be Numbers");
        if (n[1].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of * should be Numbers");
        
        return Value(n[0].number() * n[1].number());
    }

    FFI_Function(div)
    {
        if (n[0].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of / should be Numbers");
        if (n[1].valueType() != valueType::Number)
            throw Ark::TypeError("Arguments of / should be Numbers");
        
        auto d = n[1].number();
        if (d == 0)
            throw Ark::ZeroDivisionError();
        
        return Value(n[0].number() / d);
    }
    
    // ------------------------------

    FFI_Function(gt)
    {
        if (n[0].valueType() == ValueType::String)
        {
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Arguments of > should have the same type");
            
            return (n[0].string() > n[1].string()) ? trueSym : falseSym;
        }
        else if (n[0].valueType() == ValueType::Number)
        {
            if (n[1].valueType() != ValueType::Number)
                throw Ark::TypeError("Arguments of > should have the same type");
            
            return (n[0].number() > n[1].number()) ? trueSym : falseSym;
        }
        throw Ark::TypeError("Arguments of > should either be Strings or Numbers");
    }

    FFI_Function(lt)
    {
        if (n[0].valueType() == ValueType::String)
        {
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Arguments of < should have the same type");
            
            return (n[0].string() < n[1].string()) ? trueSym : falseSym;
        }
        else if (n[0].valueType() == ValueType::Number)
        {
            if (n[1].valueType() != ValueType::Number)
                throw Ark::TypeError("Arguments of < should have the same type");
            
            return (n[0].number() < n[1].number()) ? trueSym : falseSym;
        }
        throw Ark::TypeError("Arguments of < should either be Strings or Numbers");
    }

    FFI_Function(le)
    {
        if (n[0].valueType() == ValueType::String)
        {
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Arguments of <= should have the same type");
            
            return (n[0].string() <= n[1].string()) ? trueSym : falseSym;
        }
        else if (n[0].valueType() == ValueType::Number)
        {
            if (n[1].valueType() != ValueType::Number)
                throw Ark::TypeError("Arguments of <= should have the same type");
            
            return (n[0].number() <= n[1].number()) ? trueSym : falseSym;
        }
        throw Ark::TypeError("Arguments of <= should either be Strings or Numbers");
    }

    FFI_Function(ge)
    {
        if (n[0].valueType() == ValueType::String)
        {
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Arguments of >= should have the same type");
            
            return (n[0].string() >= n[1].string()) ? trueSym : falseSym;
        }
        else if (n[0].valueType() == ValueType::Number)
        {
            if (n[1].valueType() != ValueType::Number)
                throw Ark::TypeError("Arguments of >= should have the same type");
            
            return (n[0].number() >= n[1].number()) ? trueSym : falseSym;
        }
        throw Ark::TypeError("Arguments of >= should either be Strings or Numbers");
    }

    FFI_Function(neq)
    {
        return (!(n[0] == n[1])) ? trueSym : falseSym;
    }

    FFI_Function(eq)
    {
        return (n[0] == n[1]) ? trueSym : falseSym;
    }
    
    // ------------------------------
    
    FFI_Function(len)
    {
        if (n[0].valueType() == ValueType::List)
            return Value(static_cast<int>(n[0].const_list().size()));
        if (n[0].valueType() == ValueType::String)
            return Value(static_cast<int>(FFI_string(n[0]).size()));

        throw Ark::TypeError("Argument of len must be a list or a String");
    }
    
    FFI_Function(empty)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("Argument of empty must be a list");
        
        return (n[0].const_list().size() == 0) ? trueSym : falseSym;
    }
    
    FFI_Function(firstof)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("Argument of firstof must be a list");
        
        return n[0].const_list()[0];
    }
    
    FFI_Function(tailof)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("Argument of tailof must be a list");
        
        if (n[0].const_list().size() < 2)
            return nil;
        
        Value r = n[0];
        r.list().erase(r.const_list().begin());
        return r;
    }

    FFI_Function(append)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of append must be a list");
        
        Value r = n[0];
        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(concat)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of concat should be a list");
        
        Value r = n[0];
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
        Value r(/* is_list */ true);
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(isnil)
    {
        return n[0] == nil ? trueSym : falseSym;
    }
    
    // ------------------------------

    FFI_Function(print)
    {
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            std::cout << (*it) << " ";
        std::cout << std::endl;

        return nil;
    }

    FFI_Function(assert_)
    {
        if (n[0] == falseSym)
        {
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Second argument of assert must be a String");

            throw Ark::AssertionFailed(n[1].string());
        }
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

    // ------------------------------

    FFI_Function(toNumber)
    {
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument of toNumber must be a String");
        
        return Value(std::stod(n[0].string().c_str()));
    }

    FFI_Function(toString)
    {
        std::stringstream ss;
        ss << n[0];
        return Value(ss.str());
    }

    FFI_Function(at)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("Argument 1 of @ should be a List");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument 2 of @ should be a Number");
        
        return n[0].const_list()[static_cast<long>(n[1].number())];
    }

    FFI_Function(and_)
    {
        return n[0] == trueSym && n[1] == trueSym;
    }

    FFI_Function(or_)
    {
        return n[0] == trueSym || n[1] == trueSym;
    }

    FFI_Function(headof)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("Argument of headof must be a list");
        
        if (n[0].const_list().size() < 2)
            return nil;
        
        Value r = n[0];
        r.list().erase(r.const_list().end());
        return r;
    }

    FFI_Function(mod)
    {
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Arguments of mod should be Numbers");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Arguments of mod should be Numbers");
        
        return Value(std::fmod(n[0].number(), n[1].number()));
    }
}
