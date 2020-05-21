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
            throw std::runtime_error(MATH_ARITY("exp"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("exp"));
        
        Value r(std::exp(n[0].number()));
        return r;
    }

    Value logarithm(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("log"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("log"));
        if (n[0].number() <= 0.0)
            throw std::runtime_error("Argument of log must be greater than 0");
        
        Value r(std::log(n[0].number()));
        return r;
    }

    Value ceil_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("ceil"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("ceil"));
        
        Value r(std::ceil(n[0].number()));
        return r;
    }

    Value floor_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("floor"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("floor"));
        
        Value r(std::floor(n[0].number()));
        return r;
    }

    Value round_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("round"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("round"));
        
        Value r(std::round(n[0].number()));
        return r;
    }

    Value isnan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("NaN?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;
        
        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    Value isinf_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("Inf?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;
        
        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    Value cos_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("cos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("cos"));
        
        Value r(std::cos(n[0].number()));
        return r;
    }

    Value sin_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("sin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("sin"));
        
        Value r(std::sin(n[0].number()));
        return r;
    }

    Value tan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("tan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("tan"));
        
        Value r(std::tan(n[0].number()));
        return r;
    }

    Value acos_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("arccos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("arccos"));
        
        Value r(std::acos(n[0].number()));
        return r;
    }

    Value asin_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("arcsin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("arcsin"));
        
        Value r(std::asin(n[0].number()));
        return r;
    }

    Value atan_(std::vector<Value>& n, Ark::VM* vm)
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("arctan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("arctan"));
        
        Value r(std::atan(n[0].number()));
        return r;
    }
}