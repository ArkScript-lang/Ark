#ifndef DOZERG_HUGE_NUMBER_20150715
#define DOZERG_HUGE_NUMBER_20150715

#include <limits>       //numeric_limits
#include <utility>      //move, swap
#include <vector>       //vector
#include <string>       //string
#include <iostream>     //ostream
#include <cassert>      //assert
#include <algorithm>    //reverse
#include <type_traits>  //make_signed_t
#include <stdexcept>    //invalid_argument, runtime_error

namespace dozerg {

    class HugeNumber
    {
        //types
        typedef HugeNumber                  __Myt;
        typedef unsigned long long          __Int;
        typedef std::make_signed_t<__Int>   __SInt;
        typedef std::vector<__Int>          __Data;
        template<bool B, typename T1, typename T2>struct __TypeSelect { typedef T2 type; };
        template<typename T1, typename T2>struct __TypeSelect<true, T1, T2> { typedef T1 type; };
        template<typename T>struct __SupportType;
        template<typename T>using __SupportTypeT = typename __SupportType<T>::type;
        //constants
        static constexpr int kEachBits = std::numeric_limits<__Int>::digits;
    public:
        //functions
        HugeNumber() {}  //Cannot be default for "const HugeNumber a;"
        HugeNumber(const __Myt & a) = default;
        HugeNumber(__Myt && a) : data_(std::move(a.data_)), sign_(a.sign_) {}
        template<typename T>
        explicit HugeNumber(const T & a) { from(__SupportTypeT<T>(a)); }
        __Myt & operator =(const __Myt & a) = default;
        __Myt && operator =(__Myt && a) {
            if (&a != this) {
                data_ = std::move(a.data_);
                sign_ = a.sign_;
            }
            return std::move(*this);
        }
        template<typename T>
        __Myt & operator =(const T & a) { from(__SupportTypeT<T>(a)); return *this; }
        //a.swap(b);
        void swap(__Myt & a) noexcept {
            std::swap(sign_, a.sign_);
            data_.swap(a.data_);
        }
        //+a;
        __Myt operator +() { return *this; }
        //-a;
        __Myt operator -() { return std::move(__Myt(*this).negate()); }
        //++a;
        __Myt & operator ++() { *this += 1; return *this; }
        //--a;
        __Myt & operator --() { *this -= 1; return *this; }
        //a++;
        __Myt operator ++(int) {
            auto t(*this);
            ++*this;
            return std::move(t);
        }
        //a--;
        __Myt operator --(int) {
            auto t(*this);
            --*this;
            return std::move(t);
        }
        //a += b;
        __Myt & operator +=(const __Myt & a) { add(a.sign_, a.data_); return *this; }
        template<typename T>
        __Myt & operator +=(const T & a) { add(__SupportTypeT<T>(a)); return *this; }
        //a -= b;
        __Myt & operator -=(const __Myt & a) { add(!a.sign_, a.data_); return *this; }
        template<typename T>
        __Myt & operator -=(const T & a) { sub(__SupportTypeT<T>(a)); return *this; }
        //a *= b;
        __Myt & operator *=(const __Myt & a) { mul(a.sign_, a.data_); return *this; }
        template<typename T>
        __Myt & operator *=(const T & a) { mul(__SupportTypeT<T>(a)); return *this; }
        //a /= b;
        __Myt & operator /=(const __Myt & a) { div(a); return *this; }
        template<typename T>
        __Myt & operator /=(const T & a) { return (*this /= __Myt(a)); }
        //a %= b;
        __Myt & operator %=(const __Myt & a) { mod(a); return *this; }
        template<typename T>
        __Myt & operator %=(const T & a) { return (*this %= __Myt(a)); }
        //a <<= n;
        __Myt & operator <<=(int a) { (a < 0 ? shiftRight(-a) : shiftLeft(a)); return *this; }
        //a >>= n;
        __Myt & operator >>=(int a) { (a < 0 ? shiftLeft(-a) : shiftRight(a)); return *this; }
        //a + b;
        __Myt operator +(const __Myt & a) const { return std::move(__Myt(*this) += a); }
        template<class T>
        __Myt operator +(const T & a) const { return std::move(__Myt(*this) += a); }
        template<class T>
        friend __Myt operator +(const T & a, const __Myt & b) { return (b + a); }
        //a - b;
        __Myt operator -(const __Myt & a) const { return std::move(__Myt(*this) -= a); }
        template<class T>
        __Myt operator -(const T & a) const { return std::move(__Myt(*this) -= a); }
        template<class T>
        friend __Myt operator -(const T & a, const __Myt & b) { return std::move(__Myt(b - a).negate()); }
        //a * b;
        __Myt operator *(const __Myt & a) const { return std::move(__Myt(*this) *= a); }
        template<class T>
        __Myt operator *(const T & a) const { return std::move(__Myt(*this) *= a); }
        template<class T>
        friend __Myt operator *(const T & a, const __Myt & b) { return (b * a); }
        //a / b;
        __Myt operator /(const __Myt & a) const { return std::move(__Myt(*this) /= a); }
        template<class T>
        __Myt operator /(const T & a) const { return std::move(__Myt(*this) /= a); }
        template<class T>
        friend __Myt operator /(const T & a, const __Myt & b) { return std::move(__Myt(a) /= b); }
        //a % b;
        __Myt operator %(const __Myt & a) const { return std::move(__Myt(*this) %= a); }
        template<class T>
        __Myt operator %(const T & a) const { return std::move(__Myt(*this) %= a); }
        template<class T>
        friend __Myt operator %(const T & a, const __Myt & b) { return std::move(__Myt(a) %= b); }
        //a << n;
        __Myt operator <<(int a) const { return std::move(__Myt(*this) <<= a); }
        //a >> n;
        __Myt operator >>(int a) const { return std::move(__Myt(*this) >>= a); }
        //if(a){}
        explicit operator bool() const { return !operator !(); }
        //if(!a){}
        bool operator !() const { return data_.empty(); }
        //a == b;
        bool operator ==(const __Myt & a) const { return (sign_ == a.sign_ && data_ == a.data_); }
        template<typename T>
        bool operator ==(const T & a) const { return equal(__SupportTypeT<T>(a)); }
        template<typename T>
        friend bool operator ==(const T & a, const __Myt & b) { return (b == a); }
        //a != b;
        bool operator !=(const __Myt & a) const { return !(*this == a); }
        template<class T>
        bool operator !=(const T & a) const { return !(*this == a); }
        template<class T>
        friend bool operator !=(const T & a, const __Myt & b) { return (b != a); }
        //a < b;
        bool operator <(const __Myt & a) const { return less(a.sign_, a.data_); }
        template<typename T>
        bool operator <(const T & a) const { return less(__SupportTypeT<T>(a)); }
        template<typename T>
        friend bool operator <(const T & a, const __Myt & b) { return (b > a); }
        //a > b;
        bool operator >(const __Myt & a) const { return (a < *this); }
        template<typename T>
        bool operator >(const T & a) const { return greater(__SupportTypeT<T>(a)); }
        template<typename T>
        friend bool operator >(const T & a, const __Myt & b) { return (b < a); }
        //a <= b;
        bool operator <=(const __Myt & a) const { return !(a < *this); }
        template<class T>
        bool operator <=(const T & a) const { return !(a < *this); }
        template<typename T>
        friend bool operator <=(const T & a, const __Myt & b) { return !(b < a); }
        //a >= b;
        bool operator >=(const __Myt & a) const { return !(*this < a); }
        template<class T>
        bool operator >=(const T & a) const { return !(*this < a); }
        template<typename T>
        friend bool operator >=(const T & a, const __Myt & b) { return !(a < b); }
        //a.toString();
        std::string toString(int base = 10, bool uppercase = false, bool showbase = false) const {
            const char * const kDigits = (uppercase ? "0123456789ABCDEF" : "0123456789abcdef");
            switch (base) {
                case 16:return toStringX<4>(kDigits, (showbase ? (uppercase ? "X0" : "x0") : nullptr));
                case 10:return toString10();
                case 8:return toStringX<3>(kDigits, (showbase ? "0" : nullptr));
                case 2:return toStringX<1>(kDigits, (showbase ? (uppercase ? "B0" : "b0") : nullptr));
                default:;
            }
            throw std::invalid_argument("Unsupported base");
            return std::string();
        }
        //cout<<a;
        friend inline std::ostream & operator <<(std::ostream & os, const __Myt & a) {
            const auto fmt = os.flags();
            const int base = ((fmt & os.hex) ? 16 : ((fmt & os.oct) ? 8 : 10));
            const bool uppercase = (0 != (fmt & os.uppercase));
            const bool showbase = (0 != (fmt & os.showbase));
            return (os << a.toString(base, uppercase, showbase));
        }
    private:
        void from(const __SInt & a) {
            reset(a < 0);
            if (a)
                data_.push_back(abs(a));
        }
        void from(const __Int & a) {
            reset();
            if (a)
                data_.push_back(a);
        }
        void from(const std::string & a) {
            switch (checkBase(a)) {
                case 0:
                case 3:reset(); break;
                case 2: {
                    reset('-' == a[0]);
                    int p = 0;
                    for (auto it = a.rbegin(); it != a.rend() && 'b' != *it && 'B' != *it; toBits<1>(data_, p, *it++ - '0'));
                    break; }
                case 8: {
                    reset('-' == a[0]);
                    int p = 0;
                    for (auto it = a.rbegin(); it != a.rend() && '+' != *it && '-' != *it; toBits<3>(data_, p, *it++ - '0'));
                    break; }
                case 10: {
                    reset('-' == a[0]);
                    std::string t(a);
                    if ('+' == t[0] || '-' == t[0])
                        t[0] = '0';
                    reverseAdd(t, -'0');
                    int p = 0;
                    for (bool end = false; !end; toBitsF<1>(data_, p, [&t, &end] {
                        int c = 0;
                        for (auto it = t.rbegin(); it != t.rend(); ++it) {
                            *it += c * 10;
                            c = (*it & 1);
                            *it /= 2;
                        }
                        const auto p = t.find_last_not_of(char(0)) + 1;
                        end = !p;
                        t.erase(p);
                        return c;
                    }));
                    break; }
                case 16: {
                    reset('-' == a[0]);
                    int p = 0;
                    for (auto it = a.rbegin(); it != a.rend() && 'x' != *it && 'X' != *it; toBitsF<4>(data_, p, [&it] {
                        const char c = *it++;
                        if ('0' <= c && c <= '9')
                            return (c - '0');
                        if ('a' <= c && c <= 'f')
                            return (c - 'a' + 10);
                        return (c - 'A' + 10);
                    }));
                    break; }
                default:throw std::invalid_argument("Input is not an integer number");
            }
            shrink();
        }
        __Myt & negate() {
            if (*this)
                sign_ = !sign_;
            return *this;
        }
        void add(const __Int & a) { add(false, a); }
        void add(const __SInt & a) { add((a < 0), abs(a)); }
        void add(const std::string & a) { *this += __Myt(a); }
        template<class T>
        void add(bool s, const T & a) {
            if (sign_ == s) {
                addAbs(a);
                return;
            }
            switch (compare(a)) {
                case 0:reset(); break;
                case 1:subAbs(a); break;
                default:sign_ = s; subByAbs(a);
            }
        }
        void sub(const __Int & a) { add(true, a); }
        void sub(const __SInt & a) { add((a > 0), abs(a)); }
        void sub(const std::string & a) { *this -= __Myt(a); }
        void mul(const __SInt & a) { mul(abs(a), (a < 0)); }
        void mul(const __Int & a, bool s = false) {
            if (mulSign(s, !a, (1 == a)))
                return;
            __Myt t(*this), r;
            for (__Int i = a; i; i >>= 1, t <<= 1)
                if (0 != (i & 1))
                    r += t;
            data_.swap(r.data_);
            shrink();
        }
        void mul(const std::string & a) { *this *= __Myt(a); }
        void mul(bool s, const __Data & a) {
            if (mulSign(s, a.empty(), false))
                return;
            __Myt t(*this), r;
            forBits<1>(a, 0, [&r, &t](const auto & i) {
                if (i)
                    r += t;
                t <<= 1;
            });
            data_.swap(r.data_);
            shrink();
        }
        void div(const __Myt & a) {
            if (!a)
                throw std::runtime_error("Divided by 0");
            if (!*this)
                return;
            const int p = topBit(data_) - topBit(a.data_) + 1;
            if (p > 0) {
                __Myt d, r;
                divModAbs(a, d, r, p);
                data_.swap(d.data_);
                sign_ = (sign_ != a.sign_);
                shrink();
            } else
                reset();
        }
        void mod(const __Myt & a) {
            if (!a)
                throw std::runtime_error("Divided by 0");
            if (!*this)
                return;
            const int p = topBit(data_) - topBit(a.data_) + 1;
            if (p > 0) {
                __Myt q, r;
                divModAbs(a, q, r, p);
                data_.swap(r.data_);
                shrink();
            }
        }
        void divModAbs(const __Myt & a, __Myt & q, __Myt & r, int p) const {
            r.data_ = data_;
            q.data_.clear();
            assert(0 < p);
            __Myt t(a << (p - 1));
            t.sign_ = false;
            for (;; t >>= 1) {
                if (t <= r) {
                    r -= t;
                    toBitsReverse<1>(q.data_, p, 1);
                } else
                    --p;
                if (p < 1)
                    break;
            }
        }
        void shiftLeft(int a) {
            if (a < 0)
                throw std::invalid_argument("Invalid shift bits");
            if (!a || !*this)
                return;
            __Data r;
            for (const auto & v : data_)
                toBits<kEachBits>(r, a, v);
            data_.swap(r);
            shrink();
        }
        void shiftRight(int a) {
            if (a < 0)
                throw std::invalid_argument("Invalid shift bits");
            if (!a)
                return;
            __Data r;
            forBits<kEachBits>(data_, a, [&r](const auto & v) {r.push_back(v); });
            data_.swap(r);
            shrink();
        }
        bool equal(const __SInt & a) const { return (sign_ == (a < 0) && 0 == compare(abs(a))); }
        bool equal(const __Int & a) const { return (!sign_ && 0 == compare(a)); }
        bool equal(const std::string & a) const { return (*this == __Myt(a)); }
        bool less(const __Int & a) const { return less(false, a); }
        bool less(const __SInt & a) const { return less((a < 0), abs(a)); }
        bool less(const std::string & a) const { return (*this < __Myt(a)); }
        template<class T>
        bool less(bool s, const T & a) const {
            if (sign_ != s)
                return sign_;
            const int r = compare(a);
            return (sign_ ? (1 == r) : (-1 == r));
        }
        bool greater(const __Int & a) const { return greater(false, a); }
        bool greater(const __SInt & a) const { return greater((a < 0), abs(a)); }
        bool greater(const std::string & a) const { return (*this > __Myt(a)); }
        bool greater(bool s, const __Int & a) const {
            if (sign_ != s)
                return !sign_;
            const int r = compare(a);
            return (sign_ ? (-1 == r) : (1 == r));
        }

