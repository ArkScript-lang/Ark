#include <iostream>

#include <clipp.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Parser/Lexer.hpp>

void test()
{
    Ark::Parser::Lexer lexer;
    lexer.feed(Ark::Utils::readFile("tests/1"));
    lexer.check();
}

int main(int argc, char** argv)
{
    using namespace clipp;

    std::cout << "Ark programming language" << std::endl << std::endl;

    enum class mode {help, version, test};
    mode selected;
    // related to the compilers
    std::vector<std::string> input_files;
    std::string output_file = "";
    // related to the KVM
    std::string exec_file = "";
    std::vector<std::string> bytecode_args;
    // general flags and stuff
    bool debug = false
        , experimental = false;
    std::vector<std::string> wrong;

    auto cli = (
        // general options
        option("-h", "--help").set(selected, mode::help).doc("Displays this help message")
        | option("--version").set(selected, mode::version).doc("Displays the Ark interpreter version and exits")
        | command("test").set(selected, mode::test).doc("Testing mode, to test stuff")
        /*// sub-programs
        | (
            // Compilers
            (
                // Kafe compiler
                (command("kafec").set(selected, mode::kafec).doc("Launch the Kafe compiler")
                  )
                // KASM compiler
                | (command("kasm").set(selected, mode::kasm).doc("Launch the Kafe ASM compiler")
                  )
                , values("files", input_files)
                , required("-o", "--out") & value("output", output_file)
              )
            // KVM
            | (command("kvm").set(selected, mode::kvm).doc("Launch the Kafe Virtual Machine")
                , value("file", exec_file)
                , values([](const std::string& arg){ return true; }, "args", bytecode_args)
              )
            // REPL
            | (
                // KASM REPL
                (command("repl-kasm").set(selected, mode::repl_kasm).doc("Start a REPL for the KASM language")
                  )
                // Kafe REPL
                | (command("repl-kafe").set(selected, mode::repl_kafe).doc("Start a REPL for the Kafe language")
                  )
                // options related to all the REPL
                , value("-c").set(repl_code).doc("Program passed in as string")
              )
            // options attached to all those sub-programs
            , option("-d", "--debug").set(debug).doc("Enable debug mode")
            , option("-E", "--experimental").set(experimental).doc("Enable experimental features")
          )
        // cup, the package manager
        | (command("cup").set(selected, mode::cup).doc("Starts cup, the package manager")
          )*/
        , any_other(wrong)
    );

    auto fmt = doc_formatting{}
        .start_column(8)           // column where usage lines and documentation starts
        .doc_column(36)            // parameter docstring start col
        .indent_size(2)            // indent of documentation lines for children of a documented group
        .split_alternatives(true)  // split usage into several lines for large alternatives
    ;

    if (parse(argc, argv, cli) && wrong.empty())
    {
        switch (selected)
        {
        default:
        case mode::help:
            std::cerr << make_man_page(cli, argv[0], fmt).append_section("LICENSE", "        Mozilla Public License 2.0")
                      << std::endl;
            return 0;

        case mode::version:
            std::cout << "Version " << Ark::Version::Major << "." << Ark::Version::Minor << "." << Ark::Version::Patch << std::endl;
            break;
        
        case mode::test:
            test();
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
    
    return 0;
}