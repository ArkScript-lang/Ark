#include <Ark/Builtins/Builtins.hpp>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/DefaultValues.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/Value/Dict.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/VM/Helpers.hpp>

namespace Ark::internal::Builtins::Operators
{
    // cppcheck-suppress constParameterReference
    Value add(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.empty())
            throw types::TypeCheckingError(
                "+",
                { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } },
                    types::Contract { { types::Typedef("a", ValueType::String, /* is_variadic= */ true) } } } },
                n);

        if (n[0].valueType() == ValueType::Number)
        {
            double output = 0.0;
            for (const Value& num : n)
            {
                if (num.valueType() != ValueType::Number)
                    throw types::TypeCheckingError(
                        "+",
                        { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } },
                            types::Contract { { types::Typedef("a", ValueType::String, /* is_variadic= */ true) } } } },
                        n);
                output += num.number();
            }
            return Value(output);
        }

        std::string output;
        for (const Value& str : n)
        {
            if (str.valueType() != ValueType::String)
                throw types::TypeCheckingError(
                    "+",
                    { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } },
                        types::Contract { { types::Typedef("a", ValueType::String, /* is_variadic= */ true) } } } },
                    n);
            output += str.string();
        }
        return Value(output);
    }

    // cppcheck-suppress constParameterReference
    Value sub(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.empty() || n[0].valueType() != ValueType::Number)
            throw types::TypeCheckingError(
                "-",
                { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);

        double output = n[0].number();
        for (const Value& num : n | std::ranges::views::drop(1))
        {
            if (num.valueType() != ValueType::Number)
                throw types::TypeCheckingError(
                    "-",
                    { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);
            output -= num.number();
        }

        return Value(output);
    }

    // cppcheck-suppress constParameterReference
    Value mul(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() < 2 || n[0].valueType() != ValueType::Number || n[1].valueType() != ValueType::Number)
            throw types::TypeCheckingError(
                "*",
                { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);

        double output = n[0].number();
        for (const Value& num : n | std::ranges::views::drop(1))
        {
            if (num.valueType() != ValueType::Number)
                throw types::TypeCheckingError(
                    "*",
                    { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);
            output *= num.number();
        }

        return Value(output);
    }

    // cppcheck-suppress constParameterReference
    Value div(std::vector<Value>& n, VM* vm)
    {
        if (n.size() < 2 || n[0].valueType() != ValueType::Number || n[1].valueType() != ValueType::Number)
            throw types::TypeCheckingError(
                "/",
                { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);

        double output = n[0].number();
        for (const Value& num : n | std::ranges::views::drop(1))
        {
            if (num.valueType() != ValueType::Number)
                throw types::TypeCheckingError(
                    "/",
                    { { types::Contract { { types::Typedef("a", ValueType::Number, /* is_variadic= */ true) } } } }, n);

            if (num.number() == 0)
                VM::throwVMError(ErrorKind::DivisionByZero, fmt::format("Can not compute expression (/ {} {})", n[0].toString(*vm), num.toString(*vm)));

            output /= num.number();
        }

        return Value(output);
    }

    // cppcheck-suppress constParameterReference
    Value mod(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number, ValueType::Number))
            throw types::TypeCheckingError(
                "mod",
                { { types::Contract { { types::Typedef("a", ValueType::Number), types::Typedef("b", ValueType::Number) } } } },
                n);

        return Value(std::fmod(n[0].number(), n[1].number()));
    }

    // cppcheck-suppress constParameterReference
    Value toNumber(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "toNumber",
                { { types::Contract { { types::Typedef("val", ValueType::String) } } } },
                n);

        double val;
        if (Utils::isDouble(n[0].string(), &val))
            return Value(val);
        return Nil;
    }

    // cppcheck-suppress constParameterReference
    Value toString(std::vector<Value>& n, VM* vm)
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "toString",
                { { types::Contract { { types::Typedef("val", ValueType::Any) } } } },
                n);

        return Value(n[0].toString(*vm));
    }

    // cppcheck-suppress constParameterReference
    Value lessThan(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                "<",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return n[0] < n[1] ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value lessOrEq(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                "<=",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return (n[0] < n[1] || n[0] == n[1]) ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value greaterThan(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                ">",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return n[1] < n[0] ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value greaterOrEq(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                ">=",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return (n[1] < n[0] || n[0] == n[1]) ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value eq(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                "=",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return n[0] == n[1] ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value notEq(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                "!=",
                { { types::Contract { { types::Typedef("lhs", ValueType::Any), types::Typedef("rhs", ValueType::Any) } } } },
                n);

        return n[0] != n[1] ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value not_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "not",
                { { types::Contract { { types::Typedef("val", ValueType::Any) } } } },
                n);

        return !n[0] ? True : False;
    }

    // cppcheck-suppress constParameterReference
    Value len(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "len",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } },
                    types::Contract { { types::Typedef("value", ValueType::Dict) } } } },
                n);

        if (n[0].valueType() == ValueType::List)
            return Value(static_cast<int>(n[0].constList().size()));
        if (n[0].valueType() == ValueType::String)
            return Value(static_cast<int>(n[0].string().size()));
        if (n[0].valueType() == ValueType::Dict)
            return Value(static_cast<int>(n[0].dict().size()));

        throw types::TypeCheckingError(
            "len",
            { { types::Contract { { types::Typedef("value", ValueType::List) } },
                types::Contract { { types::Typedef("value", ValueType::String) } },
                types::Contract { { types::Typedef("value", ValueType::Dict) } } } },
            n);
    }

    // cppcheck-suppress constParameterReference
    Value isEmpty(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "empty?",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::Nil) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } },
                    types::Contract { { types::Typedef("value", ValueType::Dict) } } } },
                n);

        if (n[0].valueType() == ValueType::List)
            return n[0].constList().empty() ? True : False;
        if (n[0].valueType() == ValueType::String)
            return n[0].string().empty() ? True : False;
        if (n[0].valueType() == ValueType::Dict)
            return std::cmp_equal(n[0].dict().size(), 0) ? True : False;
        if (n[0].valueType() == ValueType::Nil)
            return True;

        throw types::TypeCheckingError(
            "empty?",
            { { types::Contract { { types::Typedef("value", ValueType::List) } },
                types::Contract { { types::Typedef("value", ValueType::Nil) } },
                types::Contract { { types::Typedef("value", ValueType::String) } },
                types::Contract { { types::Typedef("value", ValueType::Dict) } } } },
            n);
    }

    // cppcheck-suppress constParameterReference
    Value isNil(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "nil?",
                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                n);

        if (n[0] == Nil)
            return True;
        return False;
    }

    // cppcheck-suppress constParameterReference
    Value tail(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "tail",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                n);

        return helper::tail(&n[0]);
    }

    // cppcheck-suppress constParameterReference
    Value head(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "head",
                { { types::Contract { { types::Typedef("value", ValueType::List) } },
                    types::Contract { { types::Typedef("value", ValueType::String) } } } },
                n);

        return helper::head(&n[0]);
    }

    // cppcheck-suppress constParameterReference
    Value at(std::vector<Value>& n, VM* vm)
    {
        if (n.size() != 2)
            throw types::TypeCheckingError(
                "@",
                { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                    types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                n);

        return helper::at(n[0], n[1], *vm);
    }

    // cppcheck-suppress constParameterReference
    Value atAt(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::List, ValueType::Number, ValueType::Number))
            throw types::TypeCheckingError(
                "@@",
                { { types::Contract {
                    { types::Typedef("src", ValueType::List),
                      types::Typedef("y", ValueType::Number),
                      types::Typedef("x", ValueType::Number) } } } },
                n);

        return helper::atAt(&n[2], &n[1], n[0]);
    }

    // cppcheck-suppress constParameterReference
    Value type(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1)
            throw types::TypeCheckingError(
                "type",
                { { types::Contract { { types::Typedef("value", ValueType::Any) } } } },
                n);

        return Value(std::to_string(n[0].valueType()));
    }
}
