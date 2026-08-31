#include <iostream>
#include <optional>
#include <filesystem>
#include <cstdlib>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/Utils/Files.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/Value/Dict.hpp>
#include <CLI/JsonCompiler.hpp>
#include <CLI/REPL/Repl.hpp>
#include <CLI/Formatter.hpp>

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
constexpr int ArkErrorExitCode = 0;
#else
constexpr int ArkErrorExitCode = -1;
#endif

int main(int argc, char** argv)
{
    using namespace clipp;

    enum class mode
    {
        help,
        dev_info,
        bytecode_reader,
        version,
        run,
        repl,
        compile,
        eval,
        ast,
        format
    };
    auto selected = mode::repl;

    unsigned debug = 0;

    // Bytecode reader
    // by default, select all pages and segment types, without slicing anything
    uint16_t bcr_page = Ark::MaxValue16Bits;
    uint16_t bcr_start = Ark::MaxValue16Bits;
    uint16_t bcr_end = Ark::MaxValue16Bits;
    auto segment = Ark::BytecodeSegment::All;
    // Eval / Run / AST dump
    std::string file, eval_expression;
    std::string libdir;
    // Formatting
    bool format_dry_run = false;
    bool format_check = false;
    // Generic arguments
    std::vector<std::string> script_args;

    uint16_t passes = Ark::DefaultFeatures | Ark::FeatureASTOptimiser;

    // clang-format off
    auto debug_flag = joinable(repeatable(option("-d", "--debug").call([&]{ debug++; })
                            .doc("Increase debug level (default: 0)\n")));
    auto lib_dir_flag = option("-L", "--lib").doc("Set the location of the ArkScript standard library. Paths can be delimited by ';'\n")
                            & value("lib_dir", libdir);

    auto import_solver_pass_flag = (
        option("-fimportsolver").call([&] { passes |= Ark::FeatureImportSolver; })
        | option("-fno-importsolver").call([&] { passes &= ~Ark::FeatureImportSolver; })
    ).doc("Toggle on and off the import solver pass");
    auto macro_proc_pass_flag = (
        option("-fmacroprocessor").call([&] { passes |= Ark::FeatureMacroProcessor; })
        | option("-fno-macroprocessor").call([&] { passes &= ~Ark::FeatureMacroProcessor; })
    ).doc("Toggle on and off the macro processor pass");
    auto optimizer_pass_flag = (
        option("-foptimizer").call([&] { passes |= Ark::FeatureASTOptimiser; })
        | option("-fno-optimizer").call([&] { passes &= ~Ark::FeatureASTOptimiser; })
    ).doc("Toggle on and off the optimizer pass");
    auto ir_inliner_pass_flag = (
        option("-firinliner").call([&] { passes |= Ark::FeatureIRInliner; })
        | option("-fno-irinliner").call([&] { passes &= ~Ark::FeatureIRInliner; })
    ).doc("Toggle on and off the IR inliner pass");
    auto ir_optimizer_pass_flag = (
        option("-firoptimizer").call([&] { passes |= Ark::FeatureIROptimiser; })
        | option("-fno-iroptimizer").call([&] { passes &= ~Ark::FeatureIROptimiser; })
    ).doc("Toggle on and off the IR optimizer pass");
    auto vm_debugger_flag = (
        option("-fdebugger").call([&] { passes |= Ark::FeatureVMDebugger; })
    ).doc("Turn on the debugger");
    auto ir_dump = option("-fdump-ir").call([&] { passes |= Ark::FeatureDumpIR; })
        .doc("Dump IR to file.ark.ir");
    auto no_cache = option("-fno-cache").call([&] { passes |= Ark::DisableCache; })
        .doc("Disable the bytecode cache creation");

    const auto run_flags = (
        // cppcheck-suppress constStatement
        debug_flag, lib_dir_flag, import_solver_pass_flag, macro_proc_pass_flag,
        // cppcheck-suppress constStatement
        optimizer_pass_flag, ir_inliner_pass_flag, ir_optimizer_pass_flag,
        // cppcheck-suppress constStatement
        vm_debugger_flag, ir_dump,
        no_cache
    );

    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("-v", "--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            required("-e", "--eval").set(selected, mode::eval).doc("Evaluate ArkScript expression")
            & value("expression", eval_expression)
        )
        | (
            run_flags
            , (
                required("-c", "--compile").set(selected, mode::compile).doc("Compile the given program to bytecode, but do not run")
                & value("file", file).doc("If file is -, it reads code from stdin")
            )
            | value("file", file).set(selected, mode::run)
        )
        | (
            required("-f", "--format").set(selected, mode::format).doc("Format the given source file in place")
            & value("file", file)
            , (
                option("--dry-run").set(format_dry_run, true).doc("Do not modify the file, only print out the changes")
                | option("--check").set(format_check, true).doc("Check if a file formating is correctly, without modifying it. Return 1 if formating is needed, 0 otherwise")
            )
        )
        | (
            debug_flag
            , lib_dir_flag
            , required("--ast").set(selected, mode::ast).doc("Compile the given program and output its AST as JSON to stdout")
            & value("file", file)
        )
        | (
            required("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
            & value("file", file).doc(".arkc bytecode file or .ark source file that will be compiled first")
            , (
                option("-on", "--only-names").set(segment, Ark::BytecodeSegment::HeadersOnly).doc("Display only the bytecode segments names and sizes")
                | (
                    (
                        option("-a", "--all").set(segment, Ark::BytecodeSegment::All).doc("Display all the bytecode segments (default)")
                        | option("-st", "--symbols").set(segment, Ark::BytecodeSegment::Symbols).doc("Display only the symbols table")
                        | option("-vt", "--values").set(segment, Ark::BytecodeSegment::Values).doc("Display only the values table")
                        | (
                            option("-cs", "--code").set(segment, Ark::BytecodeSegment::Code).doc("Display only the code segments")
                            , option("-p", "--page").set(segment, Ark::BytecodeSegment::Code).doc("Set the bytecode reader code segment to display")
                            & value("page", bcr_page)
                        )
                    )
                    , option("-s", "--slice").doc("Select a slice of instructions in the bytecode")
                        & value("start", bcr_start)
                        & value("end", bcr_end)
                )
            )
        )
        , any_other(script_args)
    );
    // clang-format on

    auto fmt = doc_formatting {}
                   .first_column(8)                                   // column where usage lines and documentation starts
                   .doc_column(36)                                    // parameter docstring start col
                   .indent_size(2)                                    // indent of documentation lines for children of a documented group
                   .split_alternatives(true)                          // split usage into several lines for large alternatives
                   .merge_alternative_flags_with_common_prefix(true)  // [-fok] [-fno-ok] becomes [-f(ok|no-ok)]
                   .paragraph_spacing(1)
                   .ignore_newline_chars(false);
    const auto man_page = make_man_page(cli, "arkscript", fmt)
                              .prepend_section("DESCRIPTION", "        ArkScript programming language")
                              .append_section("VERSION", fmt::format("        {}", ARK_FULL_VERSION))
                              .append_section("BUILD DATE", fmt::format("        {}", ARK_BUILD_DATE))
                              .append_section("LICENSE", "        Mozilla Public License 2.0");

    if (auto result = parse(argc, argv, cli))
    {
        using namespace Ark;

        std::vector<std::filesystem::path> lib_paths;
        // if arkscript lib paths were provided by the CLI, bypass the automatic lookup
        if (!libdir.empty())
        {
            std::ranges::transform(Utils::splitString(libdir, ';'), std::back_inserter(lib_paths), [](const std::string& path) {
                return std::filesystem::path(path);
            });
        }
        else
        {
            if (const char* arkpath = std::getenv("ARKSCRIPT_PATH"))
            {
                std::ranges::transform(Utils::splitString(arkpath, ';'), std::back_inserter(lib_paths), [](const std::string& path) {
                    return std::filesystem::path(path);
                });
            }
            else if (Utils::fileExists("./lib") && Utils::fileExists("./lib/std/Prelude.ark"))
                lib_paths.emplace_back("lib");
            else if (!DefaultLibFolder.empty() && Utils::fileExists(std::string(DefaultLibFolder) + "/std/Prelude.ark"))
                lib_paths.emplace_back(DefaultLibFolder);
            else if (debug > 0)
                fmt::println(std::cerr, "{}: Couldn't read ARKSCRIPT_PATH environment variable", fmt::styled("Warning", fmt::fg(fmt::color::dark_orange)));
        }

        switch (selected)
        {
            case mode::help:
                std::cout << man_page << std::endl;
                break;

            case mode::version:
                fmt::println(ARK_FULL_VERSION);
                break;

            case mode::dev_info:
            {
                fmt::println("Compiler used: {}\n", ARK_COMPILER);
                fmt::println("{:^34}|{:^8}|{:^10}", "Type", "SizeOf", "AlignOf");

#define ARK_PRINT_SIZE(type) fmt::println("{:<34}| {:<7}| {:<9}", #type, sizeof(type), alignof(type))
                ARK_PRINT_SIZE(char);

                ARK_PRINT_SIZE(Ark::Value);
                ARK_PRINT_SIZE(Ark::Value::Value_t);
                ARK_PRINT_SIZE(Ark::ValueType);
                ARK_PRINT_SIZE(Ark::Procedure);
                ARK_PRINT_SIZE(std::vector<Ark::Value>);
                ARK_PRINT_SIZE(Ark::Value::Dict_t);
                ARK_PRINT_SIZE(Ark::internal::Closure);
                ARK_PRINT_SIZE(Ark::UserType);

                ARK_PRINT_SIZE(Ark::VM);
                ARK_PRINT_SIZE(Ark::State);
                ARK_PRINT_SIZE(Ark::internal::ScopeView);
                ARK_PRINT_SIZE(Ark::internal::ExecutionContext);

                ARK_PRINT_SIZE(Ark::internal::Node);
#undef ARK_PRINT_SIZE
                break;
            }

            case mode::repl:
            {
                Ark::Repl repl(lib_paths);
                return repl.run();
            }

            case mode::compile:
            {
                Ark::State state(lib_paths);
                state.setDebug(debug);

                if (!state.doFile(file, passes))
                    return ArkErrorExitCode;
                break;
            }

            case mode::run:
            {
                Ark::State state(lib_paths);
                state.setDebug(debug);
                state.setArgs(script_args);

                if (file == "-")
                {
                    std::string content(std::istreambuf_iterator<char>(std::cin), {});
                    if (!state.doString(content, passes))
                        return ArkErrorExitCode;
                }
                else if (!state.doFile(file, passes))
                    return ArkErrorExitCode;

                Ark::VM vm(state);
                return vm.run();
            }

            case mode::eval:
            {
                Ark::State state(lib_paths);
                state.setDebug(debug);

                if (!state.doString(eval_expression))
                {
                    std::cerr << "Could not evaluate expression\n";
                    return ArkErrorExitCode;
                }

                Ark::VM vm(state);
                return vm.run();
            }

            case mode::ast:
            {
                JsonCompiler compiler(debug, lib_paths);
                compiler.feed(file);
                fmt::println("{}", compiler.compile());
                break;
            }

            case mode::bytecode_reader:
            {
                try
                {
                    Ark::BytecodeReader bcr;
                    bcr.feed(file);
                    if (!bcr.checkMagic())
                    {
                        // we got a potentially non-compiled file
                        fmt::println("Compiling {}...", file);

                        Ark::Welder welder(debug, lib_paths);
                        welder.computeASTFromFile(file);
                        welder.generateBytecode();
                        bcr.feed(welder.bytecode());
                    }

                    if (bcr_page == Ark::MaxValue16Bits && bcr_start == Ark::MaxValue16Bits)
                        bcr.display(segment);
                    else if (bcr_page != Ark::MaxValue16Bits && bcr_start == Ark::MaxValue16Bits)
                        bcr.display(segment, std::nullopt, std::nullopt, bcr_page);
                    else if (bcr_page == Ark::MaxValue16Bits && bcr_start != Ark::MaxValue16Bits)
                        bcr.display(segment, bcr_start, bcr_end);
                    else
                        bcr.display(segment, bcr_start, bcr_end, bcr_page);
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    return ArkErrorExitCode;
                }
                break;
            }

            case mode::format:
            {
                // dry run and check should not update the file
                Formatter formatter(file, format_dry_run || format_check);
                formatter.run();
                if (format_dry_run)
                    fmt::println("{}", formatter.output());
                if (formatter.codeModified() && format_check)
                    return 1;
            }
        }
    }
    else
    {
        std::cerr << "Could not parse CLI arguments" << std::endl;

        auto doc_label = [](const parameter& p) {
            if (!p.flags().empty())
                return p.flags().front();
            if (!p.label().empty())
                return p.label();
            return doc_string { "<?>" };
        };

        std::cout << "args -> parameter mapping:\n";
        for (const auto& m : result)
        {
            std::cout << "#" << m.index() << " " << m.arg() << " -> ";
            if (const parameter* p = m.param(); p)
            {
                std::cout << doc_label(*p) << " \t";
                if (m.repeat() > 0)
                {
                    std::cout << (m.bad_repeat() ? "[bad repeat " : "[repeat ")
                              << m.repeat() << "]";
                }
                if (m.blocked())
                    std::cout << " [blocked]";
                if (m.conflict())
                    std::cout << " [conflict]";
                std::cout << '\n';
            }
            else
                std::cout << " [unmapped]\n";
        }

        std::cout << "missing parameters:\n";
        for (const auto& m : result.missing())
        {
            if (const parameter* p = m.param(); p)
            {
                std::cout << doc_label(*p) << " \t";
                std::cout << " [missing after " << m.after_index() << "]\n";
            }
        }
    }

    return 0;
}
