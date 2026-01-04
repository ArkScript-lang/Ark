/**
 * @file Literals.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief User defined literals for Ark internals
 * @date 2021-10-2
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_LITERALS
#define ARK_LITERALS

namespace Ark::literals
{
    /**
     * @brief Suffix to easily write uint8_t: 1_u8
     * @param num
     * @return uint8_t
     */
    consteval uint8_t operator""_u8(const unsigned long long int num)
    {
        return static_cast<uint8_t>(num);
    }

    /**
     * @brief Suffix to easily write uint16_t: 1_u16
     * @param num
     * @return uint16_t
     */
    consteval uint16_t operator""_u16(const unsigned long long int num)
    {
        return static_cast<uint16_t>(num);
    }

    /**
     * @brief Suffix to easily write std::size_t: 1_z
     * @param num
     * @return std::size_t
     */
    consteval std::size_t operator""_z(const unsigned long long int num)
    {
        return static_cast<std::size_t>(num);
    }
}

#endif  // ARK_LITERALS
