#ifndef ark_bignum
#define ark_bignum

#include <gmpxx.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

namespace Ark
{
    class BigNum
    {
    public:
        BigNum() :
            m_data(0)
        {}

        BigNum(long value) :
            m_data(value)
        {}

        BigNum(const std::string& value) :
            m_data(value)
        {}

        BigNum& operator+=(const BigNum& rhs) { m_data += rhs.m_data; return *this; }
        BigNum& operator-=(const BigNum& rhs) { m_data -= rhs.m_data; return *this; }
        BigNum& operator*=(const BigNum& rhs) { m_data *= rhs.m_data; return *this; }
        BigNum& operator/=(const BigNum& rhs) { m_data /= rhs.m_data; return *this; }
        // BigNum& operator%=(const BigNum& rhs) { m_data %= rhs.m_data; return *this; }
        // BigNum& operator<<=(const BigNum& rhs) { m_data <<= rhs.m_data; return *this; }
        // BigNum& operator>>=(const BigNum& rhs) { m_data >>= rhs.m_data; return *this; }
        BigNum& pow(const BigNum& rhs)
        {
            mpf_class r;
            if (rhs.m_data >= 0)
                mpf_pow_ui(r.get_mpf_t(), m_data.get_mpf_t(), rhs.m_data.get_ui());
            else
                mpf_pow_ui(r.get_mpf_t(), mpf_class(1 / m_data).get_mpf_t(), mpf_class(-rhs.m_data).get_ui());
            m_data = r;
            return *this;
        }
        BigNum& sqrt()
        {
            mpf_class r;
            mpf_sqrt(r.get_mpf_t(), m_data.get_mpf_t());
            m_data = r;
            return *this;
        }

        BigNum operator+ (const BigNum& rhs) { return std::move(BigNum(*this) += rhs); }
        BigNum operator- ()                  { return std::move(BigNum(*this) *= BigNum(-1)); }
        BigNum operator- (const BigNum& rhs) { return std::move(BigNum(*this) -= rhs); }
        BigNum operator* (const BigNum& rhs) { return std::move(BigNum(*this) *= rhs); }
        BigNum operator/ (const BigNum& rhs) { return std::move(BigNum(*this) /= rhs); }
        // BigNum operator% (const BigNum& rhs) { return std::move(BigNum(*this) %= rhs); }
        // BigNum operator<<(const BigNum& rhs) { return std::move(BigNum(*this) <<= rhs); }
        // BigNum operator>>(const BigNum& rhs) { return std::move(BigNum(*this) >>= rhs); }

        inline friend bool operator< (const BigNum& lhs, const BigNum& rhs) { return lhs.m_data < rhs.m_data; }
        inline friend bool operator> (const BigNum& lhs, const BigNum& rhs) { return rhs < lhs; }
        inline friend bool operator<=(const BigNum& lhs, const BigNum& rhs) { return !(lhs > rhs); }
        inline friend bool operator>=(const BigNum& lhs, const BigNum& rhs) { return !(lhs < rhs); }
        inline friend bool operator==(const BigNum& lhs, const BigNum& rhs) { return lhs.m_data == rhs.m_data; }
        inline friend bool operator!=(const BigNum& lhs, const BigNum& rhs) { return !(lhs == rhs); }

        friend std::ostream& operator<<(std::ostream& os, const BigNum& rhs)
        {
            os << rhs.m_data;
            return os;
        }

        std::string toString() const
        {
            std::stringstream ss;
            ss << std::fixed << m_data;
            return ss.str();
        }

    private:
        mpf_class m_data;
    };
}

#endif