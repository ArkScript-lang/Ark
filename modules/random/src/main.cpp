#include <string>
#include <unordered_map>
#include <random>
#include <chrono>

#include <Ark/VM/VM.hpp>

using namespace Ark;
using namespace Ark::internal;

using Mapping_t = std::unordered_map<std::string, Value::ProcType>;

const Value falseSym = Value(NFT::False);
const Value trueSym  = Value(NFT::True);
const Value nil      = Value(NFT::Nil);

namespace ArkRandom
{
    Value random(const std::vector<Value>& n)
    {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 g(seed);
        int x = static_cast<int>(g());
        x = (x > 0) ? x : -x;
        x %= 16384;
        return Value(x);
    }

    Value random_10(const std::vector<Value>& n)
    {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 g(seed);
        int x = static_cast<int>(g());
        x = (x > 0) ? x : -x;
        x %= 10;
        return Value(x);
    }
}

extern "C" Mapping_t getFunctionsMapping()
{
    Mapping_t map;
    map["random"] = ArkRandom::random;
    map["random-10"] = ArkRandom::random_10;

    return map;
}