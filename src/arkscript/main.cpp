#include <iostream>
#include <optional>
#include <filesystem>
#include <limits>
#include <cstdlib>

#include <clipp.h>
#include <fmt/core.h>
#include <fmt/color.h>

#include <Ark/Files.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <CLI/JsonCompiler.hpp>
#include <CLI/REPL/Repl.hpp>
#include <CLI/Formatter.hpp>

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

    constexpr uint16_t max_uint16 = std::numeric_limits<uint16_t>::max();

    // Bytecode reader
    // by default, select all pages and segment types, without slicing anything
    uint16_t bcr_page = max_uint16;
    uint16_t bcr_start = max_uint16;
    uint16_t bcr_end = max_uint16;
    auto segment = Ark::BytecodeSegment::All;
    // Eval / Run / AST dump
    std::string file, eval_expression;
    std::string libdir;
    // Formatting
    bool format_dry_run = false;
    bool format_check = false;
    // Generic arguments
    std::vector<std::string> wrong, script_args;

    uint16_t passes = Ark::DefaultFeatures;

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
        option("-foptimizer").call([&] { passes |= Ark::FeatureASTOptimizer; })
        | option("-fno-optimizer").call([&] { passes &= ~Ark::FeatureASTOptimizer; })
    ).doc("Toggle on and off the optimizer pass");
    auto ir_optimizer_pass_flag = (
        option("-firoptimizer").call([&] { passes |= Ark::FeatureIROptimizer; })
        | option("-fno-iroptimizer").call([&] { passes &= ~Ark::FeatureIROptimizer; })
    ).doc("Toggle on and off the IR optimizer pass");
    auto ir_dump = option("-fdump-ir").call([&] { passes |= Ark::FeatureDumpIR; })
        .doc("Dump IR to file.ark.ir");

    const auto compiler_passes_flag = (
        // cppcheck-suppress constStatement
        import_solver_pass_flag, macro_proc_pass_flag, optimizer_pass_flag, ir_optimizer_pass_flag, ir_dump
    );

    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("-v", "--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            required("-e", "--eval").set(selected, mode::eval).doc("Evaluate ArkScript expression\n")
            & value("expression", eval_expression)
        )
        | (
            required("-c", "--compile").set(selected, mode::compile).doc("Compile the given program to bytecode, but do not run")
            & value("file", file)
            , debug_flag
            , compiler_passes_flag
        )
        | (
            value("file", file).set(selected, mode::run)
            , (
                  debug_flag
                , lib_dir_flag
                , compiler_passes_flag
            )
            , any_other(script_args)
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
            required("--ast").set(selected, mode::ast).doc("Compile the given program and output its AST as JSON to stdout")
            & value("file", file)
            , debug_flag
            , lib_dir_flag
        )
        | (
            required("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
            & value("file", file).doc("If file isn't a bytecode file, the cached compiled will be loaded ; if there are none, it will be compiled first")
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
        , any_other(wrong)
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
                              .append_section("LICENSE", "        Mozilla Public License 2.0");

    if (parse(argc, argv, cli) && wrong.empty())
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
            else if (Utils::fileExists("./lib"))
                lib_paths.emplace_back("lib");
            else
                fmt::println("{}:  Couldn't read ARKSCRIPT_PATH environment variable", fmt::styled("Warning", fmt::fg(fmt::color::dark_orange)));
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
                fmt::println(
                    "Have been compiled with {}\n\n"
                    "sizeof(Ark::Value)    = {}B\n"
                    "      sizeof(Value_t) = {}B\n"
                    "      sizeof(ValueType) = {}B\n"
                    "      sizeof(ProcType)  = {}B\n"
                    "      sizeof(Ark::Closure)  = {}B\n"
                    "      sizeof(Ark::UserType) = {}B\n"
                    "\nVirtual Machine\n"
                    "sizeof(Ark::VM)       = {}B\n"
                    "      sizeof(Ark::State)    = {}B\n"
                    "      sizeof(Ark::Scope)    = {}B\n"
                    "      sizeof(ExecutionContext) = {}B\n"
                    "\nMisc\n"
                    "    sizeof(vector<Ark::Value>) = {}B\n"
                    "    sizeof(char)          = {}B\n"
                    "\nsizeof(Node)           = {}B",
                    ARK_COMPILER,
                    // value
                    sizeof(Ark::Value),
                    sizeof(Ark::Value::Value_t),
                    sizeof(Ark::ValueType),
                    sizeof(Ark::Value::ProcType),
                    sizeof(Ark::internal::Closure),
                    sizeof(Ark::UserType),
                    // vm
                    sizeof(Ark::VM),
                    sizeof(Ark::State),
                    sizeof(Ark::internal::Scope),
                    sizeof(Ark::internal::ExecutionContext),
                    // misc
                    sizeof(std::vector<Ark::Value>),
                    sizeof(char),
                    sizeof(Ark::internal::Node));
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
                    return -1;

                break;
            }

            case mode::run:
            {
                Ark::State state(lib_paths);
                state.setDebug(debug);
                state.setArgs(script_args);

                if (!state.doFile(file, passes))
                    return -1;

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
                    return -1;
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

                    if (bcr_page == max_uint16 && bcr_start == max_uint16)
                        bcr.display(segment);
                    else if (bcr_page != max_uint16 && bcr_start == max_uint16)
                        bcr.display(segment, std::nullopt, std::nullopt, bcr_page);
                    else if (bcr_page == max_uint16 && bcr_start != max_uint16)
                        bcr.display(segment, bcr_start, bcr_end);
                    else
                        bcr.display(segment, bcr_start, bcr_end, bcr_page);
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    return -1;
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
                if (formatter.codeModified())
                    return 1;
            }
        }
    }
    else
    {
        for (const auto& arg : wrong)
            std::cerr << "'" << arg.c_str() << "' isn't a valid argument\n";

        std::cout << usage_lines(cli, fmt) << std::endl;
    }

    return 0;
}
