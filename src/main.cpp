#ifndef ark_ark

#include <chrono>
#include <iostream>

#include <clipp.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Log.hpp>

void compile(bool debug, const std::string& file, const std::string& output)
{
    Ark::Compiler compiler(debug);
    compiler.feed(Ark::Utils::readFile(file), file);

    compiler.compile();

    if (output != "")
        compiler.saveTo(output);
    else
        compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
}

void vm(bool debug, const std::string& file)
{
    if (debug)
    {
        Ark::VM_debug vm;
        vm.feed(file);
        vm.run();
    }
    else
    {
        Ark::VM vm;
        vm.feed(file);
        vm.run();
    }
}

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

void run(const std::string& file, bool debug)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("Can not find file '" + file + "'");
        return;
    }

    // check if it's a bytecode file or a source code file
    Ark::BytecodeReader bcr;
    try {
        bcr.feed(file);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return;
    }

    if (bcr.timestamp() == 0)  // couldn't read magic number, it's a source file
    {
        // check if it's in the arkscript cache
        std::string short_filename = Ark::Utils::getFilenameFromPath(file);
        std::string filename = short_filename.substr(0, short_filename.find_last_of('.')) + ".arkc";
        std::filesystem::path directory =  (std::filesystem::path(file)).parent_path() / ARK_CACHE_DIRNAME;
        std::string path = (directory / filename).string();

        if (Ark::Utils::fileExists(path))
        {
            auto ftime = std::filesystem::last_write_time(directory / filename);

            // this shouldn't fail
            Ark::BytecodeReader bcr2;
            bcr2.feed(path);
            auto timestamp = bcr.timestamp();
            auto file_last_write = static_cast<decltype(timestamp)>(ftime.time_since_epoch().count());
            // recompile
            if (timestamp < file_last_write)
                compile(debug, file, path);
            vm(debug, path);
        }
        else
        {
            if (!std::filesystem::exists(directory))  // create ark cache directory
                std::filesystem::create_directory(directory);
            
            compile(debug, file, path);
            vm(debug, path);
        }
    }
    else  // it's a bytecode file, run it
        vm(debug, file);
}

int main(int argc, char** argv)
{
    using namespace clipp;

    enum class mode { help, dev_info, bytecode_reader, version, run };
    mode selected = mode::help;

    std::string file = "";
    bool debug = false;
    std::vector<std::string> wrong;

    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            value("file", file).set(selected, mode::run)
            , (
                (
                    option("-d", "--debug").set(debug).doc("Enable debug mode")
                    // , option("-t", "--time").set(timer).doc("Enable timer")
                )
                | option("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
            )
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
            case mode::help:
                std::cerr << make_man_page(cli, argv[0], fmt).append_section(
                    "LICENSE",
                    "        Mozilla Public License 2.0"
                    ) << std::endl;
                break;
            
            case mode::version:
                std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << std::endl;
                break;
            
            case mode::dev_info:
                std::cout << ARK_COMPILER << " " << ARK_COMPILATION_OPTIONS << "\n";
                std::cout << "sizeof(Ark::Value) [VM] = " << sizeof(Ark::internal::Value) << "B\n";
                std::cout << "sizeof(Ark::Frame) [VM] = " << sizeof(Ark::internal::Frame) << "B\n";
                std::cout << "sizeof(Ark::Closure)    = " << sizeof(Ark::internal::Closure) << "B\n";
                std::cout << "sizeof(Ark::VM)         = " << sizeof(Ark::VM) << "B\n";
                std::cout << "sizeof(char)            = " << sizeof(char) << "B\n";
                std::cout << std::endl;
                break;
            
            case mode::run:
                run(file, debug);
                break;
            
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
