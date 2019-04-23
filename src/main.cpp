#ifndef ark_ark

#include <chrono>
#include <iostream>

#include <clipp.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Lang/Program.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/Log.hpp>

void exec(bool debug, bool timer, const std::string& file)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("(Interpreter) Can not find file '" + file + "'");
        return;
    }

    Ark::Lang::Program program(debug);
    program.feed(Ark::Utils::readFile(file));

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    program.execute();

    end = std::chrono::system_clock::now();
    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (timer)
        std::cout << "Took " << elapsed_microseconds << "us" << std::endl;
}

void compile(bool debug, const std::string& file)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("(Compiler) Can not find file '" + file + "'");
        return;
    }

    Ark::Compiler::Compiler compiler(debug);
    compiler.feed(Ark::Utils::readFile(file));
    compiler.compile();
    compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
}

int main(int argc, char** argv)
{
    using namespace clipp;

    std::cout << "Ark programming language" << std::endl << std::endl;

    enum class mode { help, version, interpreter, compiler };
    mode selected;
    std::string input_file = "";
    bool debug = false;
    bool timer = false;
    std::vector<std::string> wrong;

    auto cli = (
        // general options
        option("-h", "--help").set(selected, mode::help).doc("Display this help message")
        | option("--version").set(selected, mode::version).doc("Display Ark lang version and exit")
        | (
            value("file", input_file).set(selected, mode::interpreter)
            , option("-t", "--time").set(timer).doc("Calculate time needed for the code to run")
            , option("-d", "--debug").set(debug).doc("Enable debug mode")
          )
        | (command("compile").set(selected, mode::compiler).doc("Start the compiler to generate a bytecode file from the given Ark source file")
            , value("file", input_file)
            , option("-d", "--debug").set(debug).doc("Enable debug mode")
          )
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

        case mode::interpreter:
            exec(debug, timer, input_file);
            break;
        
        case mode::compiler:
            compile(debug, input_file);
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

#endif
