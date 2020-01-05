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
        Value packed;

        buffer.seekg(0);
        if(type == ValueType::Number)
        {
            auto src {std::get<double>(value_src)};
            msgpack::pack(buffer, src);
            packed = Value(buffer.str());
        }
        else if(type == ValueType::String)
        {
            auto src {std::get<std::string>(value_src)};
            msgpack::pack(buffer, src);
            packed = Value(buffer.str());
        }
        else
        {
            auto src {std::get<std::vector<Value>>(value_src)};
            packed = list_packing(src);
        }
        return packed;
    }
    Value unpack(const std::vector<Value>& args)
    {
        //unpack(Value packed_str_buffer) and return an object unpacked 
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 2 arguments");
        if(args[0].valueType() != ValueType::String && args[0].valueType() != ValueType::List)
            throw Ark::TypeError("The packed buffer must be a string or a list");
        //if(args[1].valueType() != ValueType::Number)
        //    throw Ark::TypeError("The type index must be a number");        
        Value dst;
        double ark_number;
        std::string ark_string;
        msgpack::object deserialized;
        auto type_test = [&](void) {
            std::string packed {static_cast<Value>(args[0]).string_ref()};
            deserialized = msgpack::unpack(packed.data(), packed.size()).get();
            try
            {
                deserialized.convert(ark_number);
                dst = Value(ark_number);
            }
            catch(const std::bad_cast &e)
            {
                try
                {
                    deserialized.convert(ark_string);
                    dst = Value(ark_string);
                }
                catch(const std::exception &e) {}
            }
        };

        if(args[0].valueType() == ValueType::List)
        {
            auto packed {static_cast<Value>(args[0]).list()};
            dst = list_unpacking(packed);
        }
        else
        {
            type_test();
        }
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