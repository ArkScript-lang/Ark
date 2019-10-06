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
        try
        {
            Ark::BytecodeReader bcr;
            bcr.feed(bytecode_filename);
            m_bytecode = bcr.bytecode();

            m_filename = bytecode_filename;
            configure();
        }
        catch (const std::exception& e)
        {
            result = false;
            std::cout << e.what() << std::endl;
        }

        return result;
    }

    bool State::feed(const bytecode_t& bytecode)
    {
        bool result = true;
        try
        {
            m_bytecode = bytecode;
            configure();
        }
        catch (const std::exception& e)
        {
            result = false;
            std::cout << e.what() << std::endl;
        }

        return result;
    }

    static bool compile(bool debug, const std::string& file, const std::string& output, const std::string& lib_dir)
    {
        Compiler compiler(debug, lib_dir);

        try
        {
            compiler.feed(Utils::readFile(file), file);
            compiler.compile();

            if (output != "")
                compiler.saveTo(output);
            else
                compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }
        catch (...)
        {
            std::cerr << "Unknown lexer-parser-or-compiler error" << std::endl;
            return false;
        }

        return true;
    }

    bool State::doFile(const std::string& file)
    {
        if (!Ark::Utils::fileExists(file))
        {
            Ark::logger.error("Can not find file '" + file + "'");
            return;
        }

        Ark::logger.data("doFile() launched on", file);

        // check if it's a bytecode file or a source code file
        Ark::BytecodeReader bcr;
        try
        {
            bcr.feed(file);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            return false;
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

    void State::configure()
    {
        using namespace Ark::internal;

        // configure tables and pages
        std::size_t i = 0;

        auto readNumber = [&m_bytecode] (std::size_t& i) -> uint16_t {
            uint16_t x = (static_cast<uint16_t>(m_bytecode[i]) << 8); ++i;
            uint16_t y = static_cast<uint16_t>(m_bytecode[i]);
            return x + y;
        };

        // read tables and check if bytecode is valid
        if (!(m_bytecode.size() > 4 && m_bytecode[i++] == 'a' &&
            m_bytecode[i++] == 'r' && m_bytecode[i++] == 'k' &&
            m_bytecode[i++] == Instruction::NOP))
            throwStateError("invalid format: couldn't find magic constant");

        uint16_t major = readNumber(i); i++;
        uint16_t minor = readNumber(i); i++;
        uint16_t patch = readNumber(i); i++;
        
        if (major != ARK_VERSION_MAJOR)
        {
            std::string str_version = Ark::Utils::toString(major) + "." +
                Ark::Utils::toString(minor) + "." +
                Ark::Utils::toString(patch);
            std::string builtin_version = Ark::Utils::toString(ARK_VERSION_MAJOR) + "." +
                Ark::Utils::toString(ARK_VERSION_MINOR) + "." +
                Ark::Utils::toString(ARK_VERSION_PATCH);
            throwStateError("Compiler and VM versions don't match: " + str_version + " and " + builtin_version);
        }

        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 0;
        auto aa = (static_cast<timestamp_t>(m_bytecode[  i]) << 56),
            ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
            ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
            da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
            ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
            fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
            ga = (static_cast<timestamp_t>(m_bytecode[++i]) <<  8),
            ha = (static_cast<timestamp_t>(m_bytecode[++i]));
        i++;
        timestamp = aa + ba + ca + da + ea + fa + ga + ha;

        if (m_bytecode[i] == Instruction::SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_symbols.reserve(size);
            i++;

            for (uint16_t j=0; j < size; ++j)
            {
                std::string symbol = "";
                while (m_bytecode[i] != 0)
                    symbol.push_back(m_bytecode[i++]);
                i++;

                m_symbols.push_back(symbol);
            }
        }
        else
            throwStateError("couldn't find symbols table");

        if (m_bytecode[i] == Instruction::VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_constants.reserve(size);
            i++;

            for (uint16_t j=0; j < size; ++j)
            {
                uint8_t type = m_bytecode[i];
                i++;

                if (type == Instruction::NUMBER_TYPE)
                {
                    std::string val = "";
                    while (m_bytecode[i] != 0)
                        val.push_back(m_bytecode[i++]);
                    i++;

                    m_constants.emplace_back(std::stod(val));
                }
                else if (type == Instruction::STRING_TYPE)
                {
                    std::string val = "";
                    while (m_bytecode[i] != 0)
                        val.push_back(m_bytecode[i++]);
                    i++;

                    m_constants.emplace_back(val);
                }
                else if (type == Instruction::FUNC_TYPE)
                {
                    uint16_t addr = readNumber(i);
                    i++;
                    m_constants.emplace_back(addr);
                    i++;  // skip NOP
                }
                else
                    throwStateError("unknown value type for value " + Ark::Utils::toString(j));
            }
        }
        else
            throwStateError("couldn't find constants table");

        if (m_bytecode[i] == Instruction::PLUGIN_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_plugins.reserve(size);
            i++;

            for (uint16_t j=0; j < size; ++j)
            {
                std::string plugin = "";
                while (m_bytecode[i] != 0)
                    plugin.push_back(m_bytecode[i++]);
                i++;

                m_plugins.push_back(plugin);
            }
        }
        else
            throwStateError("couldn't find plugins table");
        
        while (m_bytecode[i] == Instruction::CODE_SEGMENT_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;

            m_pages.emplace_back();
            m_pages.back().reserve(size);

            for (uint16_t j=0; j < size; ++j)
                m_pages.back().push_back(m_bytecode[i++]);
            
            if (i == m_bytecode.size())
                break;
        }
    }
}