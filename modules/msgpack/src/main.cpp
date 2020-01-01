#include <iostream> 
#include <sstream>
#include <msgpack.hpp>
#include <msgpack_module.hpp>

MAKE_ENTRY_POINT()

namespace ArkMsgpack
{
    Value pack(const std::vector<Value>& args)
    {
        // pack(Value src) and return an strin buffer
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 1 arguments");
        ValueType type {args[0].valueType()};
        std::stringstream buffer;
        ObjekType value_src {get_objekt(args[0], type)};
        std::string dst;

        buffer.seekg(0);
        if(type == ValueType::Number)
        {
            auto src = std::get<double>(value_src);
            msgpack::pack(buffer, src);
            dst = buffer.str();
        }
        else if(type == ValueType::String)
        {
            auto src = std::get<std::string>(value_src);
            msgpack::pack(buffer, src);
            dst = buffer.str();
        }
        return Value(dst);
    }
}

ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;

    map["msgPack"] = ArkMsgpack::pack;
    return map;
}