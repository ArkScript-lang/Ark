#ifndef ark_ark

#include <chrono>
#include <iostream>

#include <clipp.hpp>
#include <Ark/Ark.hpp>

void bcr(const std::string& file)
{
    try {
        Ark::BytecodeReader bcr;
        bcr.feed(file);
        bcr.display();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    using namespace clipp;

    enum class mode { help, dev_info, bytecode_reader, version, run, repl, compile };
    mode selected = mode::help;

    std::string file = "", lib_dir = "?";
    unsigned debug = 0;
    std::vector<std::string> wrong;
    uint16_t options = Ark::DefaultFeatures;

    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            (
                ( value("file", file).set(selected, mode::run)
                , option("-c", "--compile").set(selected, mode::compile).doc("Compile the given program to bytecode, but do not run")
                )
            | option("-r", "--repl").set(selected, mode::repl).doc("Run the ArkScript REPL")
            )
            // options taken by mode::run, mode::repl and mode::compile
            , (
                (
                    (
                        // options which can be cumulated (separated by commas)
                        joinable(repeatable(option("-d", "--debug").call([&]{ debug++; }).doc("Increase debug level (default: 0)")))
                    )
                    // other options which can not be cumulated, separated by pipes
                    | option("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
                ),
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
                    ( option("aitap"   ).call([&]{ options &= ~Ark::FeatureDisallowInvalidTokenAfterParen; })
                    | option("no-aitap").call([&]{ options |= Ark::FeatureDisallowInvalidTokenAfterParen; })
                    ).doc("Authorize invalid token after `(' (default: OFF). When ON, only display a warning")
                )
            )
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
                std::cout << make_man_page(cli, argv[0], fmt)
                            .append_section("LICENSE",
                                            "        Mozilla Public License 2.0")
                            .prepend_section("DESCRIPTION",
                                             "        ArkScript programming language");
                std::cout << std::endl;
                break;

            case mode::version:
                std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << std::endl;
                break;

            case mode::dev_info:
                std::cout << "Have been compiled with " << ARK_COMPILER << ", options: " << ARK_COMPILATION_OPTIONS << "\n\n";
                std::cout << "sizeof(Ark::Value)    = " << sizeof(Ark::internal::Value) << "B\n";
                std::cout << "sizeof(Ark::Frame)    = " << sizeof(Ark::internal::Frame) << "B\n";
                std::cout << "sizeof(Ark::State)    = " << sizeof(Ark::State) << "B\n";
                std::cout << "sizeof(Ark::Plugin)   = " << sizeof(Ark::internal::SharedLibrary) << "B\n";
                std::cout << "sizeof(Ark::Closure)  = " << sizeof(Ark::internal::Closure) << "B\n";
                std::cout << "sizeof(Ark::UserType) = " << sizeof(Ark::UserType) << "B\n";
                std::cout << "sizeof(Ark::VM)       = " << sizeof(Ark::VM) << "B\n";
                std::cout << "sizeof(vector<Ark::Value>) = " << sizeof(std::vector<Ark::internal::Value>) << "B\n";
                std::cout << "sizeof(std::string)   = " << sizeof(std::string) << "B\n";
                std::cout << "sizeof(char)          = " << sizeof(char) << "B\n";
                std::cout << std::endl;
                break;

            case mode::repl:
            {
                Ark::Repl repl(options, lib_dir);
                repl.run();
                break;
            }

            case mode::compile:
            {
                Ark::State state(lib_dir, options);
                state.setDebug(debug);

                if (!state.doFile(file))
                {
                    Ark::logger.error("Ark::State.doFile(" + file + ") failed");
                    return -1;
                }

                break;
            }
            
            case mode::run:
            {
                Ark::State state(lib_dir, options);
                state.setDebug(debug);

                if (!state.doFile(file))
                {
                    Ark::logger.error("Ark::State.doFile(" + file + ") failed");
                    return -1;
                }

                Ark::VM vm(&state);
                return vm.run();
            }

            case mode::bytecode_reader:
                bcr(file);
                break;
        }
    }
    else
    {
        for (const auto& arg : wrong)
            std::cerr << "'" << arg << "'" << " ins't a valid argument" << std::endl;
            
        std::cerr << "Usage:"   << std::endl << usage_lines(cli, argv[0], fmt) << std::endl
                  << "Options:" << std::endl << documentation(cli, fmt) << std::endl
                  << "LICENSE"  << std::endl << "        Mozilla Public License 2.0" << std::endl;
    }

    // to avoid some "CLI glitches"
    std::cout << termcolor::reset;
    return 0;
}

#endif
