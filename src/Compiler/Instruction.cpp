#include <Ark/Compiler/Instructions.hpp>

namespace Ark
{
    namespace Compiler
    {
        Inst::Inst(Instruction inst) :
            inst((uint8_t) inst)
        {}

        Inst::Inst(uint8_t inst) :
            inst(inst)
        {}
    }
}