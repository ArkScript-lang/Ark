#include <boost/ut.hpp>

#include <ranges>
#include <string>
#include <sstream>
#include <fmt/core.h>

#include <Ark/State.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/VM/Value/Value.hpp>
#include <Ark/TypeChecker.hpp>
#include <Ark/Utils/Utils.hpp>
#include <TestsHelper.hpp>

using namespace boost;

struct Input
{
    std::string func;
    std::size_t expected_arg_count {};
    Ark::types::Contract expected_arg_types;
    std::vector<Ark::Value> given_args;

    std::string filename;
    bool initialized = false;
};

Input parse_input(const std::string& path)
{
    using namespace ut;

    const auto content = Ark::Utils::readFile(path);
    const auto lines = Ark::Utils::splitString(content, '\n');
    expect(lines.size() > 1) << fmt::format("not enough data to construct test input {}\n", path) << fatal;

    // parsing "# name,count"
    //            ^^^^
    const auto funcname = lines[0].substr(
        lines[0].find_first_not_of("# "),
        lines[0].find_first_of(',') - lines[0].find_first_not_of("# "));
    expect(!funcname.empty()) << fmt::format("function name empty in test input {}\n", path) << fatal;

    // parsing "# name,count"
    //                 ^^^^^
    const auto s_arg_count = lines[0].substr(lines[0].find_first_of(',') + 1);
    expect(s_arg_count.size() == 1 && s_arg_count[0] >= '0' && s_arg_count[0] <= '9')
        << fmt::format("expected argc should be 0-9 in test input {}\n", path)
        << fatal;
    const auto arg_count = static_cast<std::size_t>(s_arg_count[0] - '0');
    expect(lines.size() >= 2 + arg_count)
        << fmt::format("not enough defined arguments in test input {}\n", path)
        << fatal;

    // parsing following lines defining argument types "# name:ArkType,name:ArkType,..."
    Ark::types::Contract args;
    auto parse_def = [&path, &args](std::size_t i, const std::string& def, bool is_sum_type = false) {
        // create variadic argument
        if (def == "..." && !args.arguments.empty())
        {
            args.arguments.back().variadic = true;
            // bypass all the subsequent tests as they would fail since "..." isn't a valid typedef
            return;
        }

        const auto arg_name = def.substr(0, def.find_first_of(':'));
        const auto type_name = def.substr(def.find_first_of(':') + 1);

        if (auto it = std::ranges::find(Ark::types_to_str, type_name); it != Ark::types_to_str.end())
        {
            auto type = static_cast<Ark::ValueType>(std::distance(Ark::types_to_str.begin(), it));
            if (!is_sum_type || args.arguments.empty() || args.arguments.back().types.empty())
                args.arguments.emplace_back(arg_name, type);
            else
                args.arguments.back().types.emplace_back(type);
        }
        else
            expect(false)
                << fmt::format("[line {}] couldn't find the corresponding Ark type for {} in test input {}\n", i + 1, type_name, path)
                << fatal;
    };
    for (std::size_t i = 0; i < arg_count; ++i)
    {
        const auto& definition = lines[1 + i].substr(lines[1 + i].find_first_not_of("# "));
        const auto args_with_name = Ark::Utils::splitString(definition, ',');

        if (args_with_name.size() == 1)
        {
            const auto& def = args_with_name.front();
            parse_def(i, def);
        }
        else
        {
            // argument can have multiple types
            for (auto&& def : args_with_name)
                parse_def(i, def, /* is_sum_type= */ true);
        }
    }

    // parsing given argument list "# ArkType,ArkType,..."
    const auto tmp = lines[1 + arg_count].find_first_not_of("# ");
    const auto args_def = Ark::Utils::splitString(lines[1 + arg_count].substr(tmp), ',');
    std::vector<Ark::Value> given_args;
    for (const auto& arg_type : args_def)
    {
        if (auto it = std::ranges::find(Ark::types_to_str, arg_type); it != Ark::types_to_str.end())
        {
            auto type = static_cast<Ark::ValueType>(std::distance(Ark::types_to_str.begin(), it));
            switch (type)
            {
                case Ark::ValueType::List:
                    given_args.emplace_back(std::vector { Ark::Value(1) });
                    break;

                case Ark::ValueType::Number:
                    given_args.emplace_back(1.0);
                    break;

                case Ark::ValueType::String:
                    given_args.emplace_back("hello");
                    break;

                case Ark::ValueType::PageAddr:
                    given_args.emplace_back(static_cast<Ark::internal::PageAddr_t>(12));
                    break;

                case Ark::ValueType::CProc:
                    given_args.emplace_back(Ark::Procedure([](std::vector<Ark::Value>&, Ark::VM*) -> Ark::Value {
                        return Ark::Value(Ark::ValueType::Nil);
                    }));
                    break;

                case Ark::ValueType::Nil:
                    [[fallthrough]];
                case Ark::ValueType::True:
                    [[fallthrough]];
                case Ark::ValueType::False:
                    given_args.emplace_back(type);
                    break;

                case Ark::ValueType::Closure:
                    // unsupported
                    [[fallthrough]];
                case Ark::ValueType::Dict:
                    // unsupported
                    [[fallthrough]];
                case Ark::ValueType::User:
                    // unsupported
                    [[fallthrough]];
                case Ark::ValueType::Undefined:
                    [[fallthrough]];
                case Ark::ValueType::Reference:
                    [[fallthrough]];
                case Ark::ValueType::InstPtr:
                    [[fallthrough]];
                case Ark::ValueType::Garbage:
                    [[fallthrough]];
                case Ark::ValueType::Any:
                    break;
            }
        }
        else
            expect(false)
                << fmt::format("[line {}] couldn't find the corresponding Ark type for {} in test input {}\n", 1 + arg_count, arg_type, path)
                << fatal;
    }

    return Input {
        .func = funcname,
        .expected_arg_count = arg_count,
        .expected_arg_types = args,
        .given_args = given_args,
        .filename = std::filesystem::path(path).stem().generic_string(),
        .initialized = true
    };
}

