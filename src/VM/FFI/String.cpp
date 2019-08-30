#include <Ark/VM/FFI.hpp>

#include <fmt/format.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    FFI_Function(format)
    {
        if (n.size() == 0)
            throw std::runtime_error("format take at least one argument");
        if (n[0].valueType() != ValueType::String)
            throw std::runtime_error("Argument 1 of format must be of type String");

        rj::format f(n[0].string());

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() == ValueType::String)
                f.args(it->string());
            else if (it->valueType() == ValueType::Number)
                f.args(it->number());
            else
                throw std::runtime_error("Argument of format must be of type String or Number");
        }

        return Value(std::string(f));
    }
}