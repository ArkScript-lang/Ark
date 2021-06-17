#define _USE_MATH_DEFINES
#include <cmath>

#include <Ark/Builtins/Builtins.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Mathematics
{
    Value exponential(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:exp"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:exp"));
        
        Value r(std::exp(n[0].number()));
        return r;
    }

    Value logarithm(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:log"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:log"));
        if (n[0].number() <= 0.0)
            throw std::runtime_error("Argument of math:log must be greater than 0");
        
        Value r(std::log(n[0].number()));
        return r;
    }

    Value ceil_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:ceil"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:ceil"));
        
        Value r(std::ceil(n[0].number()));
        return r;
    }

    Value floor_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:floor"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:floor"));
        
        Value r(std::floor(n[0].number()));
        return r;
    }

    Value round_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:round"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:round"));
        
        Value r(std::round(n[0].number()));
        return r;
    }

    Value isnan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:NaN?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;
        
        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    Value isinf_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:Inf?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;
        
        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    Value cos_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:cos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:cos"));
        
        Value r(std::cos(n[0].number()));
        return r;
    }

    Value sin_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:sin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:sin"));
        
        Value r(std::sin(n[0].number()));
        return r;
    }

    Value tan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:tan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:tan"));
        
        Value r(std::tan(n[0].number()));
        return r;
    }

    Value acos_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arccos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arccos"));
        
        Value r(std::acos(n[0].number()));
        return r;
    }

    Value asin_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arcsin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arcsin"));
        
        Value r(std::asin(n[0].number()));
        return r;
    }

    Value atan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arctan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arctan"));
        
        Value r(std::atan(n[0].number()));
        return r;
    }

    Value cosh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:cosh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:cosh"));
        
        Value r(std::cosh(n[0].number()));
        return r;
    }

    Value sinh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:sinh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:sinh"));
        
        Value r(std::sinh(n[0].number()));
        return r;
    }

    Value tanh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:tanh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:tanh"));
        
        Value r(std::tanh(n[0].number()));
        return r;
    }

    Value acosh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:acosh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:acosh"));
        
        Value r(std::acosh(n[0].number()));
        return r;
    }

    Value asinh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:asinh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:asinh"));
        
        Value r(std::asinh(n[0].number()));
        return r;
    }

    Value atanh_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:atanh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:atanh"));
        
        Value r(std::atanh(n[0].number()));
        return r;
    }
}