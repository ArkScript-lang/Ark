#include <Ark/Builtins/Builtins.hpp>

#include <utility>
#include <utf8.hpp>
#include <fmt/args.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::String
{
    /**
     * @name format
     * @brief Format a String given replacements
     * @details https://fmt.dev/12.0/syntax/
     * @param format the String to format
     * @param values as any argument as you need, of any valid ArkScript type
     * =begin
     * (format "Hello {}, my name is {}" "world" "ArkScript")
     * # Hello world, my name is ArkScript
     *
     * (format "Test {} with {{}}" "1")
     * # Test 1 with {}
     * =end
     * @author https://github.com/SuperFola
     */
    Value format(std::vector<Value>& n, VM* vm)
    {
        if (n.size() < 2 || n[0].valueType() != ValueType::String)
            throw types::TypeCheckingError(
                "format",
                { { types::Contract { { types::Typedef("string", ValueType::String),
                                        types::Typedef("value", ValueType::Any, /* variadic */ true) } } } },
                n);

        fmt::dynamic_format_arg_store<fmt::format_context> store;

        for (auto it = n.begin() + 1, it_end = n.end(); it != it_end; ++it)
        {
            if (it->valueType() == ValueType::String)
                store.push_back(it->stringRef());
            else if (it->valueType() == ValueType::Number)
                store.push_back(it->number());
            else if (it->valueType() == ValueType::Nil)
                store.push_back("nil");
            else if (it->valueType() == ValueType::True)
                store.push_back("true");
            else if (it->valueType() == ValueType::False)
                store.push_back("false");
            else
                store.push_back(it->toString(*vm));
        }

        try
        {
            return Value(fmt::vformat(n[0].stringRef(), store));
        }
        catch (fmt::format_error& e)
        {
            throw std::runtime_error(
                fmt::format("format: can not format \"{}\" ({} argument{} provided) because of {}",
                            n[0].stringRef(),
                            n.size() - 1,
                            // if we have more than one argument (not counting the string to format), plural form
                            n.size() > 2 ? "s" : "",
                            e.what()));
        }
    }

    Value findSubStr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String, ValueType::String) &&
            !types::check(n, ValueType::String, ValueType::String, ValueType::Number))
            throw types::TypeCheckingError(
                "string:find",
                { { types::Contract {
                        { types::Typedef("string", ValueType::String),
                          types::Typedef("substr", ValueType::String) } },
                    types::Contract {
                        { types::Typedef("string", ValueType::String),
                          types::Typedef("substr", ValueType::String),
                          types::Typedef("startIndex", ValueType::Number) } } } },
                n);

        const std::size_t start = n.size() == 3 ? static_cast<std::size_t>(n[2].number()) : 0;
        const std::size_t index = n[0].stringRef().find(n[1].stringRef(), start);
        if (index != std::string::npos)
            return Value(static_cast<int>(index));
        return Value(-1);
    }

    Value removeAtStr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String, ValueType::Number))
            throw types::TypeCheckingError(
                "string:removeAt",
                { { types::Contract { { types::Typedef("string", ValueType::String), types::Typedef("index", ValueType::Number) } } } },
                n);

        long num = static_cast<long>(n[1].number());
        const auto i = static_cast<std::size_t>(num < 0 ? static_cast<long>(n[0].stringRef().size()) + num : num);
        if (i < n[0].stringRef().size())
        {
            n[0].stringRef().erase(i, 1);
            return n[0];
        }
        else
            throw std::runtime_error(fmt::format("string:removeAt: index {} out of range (length: {})", num, n[0].stringRef().size()));
    }

    Value ord(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "string:ord",
                { { types::Contract { { types::Typedef("string", ValueType::String) } } } },
                n);

        return Value(utf8::codepoint(n[0].stringRef().c_str()));
    }

    // cppcheck-suppress constParameterReference
    Value chr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::Number))
            throw types::TypeCheckingError(
                "string:chr",
                { { types::Contract { { types::Typedef("codepoint", ValueType::Number) } } } },
                n);

        std::array<char, 5> utf8 {};
        utf8::codepointToUtf8(static_cast<int>(n[0].number()), utf8.data());
        return Value(std::string(utf8.data()));
    }

    Value setStringAt(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String, ValueType::Number, ValueType::String))
            throw types::TypeCheckingError(
                "string:setAt",
                { { types::Contract { { types::Typedef("string", ValueType::String),
                                        types::Typedef("index", ValueType::Number),
                                        types::Typedef("value", ValueType::String) } } } },
                n);

        auto& string = n[0].stringRef();

        const std::size_t size = string.size();
        long idx = static_cast<long>(n[1].number());
        idx = idx < 0 ? static_cast<long>(size) + idx : idx;
        if (std::cmp_greater_equal(idx, size))
            throw std::runtime_error(
                fmt::format("IndexError: string:setAt index ({}) out of range (string size: {})", idx, size));

        string[static_cast<std::size_t>(idx)] = n[2].string()[0];
        return n[0];
    }
}