        void reset(bool s = false) { sign_ = s; data_.clear(); }
        void shrink() {
            eraseTailIf(data_, [](auto v) {return (0 == v); });
            if (data_.empty() && sign_)
                sign_ = false;
        }
        template<int N>
        std::string toStringX(const char * digits, const char * base) const {
            static_assert(N > 0, "N is less than 1");
            std::string ret;
            if (data_.empty())
                ret.push_back('0');
            else {
                forBits<N>(data_, 0, [&ret, &digits](const auto & i) {ret.push_back(digits[i]); });
                eraseTailIf(ret, [](auto c) {return ('0' == c); });
            }
            if (base)
                ret += base;
            if (sign_)
                ret.push_back('-');
            std::reverse(ret.begin(), ret.end());
            return std::move(ret);
        }
        std::string toString10() const {
            std::string ret;
            forBitsReverse<1>(data_, 0, [&ret](const auto & v) {
                int add = static_cast<int>(v);
                assert(0 <= add && add < 10);
                if (!ret.empty())
                    for (char & c : ret) {
                        c = (c << 1) + add;
                        if ((add = (c > 9 ? 1 : 0)))
                            c -= 10;
                    }
                if (add)
                    ret.push_back(add);
            });
            if (ret.empty())
                ret.push_back(0);
            if (sign_)
                ret.push_back('-' - '0');
            reverseAdd(ret, '0');
            return std::move(ret);
        }
        int compare(const __Int & a) const {
            if (!a)
                return (data_.empty() ? 0 : 1);
            if (data_.size() < 1)
                return -1;
            else if (data_.size() > 1)
                return 1;
            return (data_[0] < a ? -1 : (data_[0] > a ? 1 : 0));
        }
        int compare(const __Data & a) const {
            if (data_.size() < a.size())
                return -1;
            else if (data_.size() > a.size())
                return 1;
            for (int i = int(a.size() - 1); i >= 0; --i)
                if (data_[i] < a[i])
                    return -1;
                else if (data_[i] > a[i])
                    return 1;
            return 0;
        }
        void addAbs(const __Int & a) { (a ? addAbs({}, a) : (void)0); }
        void addAbs(const __Data & a, const __Int & b = 0) {
            if (!b) {
                if (a.empty())
                    return;
                if (data_.empty()) {
                    data_ = a;
                    return;
                }
            }
            __Int c = b;
            for (size_t i = 0; i < a.size() || c; ++i) {
                __Int t = c;
                c = 0;
                if (i < a.size())
                    c += plus(t, a[i]);
                if (i < data_.size())
                    c += plus(data_[i], t);
                else
                    data_.push_back(t);
            }
        }
        void subAbs(const __Int & a) { (a ? subAbs({}, a) : (void)0); }
        void subAbs(const __Data & a, const __Int & b = 0) {
            if (a.empty() && !b)
                return;
            __Int c = b;
            for (size_t i = 0; i < a.size() || c; ++i) {
                c = minus(data_[i], c);
                if (i < a.size())
                    c += minus(data_[i], a[i]);
            }
            shrink();
        }
        void subByAbs(const __Int & a) { (a ? subByAbs({}, a) : (void)0); }
        void subByAbs(const __Data & a, const __Int & b = 0) {
            if (data_.empty() && !b) {
                data_ = a;
                return;
            }
            __Data r;
            __Int c = b, d = 0;
            for (size_t i = 0; i < a.size() || c || d; ++i) {
                __Int t = c;
                c = 0;
                d = minus(t, d);
                if (i < a.size())
                    c += plus(t, a[i]);
                if (i < data_.size())
                    d += minus(t, data_[i]);
                if (c == d)
                    c = d = 0;
                r.push_back(t);
            }
            r.swap(data_);
            shrink();
        }
        bool mulSign(bool s, bool zero, bool one) {
            if (!*this)
                return true;
            if (zero) {
                reset();
                return true;
            }
            sign_ = (sign_ != s);
            return one;
        }
        static __Int abs(const __SInt & a) { return (a < 0 ? -a : a); }
        static int plus(__Int & a, const __Int & b) {
            const auto t(a);
            a += b;
            return (a < t || a < b ? 1 : 0);
        }
        static int minus(__Int & a, const __Int & b) {
            const int r = (a < b ? 1 : 0);
            a -= b;
            return r;
        }
        static int topBit(const __Data & a) {
            if (a.empty())
                return 0;
            int i = kEachBits - 1;
            for (; i >= 0 && !(a.back() & (__Int(1) << i)); --i);
            return int(i + kEachBits * (a.size() - 1));
        }
        template<class T, class F>
        static void eraseTailIf(T & c, F && f) {
            auto it = c.rbegin();
            if (it != c.rend() && f(*it)) {
                for (++it; it != c.rend() && f(*it); ++it);
                c.erase(it.base(), c.end());
            }
        }
        static constexpr __Int mask(int bits) { return (bits < 1 ? 0 : (bits >= kEachBits ? __Int(-1) : ((__Int(1) << bits) - 1))); }
        static constexpr __Int getBits(const __Int & val, int from, int bits) { return ((val >> from) & mask(bits)); }
        template<int N>
        static __Int getBits(const __Data & data, int from) {
            static_assert(0 < N && N <= kEachBits, "read invlaid bits");
            const int fi = from / kEachBits, ri = from % kEachBits;
            assert(size_t(fi) < data.size());
            __Int val = getBits(data[fi], ri, N);
            if (kEachBits - N < ri && size_t(fi + 1) < data.size()) {
                const int s1 = kEachBits - ri, s2 = N - s1;
                setBits(val, s1, s2, getBits(data[fi + 1], 0, s2));
            }
            return val;
        }
        static void setBits(__Int & ret, int from, int bits, const __Int & val) {
            assert(0 <= from && from < kEachBits);
            const __Int m = mask(bits);
            ret &= ~(m << from);
            ret += (val & m) << from;
        }
        template<int N>
        static void setBits(__Int & ret, int from, const __Int & val) {
            assert(0 <= from && from < kEachBits);
            constexpr __Int m = mask(N);
            ret &= ~(m << from);
            ret += (val & m) << from;
        }
        template<int N>
        static void setBits(__Data & data, int from, const __Int & val) {
            static_assert(0 < N && N <= kEachBits, "write invalid bits");
            const int fi = from / kEachBits, ri = from % kEachBits;
            assert(size_t(fi) < data.size());
            setBits<N>(data[fi], ri, val);
            if (kEachBits - N < ri && size_t(fi + 1) < data.size()) {
                const int s1 = kEachBits - ri, s2 = N - s1;
                setBits(data[fi + 1], 0, s2, getBits(val, s1, s2));
            }
        }
        template<int N, class F>
        static void forBits(const __Data & data, int from, F && func) {
            for (const int kTotalBits = int(kEachBits * data.size()); from < kTotalBits; from += N)
                func(getBits<N>(data, from));
        }
        template<int N, class F>
        static void forBitsReverse(const __Data & data, int from, F && func) {
            for (const int kTotalBits = int(kEachBits * data.size()); from < kTotalBits; from += N)
                func(getBits<N>(data, kTotalBits - N - from));
        }
        template<int N>
        static void toBits(__Data & data, int & from, const __Int & val) {
            const int kTotalBits = int(kEachBits * data.size());
            if (kTotalBits < from + N)
                data.resize((from + N + kEachBits - 1) / kEachBits);
            setBits<N>(data, from, val);
            from += N;
        }
        template<int N, class F>
        static void toBitsF(__Data & data, int & from, F && func) { toBits<N>(data, from, func()); }
        template<int N>
        static void toBitsReverse(__Data & data, int & from, const __Int & val) {
            assert(from >= N);
            const int kTotalBits = int(kEachBits * data.size());
            if (kTotalBits < from)
                data.resize((from + kEachBits - 1) / kEachBits);
            setBits<N>(data, from - N, val);
            from -= N;
        }
        static void reverseAdd(std::string & s, int v) {
            for (int i = 0, j = int(s.size() - 1); i <= j; ++i, --j) {
                const auto t = s[i];
                s[i] = s[j] + v;
                if (i < j)
                    s[j] = t + v;
            }
        }
        //return:
        //  0       empty string
        //  2       base is 2
        //  3       for "0", "-0", "+0", base is 10
        //  8       base is 8
        //  10      base is 10
        //  16      base is 16
        //  others  error
        static int checkBase(const std::string & a) {
            int r = 0;
            for (auto c : a) {
                switch (r) {
                    case 0:
                        if ('+' == c || '-' == c)
                            break;
                    case 1:
                        if ('0' == c) {
                            r = 3;
                            break;
                        }
                    case 10:r = ('0' <= c && c <= '9' ? 10 : -1); break;
                    case 3:
                        if ('b' == c || 'B' == c) {
                            r = 5;
                            break;
                        } else if ('x' == c || 'X' == c) {
                            r = 7;
                            break;
                        }
                    case 8:r = ('0' <= c && c < '8' ? 8 : -1); break;
                    case 2:
                    case 5:r = ('0' == c || '1' == c ? 2 : -1); break;
                    case 7:
                    case 16:r = (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') ? 16 : -1); break;
                }
                if (r < 0)
                    break;
            }
            return r;
        }
        //fields
        __Data data_;
        bool sign_ = false;
    };

