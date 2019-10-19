#include <Ark/VM/FFI.hpp>

#include <fmt/format.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::String
{
    FFI_Function(format)
    {
        if (n.size() == 0)
            throw std::runtime_error("format take at least one argument");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument 1 of format must be of type String");

        rj::format f(n[0].string());

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() == ValueType::String)
                f.args(it->string());
            else if (it->valueType() == ValueType::Number)
                f.args(it->number());
            else
                throw Ark::TypeError("Argument of format must be of type String or Number");
        }
        return Value(std::string(f));
    }

    FFI_Function(findSubStr)
    {
        if (n.size() != 2)
            throw std::runtime_error("findSubStr take exactly 2 arguments: a string and the substring to search for");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument 1 of findSubStr must be of type String");
        if (n[1].valueType() != ValueType::String)
            throw Ark::TypeError("Argument 2 of findSubStr must be of type String");
        
        return (n[0].string().find(n[1].string()) != std::string::npos) ? trueSym : falseSym;
    }

    FFI_Function (removeAtStr)
    {
        if (n.size () != 2)
            throw std::runtime_error("removeAtStr take exactly 2 arguments: a string and an index");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument 1 of removeAtStr must be of type String");
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument 2 of removeAtStr must be of type Number");

        std::string str = n[0].string();
        long id = static_cast<long>(n[1].number());
        if (id < 0 || id > str.size())
            throw std::runtime_error("String index out of range");

        str.erase(str.begin() + id);
        return Value(str);
    }
}
