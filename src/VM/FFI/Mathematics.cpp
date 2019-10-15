#include <Ark/VM/FFI.hpp>

#include <cmath>
#include <limits>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::Mathematics
{
    extern const Value pi_ = Value(M_PI);
    extern const Value e_ = Value(std::exp(1.0));
    extern const Value tau_ = Value(M_PI * 2.0);
    extern const Value inf_ = Value(HUGE_VAL);
    extern const Value nan_ = Value(std::numeric_limits<double>::signaling_NaN());

    FFI_Function(exponential)
    {
        if (n.size() != 1)
            throw std::runtime_error("exp can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of exp must be of type Number");
        
        Value r(std::exp(n[0].number()));
        return r;
    }

    FFI_Function(logarithm)
    {
        if (n.size() != 1)
            throw std::runtime_error("log can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of log must be of type Number");
        if (n[0].number() <= 0.0)
            throw std::runtime_error("Argument of log must be greater than 0");
        
        Value r(std::log(n[0].number()));
        return r;
    }

    FFI_Function(ceil_)
    {
        if (n.size() != 1)
            throw std::runtime_error("ceil can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of ceil must be of type Number");
        
        Value r(std::ceil(n[0].number()));
        return r;
    }

    FFI_Function(floor_)
    {
        if (n.size() != 1)
            throw std::runtime_error("floor can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of floor must be of type Number");
        
        Value r(std::floor(n[0].number()));
        return r;
    }

    FFI_Function(round_)
    {
        if (n.size() != 1)
            throw std::runtime_error("round can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of round must be of type Number");
        
        Value r(std::round(n[0].number()));
        return r;
    }

    FFI_Function(isnan_)
    {
        if (n.size() != 1)
            throw std::runtime_error("isNaN can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of isNaN must be of type Number");
        
        Value r(std::isnan(n[0].number()));
        return r;
    }

    FFI_Function(isinf_)
    {
        if (n.size() != 1)
            throw std::runtime_error("isInf can take only one argument, a number");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of isInf must be of type Number");
        
        Value r(std::isinf(n[0].number()));
        return r;
    }
}