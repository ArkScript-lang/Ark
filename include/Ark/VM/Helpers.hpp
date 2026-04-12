/**
 * @file Helpers.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Helpers for the VM
 * @date 2026-02-11
 *
 * @copyright Copyright (c) 2026-02-11
 *
 */

#ifndef ARK_VM_HELPERS_HPP
#define ARK_VM_HELPERS_HPP

#include <Ark/Builtins/Builtins.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/VM/Value/Value.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::helper
{
    using namespace internal;

    inline ARK_ALWAYS_INLINE Value tail(Value* a)
    {
        if (a->valueType() == ValueType::List)
        {
            if (a->constList().size() < 2)
                return Value(ValueType::List);

            std::vector<Value> tmp(a->constList().size() - 1);
            for (std::size_t i = 1, end = a->constList().size(); i < end; ++i)
                tmp[i - 1] = a->constList()[i];
            return Value(std::move(tmp));
        }
        if (a->valueType() == ValueType::String)
        {
            if (a->string().size() < 2)
                return Value(ValueType::String);

            Value b { *a };
            b.stringRef().erase(b.stringRef().begin());
            return b;
        }

        throw types::TypeCheckingError(
            "tail",
            { { types::Contract { { types::Typedef("value", ValueType::List) } },
                types::Contract { { types::Typedef("value", ValueType::String) } } } },
            { *a });
    }

    inline ARK_ALWAYS_INLINE Value head(Value* a)
    {
        if (a->valueType() == ValueType::List)
        {
            if (a->constList().empty())
                return Builtins::nil;
            return a->constList()[0];
        }
        if (a->valueType() == ValueType::String)
        {
            if (a->string().empty())
                return Value(ValueType::String);
            return Value(std::string(1, a->stringRef()[0]));
        }

        throw types::TypeCheckingError(
            "head",
            { { types::Contract { { types::Typedef("value", ValueType::List) } },
                types::Contract { { types::Typedef("value", ValueType::String) } } } },
            { *a });
    }

    inline ARK_ALWAYS_INLINE Value at(Value& container, Value& index, VM& vm)
    {
        if (index.valueType() != ValueType::Number)
            throw types::TypeCheckingError(
                "@",
                { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                    types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                { container, index });

        const auto num = static_cast<long>(index.number());

        if (container.valueType() == ValueType::List)
        {
            const auto i = static_cast<std::size_t>(num < 0 ? static_cast<long>(container.list().size()) + num : num);
            if (i < container.list().size())
                return container.list()[i];
            else
                VM::throwVMError(
                    ErrorKind::Index,
                    fmt::format("{} out of range {} (length {})", num, container.toString(vm), container.list().size()));
        }
        else if (container.valueType() == ValueType::String)
        {
            const auto i = static_cast<std::size_t>(num < 0 ? static_cast<long>(container.string().size()) + num : num);
            if (i < container.string().size())
                return Value(std::string(1, container.string()[i]));
            else
                VM::throwVMError(
                    ErrorKind::Index,
                    fmt::format("{} out of range \"{}\" (length {})", num, container.string(), container.string().size()));
        }
        else
            throw types::TypeCheckingError(
                "@",
                { { types::Contract { { types::Typedef("src", ValueType::List), types::Typedef("idx", ValueType::Number) } },
                    types::Contract { { types::Typedef("src", ValueType::String), types::Typedef("idx", ValueType::Number) } } } },
                { container, index });
    }

    inline ARK_ALWAYS_INLINE Value atAt(const Value* x, const Value* y, Value& list)
    {
        if (y->valueType() != ValueType::Number || x->valueType() != ValueType::Number ||
            list.valueType() != ValueType::List)
            throw types::TypeCheckingError(
                "@@",
                { { types::Contract {
                    { types::Typedef("src", ValueType::List),
                      types::Typedef("y", ValueType::Number),
                      types::Typedef("x", ValueType::Number) } } } },
                { list, *y, *x });

        long idx_y = static_cast<long>(y->number());
        idx_y = idx_y < 0 ? static_cast<long>(list.list().size()) + idx_y : idx_y;
        if (std::cmp_greater_equal(idx_y, list.list().size()) || idx_y < 0)
            VM::throwVMError(
                ErrorKind::Index,
                fmt::format("@@ index ({}) out of range (list size: {})", idx_y, list.list().size()));

        const bool is_list = list.list()[static_cast<std::size_t>(idx_y)].valueType() == ValueType::List;
        const std::size_t size =
            is_list
            ? list.list()[static_cast<std::size_t>(idx_y)].list().size()
            : list.list()[static_cast<std::size_t>(idx_y)].stringRef().size();

        long idx_x = static_cast<long>(x->number());
        idx_x = idx_x < 0 ? static_cast<long>(size) + idx_x : idx_x;
        if (std::cmp_greater_equal(idx_x, size) || idx_x < 0)
            VM::throwVMError(
                ErrorKind::Index,
                fmt::format("@@ index (x: {}) out of range (inner indexable size: {})", idx_x, size));

        if (is_list)
            return list.list()[static_cast<std::size_t>(idx_y)].list()[static_cast<std::size_t>(idx_x)];
        else
            return Value(std::string(1, list.list()[static_cast<std::size_t>(idx_y)].stringRef()[static_cast<std::size_t>(idx_x)]));
    }

    inline ARK_ALWAYS_INLINE double doMath(double a, double b, const Instruction op)
    {
        if (op == ADD)
            a += b;
        else if (op == SUB)
            a -= b;
        else if (op == MUL)
            a *= b;
        else if (op == DIV)
        {
            if (b == 0)
                Ark::VM::throwVMError(ErrorKind::DivisionByZero, fmt::format("Can not compute expression (/ {} {})", a, b));
            a /= b;
        }

        return a;
    }

    inline ARK_ALWAYS_INLINE std::string mathInstToStr(const Instruction op)
    {
        if (op == ADD)
            return "+";
        if (op == SUB)
            return "-";
        if (op == MUL)
            return "*";
        if (op == DIV)
            return "/";
        return "???";
    }
}

#endif  // ARK_VM_HELPERS_HPP
