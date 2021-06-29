#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4996)
#endif
#include <stdlib.h>
#include <picosha2.hpp>
#include <termcolor.hpp>

namespace Ark
{
    State::State(uint16_t options, const std::string& libdir) noexcept :
        m_libdir(libdir), m_filename(ARK_NO_NAME_FILE),
        m_options(options), m_debug_level(0)
    {
        // read environment variable to locate ark std lib, *only* if the standard library folder wasn't provided
        // or if it doesn't exist
        if (m_libdir == "?" || m_libdir.size() == 0 || !Ark::Utils::fileExists(m_libdir))
        {
            // first, check in the environment variable, pointing to something like
            // /folder/where/ark/is
            // |___________________ ark
            // |___________________ lib/
            // |                    |___ std/
            // |                    |___ file.arkm
            // |                    |___ ...
            // |___________________ libArkReactor.so

            char* val = getenv("ARKSCRIPT_PATH");
            m_libdir = val == nullptr ? "" : std::string(val);

            // check that the environment variable does point to an existing folder
            if (m_libdir != "" && Ark::Utils::fileExists(m_libdir + "/lib"))
                m_libdir += "/lib";
            // check in the current working directory
            else if (Ark::Utils::fileExists("./lib"))
                m_libdir = Ark::Utils::canonicalRelPath("./lib");
        }
    }

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
            std::printf("%s\n", e.what());
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
            std::printf("%s\n", e.what());
        }

        return result;
    }

    bool State::compile(const std::string& file, const std::string& output)
    {
        Compiler compiler(m_debug_level, m_libdir, m_options);

        try
        {
            compiler.feed(Utils::readFile(file), file);
            for (auto& p : m_binded)
                compiler.m_defined_symbols.push_back(p.first);
            compiler.compile();

            if (output != "")
                compiler.saveTo(output);
            else
                compiler.saveTo(file.substr(0, file.find_last_of('.')) + ".arkc");
        }
        catch (const std::exception& e)
        {
            std::printf("%s\n", e.what());
            return false;
        }
        catch (...)
        {
            std::printf("Unknown lexer-parser-or-compiler error (%s)\n", file.c_str());
            return false;
        }

        return true;
    }

    bool State::doFile(const std::string& file)
    {
        if (!Ark::Utils::fileExists(file))
        {
            std::cerr << termcolor::red << "Can not find file '" << file << "'\n" << termcolor::reset;
            return false;
        }

        // check if it's a bytecode file or a source code file
        Ark::BytecodeReader bcr;
        try
        {
            bcr.feed(file);
        }
        catch (const std::exception& e)
        {
            std::printf("%s\n", e.what());
            return false;
        }

        if (bcr.timestamp() == 0)  // couldn't read magic number, it's a source file
        {
            // check if it's in the arkscript cache
            std::string short_filename = Ark::Utils::getFilenameFromPath(file);
            std::string filename = short_filename.substr(0, short_filename.find_last_of('.')) + ".arkc";
            std::filesystem::path directory =  (std::filesystem::path(file)).parent_path() / ARK_CACHE_DIRNAME;
            std::string path = (directory / filename).string();

            if (!std::filesystem::exists(directory))  // create ark cache directory
                std::filesystem::create_directory(directory);

            bool compiled_successfuly = compile(file, path);
            if (compiled_successfuly && feed(path))
                return true;
        }
        else if (feed(file)) // it's a bytecode file
            return true;
        return false;
    }

    bool State::doString(const std::string& code)
    {
        Compiler compiler(m_debug_level, m_libdir, m_options);

        try
        {
            compiler.feed(code);
            for (auto& p : m_binded)
                compiler.m_defined_symbols.push_back(p.first);
            compiler.compile();
        }
        catch (const std::exception& e)
        {
            std::printf("%s\n", e.what());
            return false;
        }
        catch (...)
        {
            std::printf("Unknown lexer-parser-or-compiler error\n");
            return false;
        }

        return feed(compiler.bytecode());
    }

    void State::loadFunction(const std::string& name, Value::ProcType function) noexcept
    {
        m_binded[name] = Value(std::move(function));
    }

    void State::setArgs(const std::vector<std::string>& args) noexcept
    {
        Value val(ValueType::List);
        for (const std::string& arg : args)
            val.push_back(Value(arg));
        m_binded["sys:args"] = val;

        m_binded["sys:platform"] = Value(ARK_PLATFORM_NAME);
    }

    void State::setDebug(unsigned level) noexcept
    {
        m_debug_level = level;
    }

    void State::setLibDir(const std::string& libDir) noexcept
    {
        m_libdir = libDir;
    }

    void State::configure()
    {
        using namespace internal;

        // configure tables and pages
        std::size_t i = 0;

        auto readNumber = [&, this] (std::size_t& i) -> uint16_t {
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
            std::string str_version = std::to_string(major) + "." +
                std::to_string(minor) + "." +
                std::to_string(patch);
            throwStateError("Compiler and VM versions don't match: " + str_version + " and " + ARK_VERSION_STR);
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

        std::vector<unsigned char> hash(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + i + picosha2::k_digest_size, m_bytecode.end(), hash);
        // checking integrity
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
            if (hash[j] != m_bytecode[i])
                throwStateError("Integrity check failed");
            ++i;
        }

        if (m_bytecode[i] == Instruction::SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_symbols.reserve(size);
            i++;

            for (uint16_t j = 0; j < size; ++j)
            {
                std::string symbol = "";
                while (m_bytecode[i] != 0)
                    symbol.push_back(m_bytecode[i++]);
                i++;

                m_symbols.push_back(symbol);
            }
        }
        else
            throwStateError("Couldn't find symbols table");

        if (m_bytecode[i] == Instruction::VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_constants.reserve(size);
            i++;

            for (uint16_t j = 0; j < size; ++j)
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
                    throwStateError("Unknown value type for value " + std::to_string(j));
            }
        }
        else
            throwStateError("Couldn't find constants table");

        while (m_bytecode[i] == Instruction::CODE_SEGMENT_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;

            m_pages.emplace_back();
            m_pages.back().reserve(size);

            for (uint16_t j = 0; j < size; ++j)
                m_pages.back().push_back(m_bytecode[i++]);
            
            if (i == m_bytecode.size())
                break;
        }
    }

    void State::reset() noexcept
    {
        m_symbols.clear();
        m_constants.clear();
        m_pages.clear();
        m_binded.clear(); 
    }
}

#ifdef _MSC_VER
    #pragma warning(pop)
#endif