ut::suite<"TypeChecker"> type_checker_suite = [] {
    using namespace ut;

    iterTestFiles(
        "TypeCheckerSuite",
        [&](TestData&& data) {
            std::vector<Input> inputs;
            std::vector<Ark::types::Contract> contracts;

            if (data.is_folder)
            {
                iterTestFiles(
                    data.path,
                    [&inputs](TestData&& inner) {
                        const Input input = parse_input(inner.path);
                        expect(fatal(input.initialized)) << "invalid test input: " << inner.stem;
                        inputs.push_back(input);
                    },
                    { .folder_is_resource = false, .ignore_expected = true });

                std::ranges::sort(inputs, [](const Input& a, const Input& b) {
                    return a.filename < b.filename;
                });

                std::ranges::transform(inputs, std::back_inserter(contracts), [](const Input& input) {
                    return input.expected_arg_types;
                });
            }
            else
            {
                const Input input = parse_input(data.path);
                expect(fatal(input.initialized)) << "invalid test input: " << data.stem;
                inputs.push_back(input);
                contracts.push_back(input.expected_arg_types);
            }

            should("generate error message " + data.stem) = [inputs, contracts, data] {
                Ark::State dummy_state;
                Ark::VM dummy_VM(dummy_state);
                std::stringstream stream;
                Ark::types::generateError(
                    inputs.front().func,
                    contracts,
                    inputs.front().given_args,
                    dummy_VM,
                    stream,
                    /* colorize= */ false);

                auto result = stream.str();
                Ark::Utils::rtrim(Ark::Utils::ltrim(result));
                expectOrDiff(data.expected, result);
                if (shouldWriteNewDiffsTofile() && data.expected != result)
                    updateExpectedFile(data, result);
            };
        },
        { .skip_folders = false });
};
