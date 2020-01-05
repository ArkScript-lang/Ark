#include <msgpack_module.hpp>

namespace ArkMsgpack
{
	CObjekt get_cobjekt(const Value &ark_objekt, ValueType type)
	{
		CObjekt objekt;

		if(type == ValueType::Number)
			objekt = static_cast<Value>(ark_objekt).number();
		else if(type == ValueType::String)
			objekt = static_cast<Value>(ark_objekt).string_ref();
		else
			objekt = static_cast<Value>(ark_objekt).list();

		return objekt;
	}
	Value list_packing(std::vector<Value> &src_list)
	{
		std::vector<Value> list;
		std::stringstream buffer;
		CObjekt each;

		for(unsigned i {0}; i < src_list.size(); ++ i)
		{
			ValueType type {src_list[i].valueType()};
			each = get_cobjekt(src_list[i], type);
			if(type == ValueType::Number)
			{
				auto src = std::get<double>(each);
				msgpack::pack(buffer, src);
				list.push_back(Value(buffer.str()));
				buffer.str("");
			}
			else if(type == ValueType::String)
			{
				auto src = std::get<std::string>(each);
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
		double number;
		std::string str;
		msgpack::object deserialized;
		auto each_to_value = [&](void) {
			try
			{
				deserialized.convert(number);
				list.push_back(Value(number));
			}
			catch(const std::bad_cast &e)
			{
				try
				{
					deserialized.convert(str);
					list.push_back(Value(str));
				}
				catch(const std::exception &e) {}
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
		auto each_to_value = [&](auto i) {
			try
			{
            	if(i > 0) {stream << ' ';}
            	stream << deserialized;
			}
			catch(const std::bad_cast &e)
			{
				try
				{
            		stream << ' ';
            		stream << deserialized;
				}
				catch(const std::exception &e) {}
			}
		};
 
		for(unsigned i {0}; i < buffer_list.size(); ++ i)
		{
			std::string buffer {static_cast<Value>(buffer_list[i]).string_ref()};
			deserialized = msgpack::unpack(buffer.data(), buffer.size()).get();
			each_to_value(i);
		}
		stream << ']';
	}
}