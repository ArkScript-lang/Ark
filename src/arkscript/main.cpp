#include <iostream>
#include <optional>
#include <filesystem>
#include <limits>

#include <clipp.h>
#define NOMINMAX
#include <termcolor/termcolor.hpp>
#include <spdlog/spdlog.h>

#include <Ark/Ark.hpp>
#include <Ark/REPL/Repl.hpp>
#include <Ark/Profiling.hpp>

enum class mode
{
    help,
    dev_info,
    bytecode_reader,
    version,
    run,
    repl,
    compile,
    eval
};

int main(int argc, char** argv)
{
    using namespace clipp;

    {
        namespace fs = std::filesystem;
        fs::path program(argv[0]);

        if (program.stem() == "ark")
            spdlog::warn("The command `ark' is being deprecated in favor of `arkscript'");
    }

    mode selected = mode::repl;
    uint16_t options = Ark::DefaultFeatures;

    std::string file = "",
                eval_expresion = "";

    unsigned debug = 0;
    constexpr uint16_t not_0 = std::numeric_limits<uint16_t>::max();

    uint16_t bcr_page = not_0;
    uint16_t bcr_start = not_0;
    uint16_t bcr_end = not_0;
    Ark::BytecodeSegment segment = Ark::BytecodeSegment::All;

    std::vector<std::string> wrong, script_args;

    std::string libdir = "";
    std::vector<std::string> libenv;

    // clang-format off
    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("-v", "--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            required("-e", "--eval").set(selected, mode::eval).doc("Evaluate ArkScript expression")
            & value("expression", eval_expresion)
        )
        | (
            required("-c", "--compile").set(selected, mode::compile).doc("Compile the given program to bytecode, but do not run")
            & value("file", file)
            , joinable(repeatable(option("-d", "--debug").call([&]{ debug++; }).doc("Increase debug level (default: 0)")))
        )
        | (
            required("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
            & value("file", file)
            , (
                option("-on", "--only-names").set(segment, Ark::BytecodeSegment::HeadersOnly).doc("Display only the bytecode segments names and sizes")
                | (
                    (
                        option("-a", "--all").set(segment, Ark::BytecodeSegment::All).doc("Display all the bytecode segments (default)")
                        | option("-st", "--symbols").set(segment, Ark::BytecodeSegment::Symbols).doc("Display only the symbols table")
                        | option("-vt", "--values").set(segment, Ark::BytecodeSegment::Values).doc("Display only the values table")
                    )
                    , option("-s", "--slice").doc("Select a slice of instructions in the bytecode")
                        & value("start", bcr_start)
                        & value("end", bcr_end)
                )
                | (
                    option("-cs", "--code").set(segment, Ark::BytecodeSegment::Code).doc("Display only the code segments")
                    , option("-p", "--page").doc("Set the bytecode reader code segment to display")
                        & value("page", bcr_page)
                )
            )
        )
        | (
            value("file", file).set(selected, mode::run)
            , (
                joinable(repeatable(option("-d", "--debug").call([&]{ debug++; })))
                ,
                // shouldn't change now, the lib option is fine and working
                (
                    option("-L", "--lib").doc("Set the location of the ArkScript standard library")
                    & value("lib_dir", libdir)
                )
            )
            , any_other(script_args)
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
        ;

    if (parse(argc, argv, cli) && wrong.empty())
    {
        using namespace Ark;

        if (!libdir.empty())
            libenv.push_back(libdir);

        switch (selected)
        {
            case mode::help:
                // clipp only supports streams
                std::cout << make_man_page(cli, "ark", fmt)
                                 .prepend_section("DESCRIPTION", "        ArkScript programming language")
                                 .append_section("LICENSE", "        Mozilla Public License 2.0")
                          << std::endl;
                break;

            case mode::version:
                std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << "\n";
                break;

            case mode::dev_info:
            {
                spdlog::debug(
                    "Have been compiled with {}, options: {}\n\n"
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
                    "\nMisc\n"
                    "    sizeof(vector<Ark::Value>) = {}B\n"
                    "    sizeof(std::string)   = {}B\n"
                    "    sizeof(String)        = {}B\n"
                    "    sizeof(char)          = {}B\n",
                    ARK_COMPILER, ARK_COMPILATION_OPTIONS,
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
                    // misc
                    sizeof(std::vector<Ark::Value>),
                    sizeof(std::string),
                    sizeof(String),
                    sizeof(char));
                break;
            }

            case mode::repl:
            {
                // send default features without FeatureRemoveUnusedVars to avoid deleting code which will be used later on
                Ark::Repl repl(Ark::DefaultFeatures & ~Ark::FeatureRemoveUnusedVars, libenv);
                return repl.run();
            }

            case mode::compile:
            {
                Ark::State state(options, libenv);
                state.setDebug(debug);

                if (!state.doFile(file))
                    return -1;

                break;
            }

            case mode::run:
            {
                Ark::State state(options, libenv);
                state.setDebug(debug);
                state.setArgs(script_args);

                if (!state.doFile(file))
                    return -1;

                Ark::VM vm(&state);
                int out = vm.run();

#ifdef ARK_PROFILER_COUNT
                spdlog::debug(
                    "Value\n"
                    "=====\n"
                    "\tCreations: {}\n\tCopies: {}\n\tMoves: {}\n\n\tCopy coeff: {}",
                    Ark::internal::value_creations,
                    Ark::internal::value_copies,
                    Ark::internal::value_moves,
                    static_cast<float>(Ark::internal::value_copies) / Ark::internal::value_creations);
#endif

                return out;
            }

            case mode::eval:
            {
                Ark::State state(options, libenv);
                state.setDebug(debug);

                if (!state.doString(eval_expresion))
                    return -1;

                Ark::VM vm(&state);
                return vm.run();
            }

            case mode::bytecode_reader:
            {
                try
                {
                    Ark::BytecodeReader bcr;
                    bcr.feed(file);

                    if (bcr_page == not_0 && bcr_start == not_0)
                        bcr.display(segment);
                    else if (bcr_page != not_0 && bcr_start == not_0)
                        bcr.display(segment, std::nullopt, std::nullopt, bcr_page);
                    else if (bcr_page == not_0 && bcr_start != not_0)
                        bcr.display(segment, bcr_start, bcr_end);
                    else
                        bcr.display(segment, bcr_start, bcr_end, bcr_page);
                }
                catch (const std::exception& e)
                {
                    spdlog::error("<BytecodeReader> {}", e.what());
                }
                break;
            }
        }
    }
    else
    {
        for (const auto& arg : wrong)
            std::cout << "'" << arg.c_str() << "' ins't a valid argument\n";

        // clipp only supports streams
        std::cout << make_man_page(cli, "ark", fmt)
                         .prepend_section("DESCRIPTION", "        ArkScript programming language")
                         .append_section("LICENSE", "        Mozilla Public License 2.0")
                  << std::endl;
    }

    return 0;
}
