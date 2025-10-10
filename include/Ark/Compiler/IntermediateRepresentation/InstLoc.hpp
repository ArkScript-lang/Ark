#ifndef ARK_COMPILER_INTERMEDIATEREPRESENTATION_INSTLOC_HPP
#define ARK_COMPILER_INTERMEDIATEREPRESENTATION_INSTLOC_HPP

#include <cstdint>

namespace Ark::internal
{
    // pp (2 bytes), ip (2 bytes), filename id (2 bytes), line (4 bytes) -> 10 bytes per record
    struct InstLoc
    {
        uint16_t page_pointer;
        uint16_t inst_pointer;
        uint16_t filename_id;
        uint32_t line;

        std::strong_ordering operator<=>(const InstLoc&) const = default;
    };
}

#endif  // ARK_COMPILER_INTERMEDIATEREPRESENTATION_INSTLOC_HPP
