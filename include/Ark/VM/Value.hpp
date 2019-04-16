#ifndef ark_vm_value
#define ark_vm_value

#include <variant>
#include <huge_number.hpp>
#include <string>
#include <cinttypes>

namespace Ark
{
    namespace VM
    {
        using namespace dozerg;
        using Value = std::variant<HugeNumber, std::string, uint16_t>;
    }
}

#endif