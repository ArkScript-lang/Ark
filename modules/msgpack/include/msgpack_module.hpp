#ifndef ARK_MSGPACK_HPP
#define ARK_MSGPACK_HPP
#include <variant>
#include <iostream> 
#include <sstream>
#include <msgpack.hpp>
#include <Ark/Module.hpp>

namespace ArkMsgpack
{
	using CObjekt = std::variant<double, std::string, std::vector<Value>>;
	extern CObjekt get_cobjekt(const Value &arkObjekt, ValueType type);
	extern Value list_packing(std::vector<Value> &list);
	extern Value list_unpacking(std::vector<Value> &buffer_list);
}
#endif