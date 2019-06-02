#ifndef ark_bignum
#define ark_bignum

#include <Ark/Constants.hpp>

#ifdef ARK_USE_MPIR
    #include <gmpxx.h>
#endif

#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    class BigNum
    {
    public:
        BigNum() :
            m_data(0)
        {}

        explicit BigNum(long value) :
            m_data(value)
        {}

        BigNum(const std::string& value)
        {
#ifdef ARK_USE_MPIR
            m_data = mpq_class(value);
#else
            m_data = std::stod(value);
#endif
        }

        inline BigNum& operator+=(const BigNum& rhs) { m_data += rhs.m_data; return *this; }
        inline BigNum& operator-=(const BigNum& rhs) { m_data -= rhs.m_data; return *this; }
        inline BigNum& operator*=(const BigNum& rhs) { m_data *= rhs.m_data; return *this; }
        inline BigNum& operator/=(const BigNum& rhs) { m_data /= rhs.m_data; return *this; }

        inline BigNum operator+ (const BigNum& rhs) { return std::move(BigNum(*this) += rhs); }
        inline BigNum operator- ()                  { return std::move(BigNum(*this) *= BigNum(-1)); }
        inline BigNum operator- (const BigNum& rhs) { return std::move(BigNum(*this) -= rhs); }
        inline BigNum operator* (const BigNum& rhs) { return std::move(BigNum(*this) *= rhs); }
        inline BigNum operator/ (const BigNum& rhs) { return std::move(BigNum(*this) /= rhs); }

        inline friend bool operator< (const BigNum& lhs, const BigNum& rhs) { return lhs.m_data < rhs.m_data; }
        inline friend bool operator> (const BigNum& lhs, const BigNum& rhs) { return rhs < lhs; }
        inline friend bool operator<=(const BigNum& lhs, const BigNum& rhs) { return !(lhs > rhs); }
        inline friend bool operator>=(const BigNum& lhs, const BigNum& rhs) { return !(lhs < rhs); }
        inline friend bool operator==(const BigNum& lhs, const BigNum& rhs) { return lhs.m_data == rhs.m_data; }
        inline friend bool operator!=(const BigNum& lhs, const BigNum& rhs) { return !(lhs == rhs); }

        inline long toLong() const
        {
#ifdef ARK_USE_MPIR
            return static_cast<long>(m_data.get_d());
#else
            return static_cast<long>(m_data);
#endif
        }

        friend std::ostream& operator<<(std::ostream& os, const BigNum& rhs)
        {
#ifdef ARK_USE_MPIR
            if (rhs.m_data.get_den() != 1)
                os << mpf_class(rhs.m_data);
            else
#endif
                os << rhs.m_data;
            return os;
        }

        std::string toString() const
        {
#ifdef ARK_USE_MPIR
            return m_data.get_str();
#else
            return Ark::Utils::toString(m_data);
#endif
        }

    private:
#ifdef ARK_USE_MPIR
        mpq_class m_data;
#else
        double m_data;
#endif
    };
}

#endif