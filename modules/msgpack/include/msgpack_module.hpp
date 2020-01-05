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
	extern CObjekt get_cobjekt(const Value &ark_objekt, ValueType type);
	extern Value list_packing(std::vector<Value> &list);
	extern Value list_unpacking(std::vector<Value> &buffer_list);
	extern void list_unpacked_str(std::vector<Value> &buffer_list, std::ostringstream &stream);
    extern Value pack(const std::vector<Value> &args);
    extern Value unpack(const std::vector<Value> &args);
    extern Value objekt_str(const std::vector<Value> &args);
}
#endif