#include <Ark/Compiler/Instructions.hpp>

namespace Ark
{
    namespace internal
    {
        Inst::Inst(Instruction inst) :
            inst(static_cast<uint8_t>(inst))
        {}

        Inst::Inst(uint8_t inst) :
            inst(inst)
        {}
    }
}