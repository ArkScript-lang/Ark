/**
 * @file Word.hpp
 * @author  Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Describe an instruction and its immediate argument
 * @version 0.5
 * @date 2022-07-02
 *
 * @copyright Copyright (c) 2022-2024
 *
 */

#ifndef ARK_COMPILER_WORD_HPP
#define ARK_COMPILER_WORD_HPP

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
        uint8_t opcode = 0;   ///< Instruction opcode
        union {
            uint16_t data = 0;  ///< Immediate data, interpreted differently for different instructions
            bytes_t bytes;
        };

        explicit Word(const uint8_t inst, const uint16_t arg = 0) :
            opcode(inst), data(arg)
        {}
    };
}

#endif
