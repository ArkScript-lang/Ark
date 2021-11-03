#define _USE_MATH_DEFINES
#include <cmath>

#include <Ark/Builtins/Builtins.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Mathematics
{
    /**
     * @name math:exp
     * @brief Calculate e^number
     * @param n the Number
     * =begin
     * (math:exp 1)  # 2.7182...
     * =end
     * @author https://github.com/SuperFola
     */
    Value exponential(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:exp"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:exp"));

        Value r(std::exp(n[0].number()));
        return r;
    }

    /**
     * @name math:ln
     * @brief Calculate the logarithm of a number
     * @param n the Number
     * =begin
     * (math:ln 1)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value logarithm(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
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

    /**
     * @name math:ceil
     * @brief Get the smallest possible integer greater than the number
     * @param n the Number
     * =begin
     * (math:ceil 0.2)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value ceil_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:ceil"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:ceil"));

        Value r(std::ceil(n[0].number()));
        return r;
    }

    /**
     * @name math:floor
     * @brief Get the smallest possible integer equal to the given number
     * @param n the Number
     * =begin
     * (math:floor 1.7)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value floor_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:floor"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:floor"));

        Value r(std::floor(n[0].number()));
        return r;
    }

    /**
     * @name math:round
     * @brief Get the smallest possible integer equal to or greater than the given number
     * @param n the Number
     * =begin
     * (math:round 0.2)  # 0
     * (math:round 0.6)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value round_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:round"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:round"));

        Value r(std::round(n[0].number()));
        return r;
    }

    /**
     * @name math:NaN?
     * @brief Check if a Number is NaN
     * @param n the Number
     * =begin
     * (math:NaN? 2)  # false
     * (math:NaN? nan)  # true
     * =end
     * @author https://github.com/SuperFola
     */
    Value isnan_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:NaN?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    /**
     * @name math:Inf?
     * @brief Check if a Number if Inf
     * @param n the Number
     * =begin
     * (math:Inf? 1)  # false
     * (math:Inf? nan)  # false
     * =end
     * @author https://github.com/SuperFola
     */
    Value isinf_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:Inf?"));
        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    /**
     * @name math:cos
     * @brief Calculate the cosinus of a number
     * @param n the Number (radians)
     * =begin
     * (math:cos 0)  # 1
     * (math:cos math:pi)  # -1
     * =end
     * @author https://github.com/SuperFola
     */
    Value cos_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:cos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:cos"));

        Value r(std::cos(n[0].number()));
        return r;
    }

    /**
     * @name math:sin
     * @brief Calculate the sinus of a number
     * @param n the Number (radians)
     * =begin
     * (math:sin 0)  # 0
     * (math:cos (/ math:pi 2))  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value sin_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:sin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:sin"));

        Value r(std::sin(n[0].number()));
        return r;
    }

    /**
     * @name math:tan
     * @brief Calculate the tangent of a number
     * @param n the Number (radians)
     * =begin
     * (math:tan 0)  # 0
     * (math:cos (/ math:pi 4))  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value tan_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:tan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:tan"));

        Value r(std::tan(n[0].number()));
        return r;
    }

    /**
     * @name math:arccos
     * @brief Calculate the arccosinus of a number
     * @param n the Number
     * =begin
     * (math:arccos 1)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value acos_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arccos"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arccos"));

        Value r(std::acos(n[0].number()));
        return r;
    }

    /**
     * @name math:arcsin
     * @brief Calculate the arcsinus of a number
     * @param n the Number
     * =begin
     * (math:arcsin 1)  # 1.570796326794897 (/ math:pi 2)
     * =end
     * @author https://github.com/SuperFola
     */
    Value asin_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arcsin"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arcsin"));

        Value r(std::asin(n[0].number()));
        return r;
    }

    /**
     * @name math:arctan
     * @brief Calculate the arctangent of a number
     * @param n the Number
     * =begin
     * (math:arctan 0)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value atan_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:arctan"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:arctan"));

        Value r(std::atan(n[0].number()));
        return r;
    }

    /**
     * @name math:cosh
     * @brief Calculate the hyperbolic cosinus of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value cosh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:cosh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:cosh"));

        Value r(std::cosh(n[0].number()));
        return r;
    }

    /**
     * @name math:sinh
     * @brief Calculate the hyperbolic sinus of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value sinh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:sinh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:sinh"));

        Value r(std::sinh(n[0].number()));
        return r;
    }

    /**
     * @name math:tanh
     * @brief Calculate the hyperbolic tangent of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value tanh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:tanh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:tanh"));

        Value r(std::tanh(n[0].number()));
        return r;
    }

    /**
     * @name math:acosh
     * @brief Calculate the hyperbolic arccosinus of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value acosh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:acosh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:acosh"));

        Value r(std::acosh(n[0].number()));
        return r;
    }

    /**
     * @name math:asinh
     * @brief Calculate the hyperbolic arcsinus of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value asinh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:asinh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:asinh"));

        Value r(std::asinh(n[0].number()));
        return r;
    }

    /**
     * @name math:atanh
     * @brief Calculate the hyperbolic arctangent of a number
     * @param n the Number
     * @author https://github.com/Gryfenfer97
     */
    Value atanh_(std::vector<Value>& n, Ark::VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw std::runtime_error(MATH_ARITY("math:atanh"));
        if (n[0].valueType() != ValueType::Number)
            throw Ark::TypeError(MATH_TE0("math:atanh"));

        Value r(std::atanh(n[0].number()));
        return r;
    }
}
