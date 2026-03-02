#include <Ark/Builtins/Builtins.hpp>

#include <utility>
#include <cmath>
#include <utf8.hpp>
#include <fmt/args.h>
#include <fmt/base.h>
#include <fmt/ostream.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/format.h>

#include <Ark/TypeChecker.hpp>
#include <Ark/VM/VM.hpp>

struct value_wrapper
{
    const Ark::Value& value;
    Ark::VM* vm_ptr;
    bool nested = false;
};

template <>
struct fmt::formatter<value_wrapper>
{
private:
    fmt::basic_string_view<char> opening_bracket_ = fmt::detail::string_literal<char, '['> {};
    fmt::basic_string_view<char> closing_bracket_ = fmt::detail::string_literal<char, ']'> {};
    fmt::basic_string_view<char> separator_ = fmt::detail::string_literal<char, ' '> {};
    bool is_debug = false;
    bool is_literal_str = false;
    fmt::formatter<std::string> underlying_;

public:
    void set_brackets(const fmt::basic_string_view<char> open, const fmt::basic_string_view<char> close)
    {
        opening_bracket_ = open;
        closing_bracket_ = close;
    }

    void set_separator(const fmt::basic_string_view<char> sep)
    {
        separator_ = sep;
    }

    format_parse_context::iterator parse(fmt::format_parse_context& ctx)
    {
        auto it = ctx.begin();
        const auto end = ctx.end();
        if (it == end)
            return underlying_.parse(ctx);

        switch (detail::to_ascii(*it))
        {
            case 'n':
                set_brackets({}, {});
                ++it;
                if (it == end)
                    report_error("invalid format specifier");
                if (*it == '}')
                    return it;
                if (*it != 'c' && *it != 'l')
                    report_error("invalid format specifier. Expected either :nc or :nl");
                [[fallthrough]];

            case 'c':
                if (*it == 'c')
                {
                    set_separator(fmt::detail::string_literal<char, ',', ' '> {});
                    ++it;
                    return it;
                }
                [[fallthrough]];

            case 'l':
                set_separator(fmt::detail::string_literal<char, '\n'> {});
                ++it;
                return it;

            case '?':
                is_debug = true;
                set_brackets({}, {});
                ++it;
                if (it == end || *it != 's')
                    report_error("invalid format specifier. Expected :?s, not :?");
                [[fallthrough]];

            case 's':
                if (!is_debug)
                {
                    set_brackets(fmt::detail::string_literal<char, '"'> {},
                                 fmt::detail::string_literal<char, '"'> {});
                    set_separator({});
                    is_literal_str = true;
                }
                ++it;
                return it;

            default:
                break;
        }

        if (it != end && *it != '}')
        {
            if (*it != ':')
                report_error("invalid format specifier");
            ++it;
        }

        ctx.advance_to(it);
        return underlying_.parse(ctx);
    }

    template <typename Output, typename It, typename Sentinel>
    auto write_debug_string(Output& out, It it, Sentinel end, Ark::VM* vm_ptr) const -> Output
    {
        auto buf = fmt::basic_memory_buffer<char>();
        for (; it != end; ++it)
        {
            auto formatted = it->toString(*vm_ptr);
            buf.append(formatted);
        }
        auto specs = fmt::format_specs();
        specs.set_type(fmt::presentation_type::debug);
        return fmt::detail::write<char>(
            out,
            fmt::basic_string_view<char>(buf.data(), buf.size()),
            specs);
    }

    fmt::format_context::iterator format(const value_wrapper& value, fmt::format_context& ctx) const
    {
        auto out = ctx.out();
        auto it = fmt::detail::range_begin(value.value.constList());
        const auto end = fmt::detail::range_end(value.value.constList());
        if (is_debug)
            return write_debug_string(out, it, end, value.vm_ptr);

        if ((is_literal_str && !value.nested) || !is_literal_str)
            out = fmt::detail::copy<char>(opening_bracket_, out);

        for (int i = 0; it != end; ++it)
        {
            if (i > 0)
                out = fmt::detail::copy<char>(separator_, out);
            ctx.advance_to(out);

            auto&& item = *it;
            if (item.valueType() == Ark::ValueType::List)
            {
                // if :s, do not put surrounding "" here
                format({ item, value.vm_ptr, /* nested= */ true }, ctx);
            }
            else
            {
                std::string formatted = item.toString(*value.vm_ptr);
                out = underlying_.format(formatted, ctx);
            }

            ++i;
        }

        if ((is_literal_str && !value.nested) || !is_literal_str)
            out = detail::copy<char>(closing_bracket_, out);

        return out;
    }
};

namespace Ark::internal::Builtins::String
{
    /**
     * @name format
     * @brief Format a String given replacements
     * @details See [fmt.dev](https://fmt.dev/12.0/syntax/) for syntax.
     * =details-begin
     * In the case of lists, we have custom specifiers:
     * - `n` removes surrounding brackets, uses ' ' as a separator
     * - `?s` debug format. The list is formatted as an escaped string
     * - `s` string format. The list is formatted as a string
     * - `c` changes the separator to ', '
     * - `l` changes the separator to '\n'
     *
     * `n` can be combined with either `c` and `l` (which are mutually exclusive): `nc`, `nl`.
     *
     * The underlying formatter is the one of strings, so you can write `::<10` to align all elements left in a 10 char wide block each.
     * =details-end
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
            {
                double int_part;
                if (std::modf(it->number(), &int_part) == 0.0)
                    store.push_back(static_cast<long>(it->number()));
                else
                    store.push_back(it->number());
            }
            else if (it->valueType() == ValueType::Nil)
                store.push_back("nil");
            else if (it->valueType() == ValueType::True)
                store.push_back("true");
            else if (it->valueType() == ValueType::False)
                store.push_back("false");
            else if (it->valueType() == ValueType::List)
            {
                // std::vector<value_wrapper> r;
                // std::ranges::transform(
                //     it->list(),
                //     std::back_inserter(r),
                //     [&vm](const Value& val) -> value_wrapper {
                //         return value_wrapper { val, vm };
                //     });
                store.push_back(value_wrapper { *it, vm });
            }
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
