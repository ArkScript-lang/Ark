#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>
#include <Ark/Files.hpp>
#include <Ark/Compiler/Welder.hpp>

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif

#include <picosha2.h>
#include <termcolor/proxy.hpp>
#include <fmt/core.h>

namespace Ark
{
    State::State(const std::vector<std::filesystem::path>& libenv) noexcept :
        m_debug_level(0),
        m_libenv(libenv),
        m_filename(ARK_NO_NAME_FILE)
    {}

    bool State::feed(const std::string& bytecode_filename)
    {
        if (!Utils::fileExists(bytecode_filename))
            return false;

        return feed(Utils::readFileAsBytes(bytecode_filename));
    }

    bool State::feed(const bytecode_t& bytecode)
    {
        if (!checkMagic(bytecode))
            return false;

        m_bytecode = bytecode;

        try
        {
            configure();
            return true;
        }
        catch (const std::exception& e)  // FIXME I don't like this shit
        {
            std::cout << e.what() << std::endl;
            return false;
        }
    }

    bool State::compile(const std::string& file, const std::string& output)
    {
        Welder welder(m_debug_level, m_libenv);

        if (!welder.computeASTFromFile(file))
            return false;

        for (auto& p : m_binded)
            welder.registerSymbol(p.first);

        if (!welder.generateBytecode())
            return false;

        const std::string destination = output.empty() ? (file.substr(0, file.find_last_of('.')) + ".arkc") : output;
        if (!welder.saveBytecodeToFile(destination))
            return false;

        return true;
    }

    bool State::doFile(const std::string& file)
    {
        if (!Utils::fileExists(file))
        {
            std::cerr << termcolor::red << "Can not find file '" << file << "'\n"
                      << termcolor::reset;
            return false;
        }
        m_filename = file;

        bytecode_t bytecode = Utils::readFileAsBytes(file);
        if (!checkMagic(bytecode))  // couldn't read magic number, it's a source file
        {
            // check if it's in the arkscript cache
            const std::string short_filename = (std::filesystem::path(file)).filename().string();
            const std::string filename = short_filename.substr(0, short_filename.find_last_of('.')) + ".arkc";
            const std::filesystem::path directory = (std::filesystem::path(file)).parent_path() / ARK_CACHE_DIRNAME;
            const std::string path = (directory / filename).string();

            if (!exists(directory))  // create ark cache directory
                create_directory(directory);

            if (compile(file, path) && feed(path))
                return true;
        }
        else if (feed(bytecode))  // it's a bytecode file
            return true;
        return false;
    }

    bool State::doString(const std::string& code)
    {
        Welder welder(m_debug_level, m_libenv);

        if (!welder.computeASTFromString(code))
            return false;

        for (auto& p : m_binded)
            welder.registerSymbol(p.first);
        welder.generateBytecode();

        return feed(welder.bytecode());
    }

    void State::loadFunction(const std::string& name, Value::ProcType function) noexcept
    {
        m_binded[name] = Value(function);
    }

    void State::setArgs(const std::vector<std::string>& args) noexcept
    {
        Value val(ValueType::List);
        for (const std::string& arg : args)
            val.push_back(Value(arg));
        m_binded["sys:args"] = val;

        m_binded["sys:platform"] = Value(ARK_PLATFORM_NAME);
    }

    void State::setDebug(const unsigned level) noexcept
    {
        m_debug_level = level;
    }

    void State::setLibDirs(const std::vector<std::filesystem::path>& libenv) noexcept
    {
        m_libenv = libenv;
    }

    bool State::checkMagic(const bytecode_t& bytecode)
    {
        return (bytecode.size() > 4 && bytecode[0] == 'a' &&
                bytecode[1] == 'r' && bytecode[2] == 'k' &&
                bytecode[3] == internal::Instruction::NOP);
    }

    void State::configure()
    {
        // FIXME refactor this crap and try to mutualise with the bytecode reader??
        using namespace internal;

        // configure tables and pages
        std::size_t i = 0;

        auto readNumber = [&, this](std::size_t& i) -> uint16_t {
            auto x = (static_cast<uint16_t>(m_bytecode[i]) << 8);
            ++i;
            auto y = static_cast<uint16_t>(m_bytecode[i]);
            return x + y;
        };

        // read tables and check if bytecode is valid
        if (!(m_bytecode.size() > 4 && m_bytecode[i++] == 'a' &&
              m_bytecode[i++] == 'r' && m_bytecode[i++] == 'k' &&
              m_bytecode[i++] == Instruction::NOP))
            throwStateError("invalid format: couldn't find magic constant");

        uint16_t major = readNumber(i);
        i++;
        uint16_t minor = readNumber(i);
        i++;
        uint16_t patch = readNumber(i);
        i++;

        if (major != ARK_VERSION_MAJOR)
        {
            std::string str_version = std::to_string(major) + "." +
                std::to_string(minor) + "." +
                std::to_string(patch);
            throwStateError(fmt::format("Compiler and VM versions don't match: got {} while running {}", str_version, ARK_VERSION));
        }

        using timestamp_t = unsigned long long;
        timestamp_t timestamp [[maybe_unused]] = 0;
        auto aa = (static_cast<timestamp_t>(m_bytecode[i]) << 56),
             ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
             ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
             da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
             ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
             fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
             ga = (static_cast<timestamp_t>(m_bytecode[++i]) << 8),
             ha = (static_cast<timestamp_t>(m_bytecode[++i]));
        i++;
        timestamp = aa + ba + ca + da + ea + fa + ga + ha;

        std::vector<unsigned char> hash(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + i + picosha2::k_digest_size, m_bytecode.end(), hash);
        // checking integrity
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            if (hash[j] != m_bytecode[i])
                throwStateError("Integrity check failed");
#endif
            ++i;
        }

        if (m_bytecode[i] == SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_symbols.reserve(size);
            i++;

            for (uint16_t j = 0; j < size; ++j)
            {
                std::string symbol;
                while (m_bytecode[i] != 0)
                    symbol.push_back(m_bytecode[i++]);
                i++;

                m_symbols.push_back(symbol);
            }
        }
        else
            throwStateError("Couldn't find symbols table");

        if (m_bytecode[i] == VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            m_constants.reserve(size);
            i++;

            for (uint16_t j = 0; j < size; ++j)
            {
                uint8_t type = m_bytecode[i];
                i++;

                if (type == NUMBER_TYPE)
                {
                    std::string val;
                    while (m_bytecode[i] != 0)
                        val.push_back(m_bytecode[i++]);
                    i++;

                    m_constants.emplace_back(std::stod(val));
                }
                else if (type == STRING_TYPE)
                {
                    std::string val;
                    while (m_bytecode[i] != 0)
                        val.push_back(m_bytecode[i++]);
                    i++;

                    m_constants.emplace_back(val);
                }
                else if (type == FUNC_TYPE)
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

        while (m_bytecode[i] == CODE_SEGMENT_START)
        {
            i++;
            uint16_t size = readNumber(i) * 4;  // because the instructions are on 4 bytes
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
#    pragma warning(pop)
#endif
