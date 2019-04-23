#ifndef ark_vm_value
#define ark_vm_value

#include <variant>
#include <huge_number.hpp>
#include <string>
#include <cinttypes>
#include <Ark/Lang/Node.hpp>

namespace Ark
{
    namespace VM
    {
        using namespace dozerg;

        enum class NFT { Nil, False, True };
        using PageAddr_t = uint16_t;
        using Value = std::variant<HugeNumber, std::string, PageAddr_t, NFT, Ark::Lang::ProcType>;
    }
}

#endif