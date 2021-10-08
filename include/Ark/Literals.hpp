/**
 * @file Literals.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief User defined literals for Ark internals
 * @version 0.2
 * @date 2021-10-2
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

namespace Ark::literals
{
    inline uint8_t operator""_u8(unsigned long long int num)
    {
        return static_cast<uint8_t>(num);
    }

    inline uint16_t operator""_u16(unsigned long long int num)
    {
        return static_cast<uint16_t>(num);
    }
}
