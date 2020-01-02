#include <msgpack_module.hpp>

namespace ArkMsgpack
{
	CObjekt get_cobjekt(const Value &arkObjekt, ValueType type)
	{
		CObjekt objekt;

		if(type == ValueType::Number)
			objekt = static_cast<Value>(arkObjekt).number();
		else if(type == ValueType::String)
			objekt = static_cast<Value>(arkObjekt).string_ref();
		else
			objekt = static_cast<Value>(arkObjekt).list();

		return objekt;
	}
	void list_packing(std::vector<Value> &list, std::stringstream &buffer)
	{
		CObjekt swap;

		for(unsigned i {0}; i < list.size(); ++ i)
		{
			ValueType type {list[i].valueType()};
			swap = get_cobjekt(list[i], type);
			if(type == ValueType::Number)
			{
				auto src = std::get<double>(swap);
				msgpack::pack(buffer, src);
				if(i != (list.size() - 1))
					buffer << COMMA;
			}
			else if(type == ValueType::String)
			{
				auto src = std::get<std::string>(swap);
				msgpack::pack(buffer, src);
				if(i != (list.size() - 1))
					buffer << COMMA;
			}
		}
		//std::cout << buffer.str() << std::endl;
	}
}