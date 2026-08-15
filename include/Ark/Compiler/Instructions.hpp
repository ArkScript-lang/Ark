/**
 * @file Instructions.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief The different instructions used by the compiler and virtual machine
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_COMPILER_INSTRUCTIONS_HPP
#define ARK_COMPILER_INSTRUCTIONS_HPP

#include <array>

namespace Ark::internal
{
    /**
     * @brief The different bytecodes are stored here
     * @par Adding an operator
     * It must be referenced as well under include/Ark/Compiler/Common.hpp, in
     * the operators table. The order of the operators below <code>FIRST_OPERATOR</code>
     * must be the same as the one in the operators table from the aforementioned file.
     *
     */
    enum Instruction : uint8_t
    {
        SYM_TABLE_START = 0xA1,
        VAL_TABLE_START = 0xA2,
        CODE_SEGMENT_START = 0xA3,
        NUMBER_TYPE = 0xF1,
        STRING_TYPE = 0xF2,
        FUNC_TYPE = 0xF3,
        FILENAMES_TABLE_START = 0xA4,
        INST_LOC_TABLE_START = 0xA5,

#define X(name, value) name = (value),
#include "Instructions.x"

#undef X

        InstructionsCount
    };

    constexpr uint8_t FirstOperator = Instruction::BREAKPOINT;

    constexpr std::array InstructionNames = {
#define X(name, value) #name,
#include "Instructions.x"

#undef X
    };

    static_assert(InstructionNames.size() == static_cast<std::size_t>(Instruction::InstructionsCount) && "Some instruction names appear to be missing");
}

#endif
