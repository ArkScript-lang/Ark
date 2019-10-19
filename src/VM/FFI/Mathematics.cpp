#define _USE_MATH_DEFINES
#include <cmath>

#include <Ark/VM/FFI.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI::Mathematics
{
    FFI_Function(exponential)
    {
        if (n.size() != 1)
            throw std::runtime_error("exp can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of exp must be of type Number");
        
        Value r(std::exp(n[0].number()));
        return r;
    }

    FFI_Function(logarithm)
    {
        if (n.size() != 1)
            throw std::runtime_error("log can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of log must be of type Number");
        if (n[0].number() <= 0.0)
            throw std::runtime_error("Argument of log must be greater than 0");
        
        Value r(std::log(n[0].number()));
        return r;
    }

    FFI_Function(ceil_)
    {
        if (n.size() != 1)
            throw std::runtime_error("ceil can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of ceil must be of type Number");
        
        Value r(std::ceil(n[0].number()));
        return r;
    }

    FFI_Function(floor_)
    {
        if (n.size() != 1)
            throw std::runtime_error("floor can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of floor must be of type Number");
        
        Value r(std::floor(n[0].number()));
        return r;
    }

    FFI_Function(round_)
    {
        if (n.size() != 1)
            throw std::runtime_error("round can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of round must be of type Number");
        
        Value r(std::round(n[0].number()));
        return r;
    }

    FFI_Function(isnan_)
    {
        if (n.size() != 1)
            throw std::runtime_error("isNaN can take only one argument, a value");
        if (n[0].valueType() != ValueType::Number)
            return false;
        
        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    FFI_Function(isinf_)
    {
        if (n.size() != 1)
            throw std::runtime_error("isInf can take only one argument, a value");
        if (n[0].valueType() != ValueType::Number)
            return false;
        
        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    FFI_Function(cos_)
    {
        if (n.size() != 1)
            throw std::runtime_error("cos can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of cos must be of type Number");
        
        Value r(std::cos(n[0].number()));
        return r;
    }

    FFI_Function(sin_)
    {
        if (n.size() != 1)
            throw std::runtime_error("sin can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of sin must be of type Number");
        
        Value r(std::sin(n[0].number()));
        return r;
    }

    FFI_Function(tan_)
    {
        if (n.size() != 1)
            throw std::runtime_error("tan can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of tan must be of type Number");
        
        Value r(std::tan(n[0].number()));
        return r;
    }

    FFI_Function(acos_)
    {
        if (n.size() != 1)
            throw std::runtime_error("arccos can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of arccos must be of type Number");
        
        Value r(std::acos(n[0].number()));
        return r;
    }

    FFI_Function(asin_)
    {
        if (n.size() != 1)
            throw std::runtime_error("arcsin can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of arcsin must be of type Number");
        
        Value r(std::asin(n[0].number()));
        return r;
    }

    FFI_Function(atan_)
    {
        if (n.size() != 1)
            throw std::runtime_error("arctan can take only one argument, a Number");
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError("Argument of arctan must be of type Number");
        
        Value r(std::atan(n[0].number()));
        return r;
    }
}