#ifndef ark_ark

#include <chrono>
#include <iostream>

#include <clipp.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Compiler/Compiler.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Log.hpp>

void compile(bool debug, bool timer, const std::string& file, const std::string& output)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("[Compiler] Can not find file '" + file + "'");
        return;
    }

    Ark::Compiler compiler(debug);
    compiler.feed(Ark::Utils::readFile(file), file);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    compiler.compile();

    if (output != "")
        compiler.saveTo(output);
    else
        compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");

    end = std::chrono::system_clock::now();
    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (timer)
        std::cout << "Compiler took " << elapsed_microseconds << "us" << std::endl;
}

void bcr(const std::string& file)
{
    Ark::BytecodeReader bcr;
    bcr.feed(file);
    bcr.display();
}

void vm(bool debug, bool timer, const std::string& file, bool count_fcall)
{
    if (!Ark::Utils::fileExists(file))
    {
        Ark::logger.error("[Virtual Machine] Can not find file '" + file + "'");
        return;
    }

    Ark::VM vm(debug, count_fcall);
    vm.feed(file);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    vm.run();

    end = std::chrono::system_clock::now();
    auto elapsed_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (timer)
        std::cout << "VM took " << elapsed_microseconds << "us" << std::endl;
}

int main(int argc, char** argv)
{
    using namespace clipp;

    bool help_ = false,
         version_ = false,
         compiler_ = false,
         vm_ = false,
         bcr_ = false,
         dev_info_ = false;

    std::string input_file = "", output_file = "";
    bool debug = false, timer = false, count_fcall = false;
    std::vector<std::string> wrong;

    auto cli = (
        // general options
        option("-h", "--help").set(help_).doc("Display this help message")
        | option("--version").set(version_).doc("Display Ark lang version and exit")
        | option("--dev-info").set(dev_info_).doc("Display development informations and exit")
        | (value("file", input_file)
            , option("-c", "--compile").set(compiler_).doc("Compile file")
            , option("-o", "--output").doc("Set the output filename for the compiler") & value("out", output_file)
            , (
                option("-vm").set(vm_).doc("Start the VM on the given file")
                , option("--count-fcalls").set(count_fcall).doc("Count functions calls and display result at the end of the execution")
              )
            , option("-bcr", "--bytecode-reader").set(bcr_).doc("Launch the bytecode reader")
            , option("-d", "--debug").set(debug).doc("Trigger debug mode")
            , option("-t", "--time").set(timer).doc("Launch a timer")
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
        if (help_)
        {
            std::cerr << make_man_page(cli, argv[0], fmt).append_section("LICENSE", "        Mozilla Public License 2.0")
                      << std::endl;
            return 0;
        }

        if (version_)
        {
            std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << std::endl;
            return 0;
        }

        if (dev_info_)
        {
            std::cout << ARK_COMPILER << " " << ARK_COMPILATION_OPTIONS << "\n";
            std::cout << "sizeof(Ark::Value) [VM] = " << sizeof(Ark::internal::Value) << "B\n";
            std::cout << "sizeof(Ark::Frame) [VM] = " << sizeof(Ark::internal::Frame) << "B\n";
            std::cout << "sizeof(Ark::Closure)    = " << sizeof(Ark::internal::Closure) << "B\n";
            std::cout << "sizeof(Ark::VM)         = " << sizeof(Ark::VM) << "B\n";
            std::cout << "sizeof(char)            = " << sizeof(char) << "B\n";
            std::cout << std::endl;
            return 0;
        }

        if (compiler_)
            compile(debug, timer, input_file, output_file);

        if (bcr_)
        {
            if (output_file != "")
                bcr(output_file);
            else if (compiler_ && output_file == "")
                bcr(input_file.substr(0, input_file.find_last_of('.')) + ".arkc");
            else
                bcr(input_file);
        }

        if (vm_)
        {
            if (output_file != "")
                vm(debug, timer, output_file, count_fcall);
            else if (output_file == "" && compiler_)
                vm(debug, timer, input_file.substr(0, input_file.find_last_of('.')) + ".arkc", count_fcall);
            else
                vm(debug, timer, input_file, count_fcall);
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
