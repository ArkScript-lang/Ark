#include <Ark/Builtins/Builtins.hpp>

#include <Ark/String.hpp>
#include <Ark/Utils.hpp>
#include <utf8_decoder/utf8_decoder.h>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::String
{
    Value format(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() == 0)
            throw std::runtime_error(STR_FORMAT_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FORMAT_TE0);

        ::String f(n[0].string().c_str());

        for (Value::Iterator it = n.begin() + 1, it_end = n.end(); it != it_end; ++it)
        {
            if (it->valueType() == ValueType::String)
            {
                ::String& obj = it->stringRef();
                f.format(f.size() + obj.size(), obj.c_str());
            }
            else if (it->valueType() == ValueType::Number)
            {
                double obj = it->number();
                f.format(f.size() + Utils::digPlaces(obj) + Utils::decPlaces(obj) + 1, obj);
            }
            else if (it->valueType() == ValueType::Nil)
                f.format(f.size() + 5, std::string_view("nil"));
            else if (it->valueType() == ValueType::True)
                f.format(f.size() + 5, std::string_view("true"));
            else if (it->valueType() == ValueType::False)
                f.format(f.size() + 5, std::string_view("false"));
            else
            {
                std::stringstream ss; ss << (*it);
                f.format(f.size() + ss.str().size(), std::string_view(ss.str().c_str()));
            }
        }
        n[0].stringRef() = f;
        return n[0];
    }

    Value findSubStr(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 2)
            throw std::runtime_error(STR_FIND_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE0);
        if (n[1].valueType() != ValueType::String)
            throw Ark::TypeError(STR_FIND_TE1);

        return Value(n[0].stringRef().find(n[1].stringRef()));
    }

    Value removeAtStr(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 2)
            throw std::runtime_error(STR_RM_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_RM_TE0);
        if (n[1].valueType() != ValueType::Number)
            throw Ark::TypeError(STR_RM_TE1);

        long id = static_cast<long>(n[1].number());
        if (id < 0 || static_cast<std::size_t>(id) >= n[0].stringRef().size())
            throw std::runtime_error(STR_RM_OOR);

        n[0].stringRef().erase(id, id + 1);
        return n[0];
    }

    Value ord(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(STR_ORD_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(STR_ORD_TE0);

        int ord = utf8codepoint(n[0].stringRef().c_str());

        return Value(ord);
    }

    Value chr(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(STR_CHR_ARITY);
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(STR_CHR_TE0);

        std::array<char, 5> sutf8;

        utf8chr(static_cast<int>(n[0].number()), sutf8.data());
        return Value(std::string(sutf8.data()));
    }
}
