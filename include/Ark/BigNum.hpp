#ifndef ark_bignum
#define ark_bignum

#include <gmpxx.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#include <Ark/Exceptions.hpp>

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

        BigNum(const std::string& value, int base=10) :
            m_data(value, base)
        {}

        BigNum& operator+=(const BigNum& rhs) { m_data += rhs.m_data; return *this; }
        BigNum& operator-=(const BigNum& rhs) { m_data -= rhs.m_data; return *this; }
        BigNum& operator*=(const BigNum& rhs) { m_data *= rhs.m_data; return *this; }
        BigNum& operator/=(const BigNum& rhs) { m_data /= rhs.m_data; return *this; }

        BigNum operator+ (const BigNum& rhs) { return std::move(BigNum(*this) += rhs); }
        BigNum operator- ()                  { return std::move(BigNum(*this) *= BigNum(-1)); }
        BigNum operator- (const BigNum& rhs) { return std::move(BigNum(*this) -= rhs); }
        BigNum operator* (const BigNum& rhs) { return std::move(BigNum(*this) *= rhs); }
        BigNum operator/ (const BigNum& rhs) { return std::move(BigNum(*this) /= rhs); }

        inline friend bool operator< (const BigNum& lhs, const BigNum& rhs) { return lhs.m_data < rhs.m_data; }
        inline friend bool operator> (const BigNum& lhs, const BigNum& rhs) { return rhs < lhs; }
        inline friend bool operator<=(const BigNum& lhs, const BigNum& rhs) { return !(lhs > rhs); }
        inline friend bool operator>=(const BigNum& lhs, const BigNum& rhs) { return !(lhs < rhs); }
        inline friend bool operator==(const BigNum& lhs, const BigNum& rhs) { return lhs.m_data == rhs.m_data; }
        inline friend bool operator!=(const BigNum& lhs, const BigNum& rhs) { return !(lhs == rhs); }

        inline long toLong() const { return static_cast<long>(m_data.get_d()); }

        friend std::ostream& operator<<(std::ostream& os, const BigNum& rhs)
        {
            if (rhs.m_data.get_den() != 1)
                os << mpf_class(rhs.m_data);
            else
                os << rhs.m_data;
            return os;
        }

        std::string toString(int base=10) const
        {
            return m_data.get_str(base);
        }

    private:
        mpq_class m_data;
    };
}

#endif