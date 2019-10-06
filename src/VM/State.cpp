#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>

namespace Ark
{
    State::State(const std::string& libdir, const std::string& filename) :
        m_libdir(libdir == "" ? ARK_STD_DEFAULT : libdir), m_filename(filename)
    {}

    bool State::feed(const std::string& bytecode_filename)
    {
        bool result = true;
        try {
            Ark::BytecodeReader bcr;
            bcr.feed(bytecode_filename);
            m_bytecode = bcr.bytecode();

            m_filename = bytecode_filename;
        } catch (const std::exception& e) {
            result = false;
            std::cout << e.what() << std::endl;
        }

        return result;
    }

    bool State::feed(const bytecode_t& bytecode)
    {
        bool result = true;
        try {
            m_bytecode = bytecode;
        } catch (const std::exception& e) {
            result = false;
            std::cout << e.what() << std::endl;
        }

        return result;
    }

    bool State::doFile(const std::string& filename)
    {
        if (!Ark::Utils::fileExists(file))
        {
            Ark::logger.error("Can not find file '" + file + "'");
            return;
        }

        Ark::logger.data("doFile() launched on", file);

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

            Ark::logger.info("doFile() analyzing a source file");

            bool compiled_successfuly = false;

            if (Ark::Utils::fileExists(path))
            {
                Ark::logger.info("doFile() found the ark cache directory in", directory.string());

                auto ftime = std::filesystem::last_write_time(std::filesystem::path(file));

                // this shouldn't fail
                Ark::BytecodeReader bcr2;
                bcr2.feed(path);
                auto timestamp = bcr2.timestamp();
                auto file_last_write = static_cast<decltype(timestamp)>(std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());
                
                Ark::logger.data("doFile() the cached bytecode file is too old:", (file_last_write > timestamp));
                
                // recompile
                if (timestamp < file_last_write)
                    compiled_successfuly = Ark::compile(debug, file, path, m_libdir);
                else
                    compiled_successfuly = true;
            }
            else
            {
                Ark::logger.info("doFile() need to create the ark cache directory in", directory.string());

                if (!std::filesystem::exists(directory))  // create ark cache directory
                    std::filesystem::create_directory(directory);
                
                compiled_successfuly = Ark::compile(debug, file, path, m_libdir);
            }
            
            if (compiled_successfuly && feed(path))
                return true;
        }
        else if (feed(file)) // it's a bytecode file
            return true;
        return false;
    }

    void State::loadFunction(const std::string& name, internal::Value::ProcType function)
    {
        m_binded_functions[name] = std::move(function);
    }

    static bool compile(bool debug, const std::string& file, const std::string& output, const std::string& lib_dir)
    {
        Compiler compiler(debug, lib_dir);

        try {
            compiler.feed(Utils::readFile(file), file);
            compiler.compile();

            if (output != "")
                compiler.saveTo(output);
            else
                compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        } catch (...) {
            std::cerr << "Unknown lexer-parser-or-compiler error" << std::endl;
            return false;
        }

        return true;
    }
}