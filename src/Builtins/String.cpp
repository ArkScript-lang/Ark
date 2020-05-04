#include <Ark/Builtins/Builtins.hpp>

#include <fmt/format.hpp>
#include <Ark/String.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#define Builtins_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::Builtins::String
{
    Builtins_Function(format)
    {
        if (n.size() == 0)
            throw std::runtime_error(STR_FORMAT_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FORMAT_TE0);

        rj::format f(n[0].string().c_str());

        for (Value::Iterator it=n.begin()+1, it_end=n.end(); it != it_end; ++it)
        {
            if (it->valueType() == ValueType::String)
                f.args(it->string().c_str());
            else if (it->valueType() == ValueType::Number)
                f.args(it->number());
            else
                throw Ark::TypeError(STR_FORMAT_TE1);
        }
        n[0].string_ref() = ::String(std::string(f).c_str());
        return n[0];
    }

    Builtins_Function(findSubStr)
    {
        if (n.size() != 2)
            throw std::runtime_error(STR_FIND_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE0);
        if (n[1].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE1);

        return (n[0].string_ref().find(n[1].string_ref()) != -1) ? trueSym : falseSym;
    }

    Builtins_Function(removeAtStr)
    {
        if (n.size () != 2)
            throw std::runtime_error(STR_RM_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_RM_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(STR_RM_TE1);

        long id = static_cast<long>(n[1].number());
        if (id < 0 || id > n[0].string_ref().size())
            throw std::runtime_error(STR_RM_OOR);

        n[0].string_ref().erase(id, id + 1);
        return n[0];
    }
}
