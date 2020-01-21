#include <msgpack_module.hpp>

namespace ArkMsgpack
{
    CObject get_cobject(const Value &ark_object, ValueType type)
    {
        CObject object;

        if(type == ValueType::NFT)
        {
            if(ark_object == Ark::True)
                object = true;
            else if(ark_object == Ark::False)
                object = false;
        }
        else if(type == ValueType::Number)
            object = static_cast<Value>(ark_object).number();
        else if(type == ValueType::String)
            object = static_cast<Value>(ark_object).string_ref();
        else
            object = static_cast<Value>(ark_object).list();

        return object;
    }

    Value list_packing(std::vector<Value> &src_list)
    {
        std::vector<Value> list;
        std::stringstream buffer;
        CObject each;

        for(unsigned i {0}; i < src_list.size(); ++ i)
        {
            ValueType type {src_list[i].valueType()};
            each = get_cobject(src_list[i], type);
            if(type == ValueType::NFT)
            {
                bool src = std::get<bool>(each);
                msgpack::pack(buffer, src);
                list.push_back(Value(buffer.str()));
                buffer.str("");
            }
            else if(type == ValueType::Number)
            {
                double src = std::get<double>(each);
                msgpack::pack(buffer, src);
                list.push_back(Value(buffer.str()));
                buffer.str("");
            }
            else if(type == ValueType::String)
            {
                std::string src = std::get<std::string>(each);
                msgpack::pack(buffer, src);
                list.push_back(Value(buffer.str()));
                buffer.str("");
            }
        }

        return list;
    }

    Value list_unpacking(std::vector<Value> &buffer_list)
    {
        std::vector<Value> list;
        bool ark_bool;
        double ark_number;
        std::string ark_string;
        msgpack::object deserialized;
        auto each_to_value = [&](void) {
            try
            {
                deserialized.convert(ark_bool);
                list.push_back(Value(ark_bool));
            }
            catch(const std::bad_cast &e)
            {
                try
                {
                    deserialized.convert(ark_number);
                    list.push_back(Value(ark_number));
                }
                catch(const std::bad_cast &e) 
                {
                    try
                    {
                        deserialized.convert(ark_string);
                        list.push_back(Value(ark_string));                  
                    }
                    catch(std::exception &e) {}
                }
            }
        };
 
        for(unsigned i {0}; i < buffer_list.size(); ++ i)
        {
            std::string buffer {static_cast<Value>(buffer_list[i]).string_ref()};
            deserialized = msgpack::unpack(buffer.data(), buffer.size()).get();
            each_to_value();
        }

        return list;
    }

    void list_unpacked_str(std::vector<Value> &buffer_list, std::ostringstream &stream)
    {
        msgpack::object deserialized;
        stream << '[';
 
        for(unsigned i {0}; i < buffer_list.size(); ++ i)
        {
            std::string buffer {static_cast<Value>(buffer_list[i]).string_ref()};
            deserialized = msgpack::unpack(buffer.data(), buffer.size()).get();
            if(i > 0)
                stream << ' ';
            stream << deserialized;
        }
        
        stream << ']';
    }
}