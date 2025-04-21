#ifndef ARK_COMPILER_SERIALIZATION_INTEGERSERIALIZER_HPP
#define ARK_COMPILER_SERIALIZATION_INTEGERSERIALIZER_HPP

#include <span>
#include <vector>
#include <concepts>

namespace Ark::internal
{
    void serializeToVecLE(std::integral auto number, std::vector<uint8_t>& out)
    {
        constexpr auto mask = static_cast<decltype(number)>(0xff);

        for (std::size_t i = 0; i < sizeof(decltype(number)); ++i)
            out.push_back(static_cast<uint8_t>((number & (mask << (8 * i))) >> (8 * i)));
    }

    void serializeToVecBE(std::integral auto number, std::vector<uint8_t>& out)
    {
        constexpr auto pad = sizeof(decltype(number)) - 1;
        constexpr auto mask = static_cast<decltype(number)>(0xff);

        for (std::size_t i = 0; i < sizeof(decltype(number)); ++i)
        {
            const auto shift = 8 * (pad - i);
            out.push_back(static_cast<uint8_t>((number & (mask << shift)) >> shift));
        }
    }

    void serializeOn2BytesToVecLE(std::integral auto number, std::vector<uint8_t>& out)
    {
        constexpr auto mask = static_cast<decltype(number)>(0xff);
        out.push_back(static_cast<uint8_t>(number & mask));
        out.push_back(static_cast<uint8_t>((number & (mask << 8)) >> 8));
    }

    void serializeOn2BytesToVecBE(std::integral auto number, std::vector<uint8_t>& out)
    {
        constexpr auto mask = static_cast<decltype(number)>(0xff);
        out.push_back(static_cast<uint8_t>((number & (mask << 8)) >> 8));
        out.push_back(static_cast<uint8_t>(number & mask));
    }

    template <std::integral T>
    T deserializeLE(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
    {
        constexpr std::size_t length = sizeof(T);
        T result {};
        for (std::size_t i = 0; i < length && begin != end; ++i, ++begin)
            result += static_cast<T>(*begin) << (8 * i);

        return result;
    }

    template <std::integral T>
    T deserializeBE(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
    {
        constexpr std::size_t length = sizeof(T) - 1;
        T result {};
        for (std::size_t i = 0; i < length && begin != end; ++i, ++begin)
            result += static_cast<T>(*begin) << (8 * (length - i));

        return result;
    }
}

#endif  // ARK_COMPILER_SERIALIZATION_INTEGERSERIALIZER_HPP
