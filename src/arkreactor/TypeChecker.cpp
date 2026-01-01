#include <Ark/TypeChecker.hpp>

#include <limits>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/args.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/VM/VM.hpp>

namespace Ark::types
{
    std::string typeListToString(const std::vector<ValueType>& types)
    {
        if (types.size() == 1 && types[0] == ValueType::Any)
            return "any";

        std::string acc;

        for (std::size_t i = 0, end = types.size(); i < end; ++i)
        {
            if (i > 0)
                acc += ", ";
            if (i + 1 == end && types.size() > 1)
                acc += "or ";
            acc += std::to_string(types[i]);
        }
        return acc;
    }

    void displayContract(const std::string_view& funcname, const Contract& contract, const std::vector<Value>& args, VM& vm, std::ostream& os, const bool colorize)
    {
        const std::string checkmark = "✓";
        const std::string crossmark = "×";

        auto displayArg = [colorize, &os](const Typedef& td, const bool correct) {
            const std::string arg_str = typeListToString(td.types);

            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(td.variadic ? "variadic " : "");
            if (colorize)
                store.push_back(
                    fmt::styled(
                        td.name,
                        correct
                            ? fmt::fg(fmt::color::green)
                            : fmt::fg(fmt::color::magenta)));
            else
                store.push_back(td.name);
            store.push_back(arg_str);

            fmt::vprint(os, "  → {}`{}' (expected {})", store);
        };

        fmt::print(os, "Signature\n  ↳ ({}", funcname);
        for (const Typedef& td : contract.arguments)
            fmt::print(os, " {}", td.name);
        fmt::print(os, ")\nArguments\n");

        for (std::size_t i = 0, end = contract.arguments.size(); i < end; ++i)
        {
            const Typedef& td = contract.arguments[i];

            if (td.variadic && i < args.size())
            {
                // variadic argument in contract and enough provided arguments
                std::size_t bad_type_count = 0;
                std::string formatted_varargs;
                for (std::size_t j = i, args_end = args.size(); j < args_end; ++j)
                {
                    bool type_ok = true;
                    if (td.types[0] != ValueType::Any && std::ranges::find(td.types, args[j].valueType()) == td.types.end())
                    {
                        bad_type_count++;
                        type_ok = false;
                    }
                    formatted_varargs += fmt::format(
                        "\n    {} ({}) {}",
                        args[j].toString(vm, /* show_as_code= */ true),
                        std::to_string(args[j].valueType()),
                        type_ok ? checkmark : crossmark);
                }

                if (bad_type_count)
                {
                    displayArg(td, /* correct= */ false);

                    fmt::dynamic_format_arg_store<fmt::format_context> store;
                    if (colorize)
                        store.push_back(fmt::styled(bad_type_count, fmt::fg(fmt::color::red)));
                    else
                        store.push_back(bad_type_count);
                    store.push_back(bad_type_count > 1 ? "s" : "");
                    store.push_back(formatted_varargs);

                    fmt::vprint(os, ": {} argument{} do not match:{}", store);
                }
                else
                    displayArg(td, /* correct= */ true);
            }
            else
            {
                // provided argument but wrong type
                if (i < args.size() && td.types[0] != ValueType::Any && std::ranges::find(td.types, args[i].valueType()) == td.types.end())
                {
                    displayArg(td, /* correct= */ false);
                    const auto type = std::to_string(args[i].valueType());

                    fmt::dynamic_format_arg_store<fmt::format_context> store;
                    store.push_back(args[i].toString(vm, /* show_as_code= */ true));
                    if (colorize)
                        store.push_back(fmt::styled(type, fmt::fg(fmt::color::red)));
                    else
                        store.push_back(type);
                    fmt::vprint(os, ", got {} ({})", store);
                }
                // non-provided argument
                else if (i >= args.size())
                {
                    displayArg(td, /* correct= */ false);
                    if (colorize)
                        fmt::print(os, "{}", fmt::styled(" was not provided", fmt::fg(fmt::color::red)));
                    else
                        fmt::print(os, " was not provided");
                }
                else
                {
                    displayArg(td, /* correct= */ true);
                    fmt::print(os, " {}", checkmark);
                }
            }
            fmt::print(os, "\n");
        }

        if (contract.arguments.size() < args.size())
        {
            fmt::print(os, "  → unexpected additional args: ");
            for (std::size_t i = contract.arguments.size(), end = args.size(); i < end; ++i)
            {
                fmt::print(os, "{} ({})", args[i].toString(vm, /* show_as_code= */ true), std::to_string(args[i].valueType()));
                if (i + 1 != end)
                    fmt::print(os, ", ");
            }
            fmt::print(os, "\n");
        }
    }

