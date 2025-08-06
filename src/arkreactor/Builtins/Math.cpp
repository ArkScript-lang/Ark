#define _USE_MATH_DEFINES
#include <cmath>
#include <fmt/core.h>
#include <random>

#include <Ark/Builtins/Builtins.hpp>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::Mathematics
{
    // cppcheck-suppress constParameterReference
    Value exponential(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:exp",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::exp(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
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

    // cppcheck-suppress constParameterReference
    Value ceil_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:ceil",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::ceil(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value floor_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:floor",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::floor(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value round_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:round",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::round(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value isnan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isnan(n[0].number()) ? trueSym : falseSym;
    }

    // cppcheck-suppress constParameterReference
    Value isinf_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n[0].valueType() != ValueType::Number)
            return falseSym;

        return std::isinf(n[0].number()) ? trueSym : falseSym;
    }

    // cppcheck-suppress constParameterReference
    Value cos_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:cos",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::cos(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value sin_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:sin",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::sin(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value tan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:tan",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::tan(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value acos_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arccos",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::acos(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value asin_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arcsin",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::asin(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value atan_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:arctan",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::atan(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value cosh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:cosh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::cosh(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value sinh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:sinh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::sinh(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value tanh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:tanh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::tanh(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value acosh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:acosh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::acosh(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
    Value asinh_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "math:asinh",
                { { types::Contract { { types::Typedef("value", ValueType::Number) } } } },
                n);

        return Value(std::asinh(n[0].number()));
    }

    // cppcheck-suppress constParameterReference
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
    // cppcheck-suppress constParameterReference
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
