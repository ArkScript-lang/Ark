#include <string>
#include <unordered_map>
#include <random>
#include <chrono>

#include <Ark/VM/Value.hpp>

using namespace Ark::VM;
using Mapping_t = std::unordered_map<std::string, Value::ProcType>;

const Value falseSym = Value(NFT::False);
const Value trueSym  = Value(NFT::True);
const Value nil      = Value(NFT::Nil);

Value random(const std::vector<Value>& n)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);
    return Value(static_cast<int>(g()));
}

extern "C" Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["random"] = &random;

    return map;
}