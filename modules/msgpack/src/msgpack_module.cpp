#include <msgpack_module.hpp>

namespace ArkMsgpack
{
	ObjekType get_objekt(const Value &arkObjekt, ValueType type)
	{
		ObjekType objekt;

		if(type == ValueType::Number)
			objekt = static_cast<Value>(arkObjekt).number();
		else if(type == ValueType::String)
			objekt = static_cast<Value>(arkObjekt).string_ref();
		else
			objekt = static_cast<Value>(arkObjekt).list();

		return objekt;
	}
}