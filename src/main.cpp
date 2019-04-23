#ifndef ARKLANG  // define ARKLANG to avoid creating a main function when including Ark in a project

#include <iostream>

#include <clipp.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Lang/Program.hpp>

#include <chrono>

void exec(bool debug, bool timer, const std::string& file)
{
    Ark::Lang::Program program;
    program.feed(Ark::Utils::readFile(file));

    if (debug)
        std::cout << program << std::endl;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    program.execute();

    end = std::chrono::system_clock::now();
    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (timer)
        std::cout << "Took " << elapsed_microseconds << "us" << std::endl;
}

int main2(int argc, char** argv)
{
    using namespace clipp;

    std::cout << "Ark programming language" << std::endl << std::endl;

    enum class mode {help, version, execution};
    mode selected;
    std::string exec_file = "";
    bool debug = false;
    bool timer = false;
    std::vector<std::string> wrong;

    auto cli = (
        // general options
        option("-h", "--help").set(selected, mode::help).doc("Displays this help message")
        | option("--version").set(selected, mode::version).doc("Displays the Ark interpreter version and exits")
        | (
            value("file", exec_file).set(selected, mode::execution)
            , option("-t", "--time").set(timer).doc("Calculates time needed for the code to run")
            , option("-d", "--debug").set(debug).doc("Enables debug mode")
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

        case mode::execution:
            exec(debug, timer, exec_file);
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
