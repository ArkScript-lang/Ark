#ifndef _ARK_MSGPACK_H
#define _ARK_MSGPACK_H
#include <variant>
#include <Ark/Module.hpp>

namespace ArkMsgpack
{
	using ObjekType = std::variant<double, std::string, std::vector<Value>>;
	extern ObjekType get_objekt(const Value &arkObjekt, ValueType type);
}
#endif