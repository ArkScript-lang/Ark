/**
 * @file Word.hpp
 * @author  Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Describe an instruction and its immediate argument
 * @version 1.0
 * @date 2022-07-02
 *
 * @copyright Copyright (c) 2022-2024
 *
 */

#ifndef ARK_COMPILER_WORD_HPP
#define ARK_COMPILER_WORD_HPP

namespace Ark::internal
{
    struct Word
    {
        uint8_t opcode = 0;  ///< Instruction opcode
        uint8_t byte_1 = 0;
        uint8_t byte_2 = 0;
        uint8_t byte_3 = 0;

        explicit Word(const uint8_t inst, const uint16_t arg = 0) :
            opcode(inst), byte_2(static_cast<uint8_t>(arg >> 8)), byte_3(static_cast<uint8_t>(arg & 0xff))
        {}

        /**
         * @brief Construct a word with two arguments, each on 12 bits. It's up to the caller to ensure that no data is lost
         * @param inst
         * @param primary_arg argument on 12 bits, the upper 4 bits are lost
         * @param secondary_arg 2nd argument on 12 bits, the upper 4 bits are lost
         */
        Word(const uint8_t inst, const uint16_t primary_arg, const uint16_t secondary_arg) :
            opcode(inst)
        {
            byte_1 = static_cast<uint8_t>((primary_arg & 0xff0) >> 4);
            byte_2 = static_cast<uint8_t>((primary_arg & 0x00f) << 4 | (secondary_arg & 0xf00) >> 8);
            byte_3 = static_cast<uint8_t>(secondary_arg & 0x0ff);
        }
    };
}

#endif