    void generateError(const std::string_view& funcname, const std::vector<Contract>& contracts, const std::vector<Value>& args, VM& vm, std::ostream& os, bool colorize)
    {
        {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            if (colorize)
                store.push_back(fmt::styled(funcname, fmt::fg(fmt::color::cyan)));
            else
                store.push_back(funcname);
            fmt::vprint(os, "Function {} expected ", store);
        }

        std::vector<Value> sanitizedArgs;
        std::ranges::copy_if(args, std::back_inserter(sanitizedArgs), [](const Value& value) -> bool {
            return value.valueType() != ValueType::Undefined;
        });

        // get expected arguments count
        std::size_t min_argc = std::numeric_limits<std::size_t>::max(), max_argc = 0;
        bool variadic = false;
        for (const auto& [arguments] : contracts)
        {
            if (arguments.size() < min_argc)
                min_argc = arguments.size();
            if (arguments.size() > max_argc)
                max_argc = arguments.size();

            if (!arguments.empty() && arguments.back().variadic)
                variadic = true;
        }

        bool correct_argcount = true;

        if (min_argc != max_argc)
        {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            if (colorize)
                store.push_back(fmt::styled(min_argc, fmt::fg(fmt::color::yellow)));
            else
                store.push_back(min_argc);
            store.push_back(min_argc > 1 ? "s" : "");
            if (colorize)
                store.push_back(fmt::styled(max_argc, fmt::fg(fmt::color::yellow)));
            else
                store.push_back(max_argc);
            store.push_back(max_argc > 1 ? "s" : "");

            fmt::vprint(os, "between {} argument{} and {} argument{}", store);

            if (sanitizedArgs.size() < min_argc || sanitizedArgs.size() > max_argc)
                correct_argcount = false;
        }
        else
        {
            fmt::dynamic_format_arg_store<fmt::format_context> store;
            store.push_back(variadic ? "at least " : "");
            if (colorize)
                store.push_back(fmt::styled(min_argc, fmt::fg(fmt::color::yellow)));
            else
                store.push_back(min_argc);
            store.push_back(min_argc > 1 ? "s" : "");

            fmt::vprint(os, "{}{} argument{}", store);

            if (sanitizedArgs.size() != min_argc)
                correct_argcount = false;
        }

        if (!correct_argcount || variadic)
        {
            std::string preposition = (variadic && args.size() >= min_argc) ? "and" : "but";
            if (colorize)
                fmt::print(os, " {} got {}", preposition, fmt::styled(sanitizedArgs.size(), fmt::fg(fmt::color::red)));
            else
                fmt::print(os, " {} got {}", preposition, sanitizedArgs.size());
        }

        fmt::print(os, "\nCall\n  ↳ ({}", funcname);
        for (const Value& arg : args)
            fmt::print(os, " {}", arg.toString(vm, /* show_as_code= */ true));
        fmt::print(os, ")\n");

        displayContract(funcname, contracts[0], sanitizedArgs, vm, os, colorize);
        for (std::size_t i = 1, end = contracts.size(); i < end; ++i)
        {
            fmt::print(os, "\nAlternative {}:\n", i + 1);
            displayContract(funcname, contracts[i], sanitizedArgs, vm, os, colorize);
        }
    }
}
