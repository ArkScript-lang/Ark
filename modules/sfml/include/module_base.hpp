#include <string>
#include <vector>
#include <unordered_map>
#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>

using namespace Ark::VM;
using Mapping_t = std::unordered_map<std::string, Value::ProcType>;

const Value falseSym = Value(NFT::False);
const Value trueSym  = Value(NFT::True);
const Value nil      = Value(NFT::Nil);