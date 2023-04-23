/**
 * @file Word.hpp
 * @author  Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Describe an instruction and its immediate argument
 * @version 0.3
 * @date 2022-07-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef ARK_COMPILER_WORD_HPP
#define ARK_COMPILER_WORD_HPP

#include <cinttypes>
#include <Ark/Compiler/Instructions.hpp>

namespace Ark::internal
{
    struct bytes_t
    {
        uint8_t second;
        uint8_t first;
    };

    struct Word
    {
        uint8_t padding = 0;  ///< Padding reserved for future use
        uint8_t opcode;       ///< Instruction opcode
        union {
            uint16_t data;  ///< Immediate data, interpreted differently for different instructions
            bytes_t bytes;
        };

        Word() :
            opcode(0), data(0)
        {}

        Word(uint8_t inst, uint16_t arg = 0) :
            opcode(inst), data(arg)
        {}
    };
}

#endif
