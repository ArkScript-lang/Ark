#ifndef ARK_COMPILER_SERIALIZATION_IEEE754SERIALIZER_HPP
#define ARK_COMPILER_SERIALIZATION_IEEE754SERIALIZER_HPP

namespace Ark::internal::ieee754
{
    // Narrowing conversion from long long to double, 9223372036854775807 becomes 9223372036854775808.
    // This gives us an error margin of 1.08420217248550443400745280086994171142578125 * 10^-19,
    // which is acceptable.
    static constexpr auto MaxLong = static_cast<double>(std::numeric_limits<std::int64_t>::max());

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    template <class T>
    constexpr T bswap(T i)
    {
        return i;
    }
#else
    // https://stackoverflow.com/a/36937049/21584
    template <class T, std::size_t... N>
    constexpr T bswap_impl(T i, std::index_sequence<N...>)
    {
        return (((i >> N * CHAR_BIT & static_cast<std::uint8_t>(-1)) << (sizeof(T) - 1 - N) * CHAR_BIT) | ...);
    }
    template <class T, class U = std::make_unsigned_t<T>>
    constexpr T bswap(T i)
    {
        return std::bit_cast<T>(bswap_impl<U>(std::bit_cast<U>(i), std::make_index_sequence<sizeof(T)> {}));
    }
#endif

    struct DecomposedDouble
    {
        int32_t exponent;
        int64_t mantissa;
    };

    [[nodiscard]] inline DecomposedDouble serialize(const double n)
    {
        int exp = 0;
        const auto mant = static_cast<std::int64_t>(MaxLong * std::frexp(n, &exp));

        return DecomposedDouble {
            .exponent = std::bit_cast<int32_t>(bswap(exp)),
            .mantissa = bswap(mant)
        };
    }

    [[nodiscard]] inline double deserialize(const DecomposedDouble d)
    {
        return std::ldexp(static_cast<double>(bswap(d.mantissa)) / MaxLong, std::bit_cast<int32_t>(bswap(d.exponent)));
    }
}

#endif  // ARK_COMPILER_SERIALIZATION_IEEE754SERIALIZER_HPP
