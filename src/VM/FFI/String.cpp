#include <Ark/VM/FFI.hpp>

#include <fmt/format.hpp>

#include <Ark/VM/FFIErrors.inl>
#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI::String
{
    FFI_Function(format)
    {
        if (n.size() == 0)
            throw std::runtime_error(STR_FORMAT_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FORMAT_TE0);

        rj::format f(n[0].string());

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() == ValueType::String)
                f.args(it->string());
            else if (it->valueType() == ValueType::Number)
                f.args(it->number());
            else
                throw Ark::TypeError(STR_FORMAT_TE1);
        }
        n[0].string_ref() = std::string(f);
        return n[0];
    }

    FFI_Function(findSubStr)
    {
        if (n.size() != 2)
            throw std::runtime_error(STR_FIND_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE0);
        if (n[1].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE1);
        
        return (n[0].string().find(n[1].string()) != std::string::npos) ? trueSym : falseSym;
    }

    FFI_Function(removeAtStr)
    {
        if (n.size () != 2)
            throw std::runtime_error(STR_RM_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_RM_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(STR_RM_TE1);

        long id = static_cast<long>(n[1].number());
        if (id < 0 || id > n[0].string().size())
            throw std::runtime_error(STR_RM_OOR);

        n[0].string_ref().erase(n[0].string_ref().begin() + id);
        return n[0];
    }
}
