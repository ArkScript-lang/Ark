#include <msgpack_module.hpp>

namespace ArkMsgpack
{
    Value pack(const std::vector<Value> &args)
    {
        // pack(Value src) and return a buffer's string packed 
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 1 arguments");
        ValueType type {args[0].valueType()};
        std::stringstream buffer;
        CObject value_src {get_cobject(args[0], type)};
        Value packed;
        
        buffer.seekg(0);
        if(type == ValueType::NFT)
        {
            bool src {std::get<bool>(value_src)};
            msgpack::pack(buffer, src);
            packed = Value(buffer.str());
        }
        else if(type == ValueType::Number)
        {
            double src {std::get<double>(value_src)};
            msgpack::pack(buffer, src);
            packed = Value(buffer.str());
        }
        else if(type == ValueType::String)
        {
            std::string src {std::get<std::string>(value_src)};
            msgpack::pack(buffer, src);
            packed = Value(buffer.str());
        }
        else
        {
            std::vector<Value> src {std::get<std::vector<Value>>(value_src)};
            packed = list_packing(src);
        }

        return packed;
    }

    Value unpack(const std::vector<Value> &args)
    {
        //unpack(Value packed(string or list)) and return an object unpacked 
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 1 arguments");
        if(args[0].valueType() != ValueType::String && args[0].valueType() != ValueType::List)
            throw Ark::TypeError("The packed buffer must be a string or a list");    
        Value dst;
        bool ark_bool;
        double ark_number;
        std::string ark_string;
        msgpack::object deserialized;
        auto type_test = [&](void) {
            std::string packed {static_cast<Value>(args[0]).string_ref()};
            deserialized = msgpack::unpack(packed.data(), packed.size()).get();
            try
            {
                deserialized.convert(ark_bool);
                dst = Value(ark_bool);
            }
            catch(const std::bad_cast &e)
            {
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
            }
        };

        if(args[0].valueType() == ValueType::List)
        {
            std::vector<Value> packed {static_cast<Value>(args[0]).list()};
            dst = list_unpacking(packed);
        }
        else
        {
            type_test();
        }

        return dst;
    }

    Value object_str(const std::vector<Value> &args)
    {
        if(args.size() != 1)
            throw std::runtime_error("ArgError : This function must have 1 argument");
        if(args[0].valueType() != ValueType::String && args[0].valueType() != ValueType::List)
            throw Ark::TypeError("The packed buffer must be a string or a list");
        Value msg_object_str;
        std::ostringstream str_buffer; 
        msgpack::object deserialized;

        if(args[0].valueType() == ValueType::List)
        {
            list_unpacked_str(static_cast<Value>(args[0]).list(), str_buffer);
            msg_object_str = Value(str_buffer.str());
        }
        else
        {
            std::string packed {static_cast<Value>(args[0]).string_ref()};
            deserialized =  msgpack::unpack(packed.data(), packed.size()).get();
            str_buffer << deserialized;
            msg_object_str = Value(str_buffer.str());
        }
        
        return msg_object_str;
    }
}

ARK_API_EXPORT Mapping_t getFunctionsMapping()
{
    Mapping_t map;

    map["msgPack"] = ArkMsgpack::pack;
    map["msgUnpack"] = ArkMsgpack::unpack;
    map["msgObjectStr"] = ArkMsgpack::object_str;

    return map;
}