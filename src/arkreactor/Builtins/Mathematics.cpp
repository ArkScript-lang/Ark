#define _USE_MATH_DEFINES
#include <cmath>
#include <fmt/core.h>
#include <random>

#include <Ark/Builtins/Builtins.hpp>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Mathematics
{
    /**
     * @name math:exp
     * @brief Calculate e^number
     * @param value the Number
     * =begin
     * (math:exp 1)  # 2.7182...
     * =end
     * @author https://github.com/SuperFola
     */
    Value exponential(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:exp",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::exp(n[0].number()));
    }

    /**
     * @name math:ln
     * @brief Calculate the logarithm of a number
     * @param value the Number
     * =begin
     * (math:ln 1)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value logarithm(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:ln",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        if (n[0].number() <= 0.0)
            throw std::runtime_error(fmt::format("math:ln: value {} must be greater than 0", n[0].number()));

        return Value(std::log(n[0].number()));
    }

    /**
     * @name math:ceil
     * @brief Get the smallest possible integer greater than the number
     * @param value the Number
     * =begin
     * (math:ceil 0.2)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value ceil_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:ceil",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::ceil(n[0].number()));
    }

    /**
     * @name math:floor
     * @brief Get the smallest possible integer equal to the given number
     * @param value the Number
     * =begin
     * (math:floor 1.7)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value floor_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:floor",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::floor(n[0].number()));
    }

    /**
     * @name math:round
     * @brief Get the smallest possible integer equal to or greater than the given number
     * @param value the Number
     * =begin
     * (math:round 0.2)  # 0
     * (math:round 0.6)  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value round_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:round",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::round(n[0].number()));
    }

    /**
     * @name math:NaN?
     * @brief Check if a Number is NaN
     * @param value the Number
     * =begin
     * (math:NaN? 2)  # false
     * (math:NaN? nan)  # true
     * =end
     * @author https://github.com/SuperFola
     */
    Value isnan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Any))
            throw types::TypeCheckingError(
                "math:NaN?",
                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                n);

        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    /**
     * @name math:Inf?
     * @brief Check if a Number if Inf
     * @param value the Number
     * =begin
     * (math:Inf? 1)  # false
     * (math:Inf? nan)  # false
     * =end
     * @author https://github.com/SuperFola
     */
    Value isinf_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Any))
            throw types::TypeCheckingError(
                "math:Inf?",
                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                n);

        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    /**
     * @name math:cos
     * @brief Calculate the cosinus of a number
     * @param value the Number (radians)
     * =begin
     * (math:cos 0)  # 1
     * (math:cos math:pi)  # -1
     * =end
     * @author https://github.com/SuperFola
     */
    Value cos_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:cos",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::cos(n[0].number()));
    }

    /**
     * @name math:sin
     * @brief Calculate the sinus of a number
     * @param value the Number (radians)
     * =begin
     * (math:sin 0)  # 0
     * (math:cos (/ math:pi 2))  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value sin_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:sin",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::sin(n[0].number()));
    }

    /**
     * @name math:tan
     * @brief Calculate the tangent of a number
     * @param value the Number (radians)
     * =begin
     * (math:tan 0)  # 0
     * (math:cos (/ math:pi 4))  # 1
     * =end
     * @author https://github.com/SuperFola
     */
    Value tan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:tan",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::tan(n[0].number()));
    }

    /**
     * @name math:arccos
     * @brief Calculate the arc cosinus of a number
     * @param value the Number
     * =begin
     * (math:arccos 1)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value acos_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arccos",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::acos(n[0].number()));
    }

    /**
     * @name math:arcsin
     * @brief Calculate the arc sinus of a number
     * @param value the Number
     * =begin
     * (math:arcsin 1)  # 1.570796326794897 (/ math:pi 2)
     * =end
     * @author https://github.com/SuperFola
     */
    Value asin_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arcsin",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::asin(n[0].number()));
    }

    /**
     * @name math:arctan
     * @brief Calculate the arc tangent of a number
     * @param value the Number
     * =begin
     * (math:arctan 0)  # 0
     * =end
     * @author https://github.com/SuperFola
     */
    Value atan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arctan",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::atan(n[0].number()));
    }

    /**
     * @name math:cosh
     * @brief Calculate the hyperbolic cosinus of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value cosh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:cosh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::cosh(n[0].number()));
    }

    /**
     * @name math:sinh
     * @brief Calculate the hyperbolic sinus of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value sinh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:sinh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::sinh(n[0].number()));
    }

    /**
     * @name math:tanh
     * @brief Calculate the hyperbolic tangent of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value tanh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:tanh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::tanh(n[0].number()));
    }

    /**
     * @name math:acosh
     * @brief Calculate the hyperbolic arc cosinus of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value acosh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:acosh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::acosh(n[0].number()));
    }

    /**
     * @name math:asinh
     * @brief Calculate the hyperbolic arc sinus of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value asinh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:asinh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::asinh(n[0].number()));
    }

    /**
     * @name math:atanh
     * @brief Calculate the hyperbolic arc tangent of a number
     * @param value the Number
     * @author https://github.com/Gryfenfer97
     */
    Value atanh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:atanh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::atanh(n[0].number()));
    }

    /**
     * @name random
     * @brief Compute a random number in [-2147483648, 2147483647] or in a custom range passed to the function
     * @param min optional inclusive lower bound
     * @param max optional inclusive upper bound. Must be present if `min` is passed
     * =begin
     * (print (random))  # a number in [-2147483648, 2147483647]
     * (print (random 0 10))  # a number between 0 and 10
     * =end
     * @author https://github.com/SuperFola
     */
    Value random(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        static std::mt19937 gen { std::random_device()() };

        if (n.size() == 2 && !types::check(n, ValueType::Number, ValueType::Number))
            throw types::TypeCheckingError(
                "random",
                { { types::Contract {
                    { types::Typedef("min", ValueType::Number), types::Typedef("max", ValueType::Number) } } } },
                n);

        if (n.size() == 2)
        {
            const auto inclusive_min = static_cast<int>(n[0].number()),
                       inclusive_max = static_cast<int>(n[1].number());

            std::uniform_int_distribution<> distrib(inclusive_min, inclusive_max);
            return Value(distrib(gen));
        }

        const auto x = static_cast<int>(gen());
        return Value(x);
    }
}