    //explicit specializations for member class template
    template<>struct HugeNumber::__SupportType<std::string> { typedef const std::string & type; };
    template<>struct HugeNumber::__SupportType<const char *> { typedef const std::string & type; };
    template<>struct HugeNumber::__SupportType<char *> { typedef const std::string & type; };
    //template<size_t N>struct HugeNumber::__SupportType<const char [N]>{typedef const std::string & type;};
    template<size_t N>struct HugeNumber::__SupportType<char[N]> { typedef const std::string & type; };
#define __SUPPORT_INTEGER(tp)   \
    template<>struct HugeNumber::__SupportType<tp>{ \
        typedef const __TypeSelect<std::numeric_limits<tp>::is_signed, __SInt, __Int>::type & type; \
    }
    __SUPPORT_INTEGER(char);
    __SUPPORT_INTEGER(wchar_t);
    __SUPPORT_INTEGER(char16_t);
    __SUPPORT_INTEGER(char32_t);
    __SUPPORT_INTEGER(signed char);
    __SUPPORT_INTEGER(unsigned char);
    __SUPPORT_INTEGER(short);
    __SUPPORT_INTEGER(unsigned short);
    __SUPPORT_INTEGER(int);
    __SUPPORT_INTEGER(unsigned int);
    __SUPPORT_INTEGER(long);
    __SUPPORT_INTEGER(unsigned long);
    __SUPPORT_INTEGER(long long);
    __SUPPORT_INTEGER(unsigned long long);
#undef __SUPPORT_INTEGER

    //swap(a, b);
    inline void swap(HugeNumber & a, HugeNumber & b) noexcept {
        a.swap(b);
    }
} // namespace dozerg

#endif