/**
 * @file Word.hpp
 * @author  Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Describe an instruction and its immediate argument
 * @version 0.1
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
    struct Word
    {
        uint8_t padding = 0;  ///< Padding reserved for future use
        uint8_t opcode;       ///< Instruction opcode
        union {
            uint16_t value;  ///< Immediate data, interpreted differently for different instructions
            struct
            {
                uint8_t first_half;
                uint8_t second_half;
            };
        } data;

        Word(Instruction inst, uint16_t arg = 0)
        {
            opcode = inst;
            data.value = arg;
        }
    };
}

#endif
