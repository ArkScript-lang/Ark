#ifndef ark_ark

#include <cstdio>
#include <iostream>
#include <optional>

#include <clipp.hpp>
#include <termcolor.hpp>

#include <Ark/Ark.hpp>
#include <Ark/REPL/Repl.hpp>
#include <Ark/Profiling.hpp>

int main(int argc, char** argv)
{
    using namespace clipp;

    enum class mode { help, dev_info, bytecode_reader, version, run, repl, compile, eval };
    mode selected = mode::repl;
    uint16_t options = Ark::DefaultFeatures;

    std::string file = "",
                lib_dir = "?",
                eval_expresion = "";

    unsigned debug = 0;

    uint16_t bcr_page  = ~0,
             bcr_start = ~0,
             bcr_end   = ~0;
    Ark::BytecodeSegment segment = Ark::BytecodeSegment::All;

    std::vector<std::string> wrong, script_args;

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
                    & value("lib_dir", lib_dir)
                ),
                // feature flags
                with_prefix("-f",
                    // a single feature should always be defined with an ON and an OFF version, and documentation
                    // all features must be separated by commas as following
                    // exclusing feature flags (ON/OFF) should be separated by pipes
                    ( option("fac"   ).call([&]{ options |= Ark::FeatureFunctionArityCheck; })
                    | option("no-fac").call([&]{ options &= ~Ark::FeatureFunctionArityCheck; })
                    ).doc("Toggle function arity checks (default: ON)")
                    ,
                    ( option("ruv"   ).call([&]{ options |= Ark::FeatureRemoveUnusedVars; })
                    | option("no-ruv").call([&]{ options &= ~Ark::FeatureRemoveUnusedVars; })
                    ).doc("Remove unused variables (default: ON)")
                )
            )
            , any_other(script_args)
        )
        , any_other(wrong)
    );

    auto fmt = doc_formatting{}
        .start_column(8)           // column where usage lines and documentation starts
        .doc_column(36)            // parameter docstring start col
        .indent_size(2)            // indent of documentation lines for children of a documented group
        .split_alternatives(true)  // split usage into several lines for large alternatives
        .merge_alternative_flags_with_common_prefix(true)  // [-fok] [-fno-ok] becomes [-f(ok|no-ok)]
    ;

    if (parse(argc, argv, cli) && wrong.empty())
    {
        using namespace Ark;

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
                std::printf("Version %i.%i.%i\n", ARK_VERSION_MAJOR, ARK_VERSION_MINOR, ARK_VERSION_PATCH);
                break;

            case mode::dev_info:
            {
                std::printf(
                    "Have been compiled with %s, options: %s\n\n"
                    "sizeof(Ark::Value)    = %zuB\n"
                    "      sizeof(Value_t) = %zuB\n"
                    "      sizeof(ValueType) = %zuB\n"
                    "      sizeof(ProcType)  = %zuB\n"
                    "      sizeof(Ark::Closure)  = %zuB\n"
                    "      sizeof(Ark::UserType) = %zuB\n"
                    "\nVirtual Machine\n"
                    "sizeof(Ark::VM)       = %zuB\n"
                    "      sizeof(Ark::State)    = %zuB\n"
                    "      sizeof(Ark::Scope)    = %zuB\n"
                    "\nMisc\n"
                    "    sizeof(vector<Ark::Value>) = %zuB\n"
                    "    sizeof(std::string)   = %zuB\n"
                    "    sizeof(String)        = %zuB\n"
                    "    sizeof(char)          = %zuB\n"
                    , ARK_COMPILER, ARK_COMPILATION_OPTIONS,
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
                        sizeof(char)
                );
                break;
            }

            case mode::repl:
            {
                // send default features without FeatureRemoveUnusedVars to avoid deleting code which will be used later on
                Ark::Repl repl(Ark::DefaultFeatures & ~Ark::FeatureRemoveUnusedVars, lib_dir);
                return repl.run();
            }

            case mode::compile:
            {
                Ark::State state(options, lib_dir);
                state.setDebug(debug);

                if (!state.doFile(file))
                {
                    std::cerr << termcolor::red << "Ark::State.doFile(" << file << ") failed\n" << termcolor::reset;
                    return -1;
                }

                break;
            }

            case mode::run:
            {
                Ark::State state(options, lib_dir);
                state.setDebug(debug);
                state.setArgs(script_args);

                if (!state.doFile(file))
                {
                    std::cerr << termcolor::red << "Ark::State.doFile(" << file << ") failed\n" << termcolor::reset;
                    return -1;
                }

                Ark::VM vm(&state);
                int out = vm.run();

                #ifdef ARK_PROFILER_COUNT
                std::printf(
                    "\n\nValue\n"
                    "=====\n"
                    "\tCreations: %u\n\tCopies: %u\n\tMoves: %u\n\n\tCopy coeff: %f",
                    Ark::internal::value_creations,
                    Ark::internal::value_copies,
                    Ark::internal::value_moves,
                    static_cast<float>(Ark::internal::value_copies) / Ark::internal::value_creations
                );
                #endif

                return out;
            }

            case mode::eval:
            {
                Ark::State state(options, lib_dir);
                state.setDebug(debug);

                if (!state.doString(eval_expresion))
                {
                    std::cerr << termcolor::red << "Ark::State.doString(" << eval_expresion << ") failed\n" << termcolor::reset;
                    return  -1;
                }

                Ark::VM vm(&state);
                return vm.run();
            }

            case mode::bytecode_reader:
            {
                uint16_t not_0 = ~0;
                try {
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
                } catch (const std::exception& e) {
                    std::printf("%s\n", e.what());
                }
                break;
            }
        }
    }
    else
    {
        for (const auto& arg : wrong)
            std::printf("'%s' ins't a valid argument\n", arg.c_str());

        // clipp only supports streams
        std::cout << make_man_page(cli, "ark", fmt)
                    .prepend_section("DESCRIPTION", "        ArkScript programming language")
                    .append_section("LICENSE", "        Mozilla Public License 2.0")
                    << std::endl;
    }

    return 0;
}

#endif
