#include <Ark/Ark.hpp>

namespace Ark
{
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

    void run(const std::string& file, bool debug, bool recompile)
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

        if (recompile || bcr.timestamp() == 0)  // couldn't read magic number, it's a source file
        {
            // check if it's in the arkscript cache
            std::string short_filename = Ark::Utils::getFilenameFromPath(file);
            std::string filename = short_filename.substr(0, short_filename.find_last_of('.')) + ".arkc";
            std::filesystem::path directory =  (std::filesystem::path(file)).parent_path() / ARK_CACHE_DIRNAME;
            std::string path = (directory / filename).string();

            if (!recompile && Ark::Utils::fileExists(path))
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
}