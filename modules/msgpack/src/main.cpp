#include <msgpack_module.hpp>

MAKE_ENTRY_POINT()

namespace ArkMsgpack
{
    Value pack(const std::vector<Value>& args)
    {
        // pack(Value src) and return a buffer's string packed 
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 1 arguments");
        ValueType type {args[0].valueType()};
        std::stringstream buffer;
        CObjekt value_src {get_cobjekt(args[0], type)};
        std::string packed;

        buffer.seekg(0);
        if(type == ValueType::Number)
        {
            auto src {std::get<double>(value_src)};
            msgpack::pack(buffer, src);
            packed = buffer.str();
        }
        else if(type == ValueType::String)
        {
            auto src {std::get<std::string>(value_src)};
            msgpack::pack(buffer, src);
            packed = buffer.str();
        }
        else
        {
            auto src {std::get<std::vector<Value>>(value_src)};
            list_packing(src, buffer);
            packed = buffer.str();
        }
        return Value(packed);
    }
    Value unpack(const std::vector<Value>& args)
    {
        //unpack(Value packed_str_buffer) and return an object unpacked 
        if(args.size() != 2)
            throw std::runtime_error("ArgError : This function must have 2 arguments");
        if(args[0].valueType() != ValueType::String)
            throw Ark::TypeError("The packed buffer must be a string");
        if(args[1].valueType() != ValueType::Number)
            throw Ark::TypeError("The type index must be a number");
        
        std::string packed {static_cast<Value>(args[0]).string_ref()};
        Value dst;
        msgpack::object deserialized = msgpack::unpack(packed.data(), packed.size()).get();

        if(args[1] == 0)
        {
            double ark_number;
            deserialized.convert(ark_number);
            dst = Value(ark_number);
        }
        else if(args[1] == 1)
        {
            std::string ark_string;
            deserialized.convert(ark_string);
            dst = Value(ark_string);
        }
        else if()
        return dst;
    }
}

ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;

    map["msgPack"] = ArkMsgpack::pack;
    map["msgUnpack"] = ArkMsgpack::unpack;
    return map;
}