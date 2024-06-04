#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>
#include <Ark/Files.hpp>
#include <Ark/Compiler/Welder.hpp>

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif

#include <picosha2.h>
#include <Ark/Compiler/BytecodeReader.hpp>
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
        BytecodeReader bcr;
        bcr.feed(bytecode);
        if (!bcr.checkMagic())
            return false;

        m_bytecode = bytecode;

        try
        {
            configure(bcr);
            return true;
        }
        catch (const std::exception& e)  // FIXME I don't like this shit
        {
            std::cout << e.what() << std::endl;
            return false;
        }
    }

    bool State::compile(const std::string& file, const std::string& output) const
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

        const bytecode_t bytecode = Utils::readFileAsBytes(file);
        BytecodeReader bcr;
        bcr.feed(bytecode);
        if (!bcr.checkMagic())  // couldn't read magic number, it's a source file
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

    void State::loadFunction(const std::string& name, const Value::ProcType function) noexcept
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

    void State::configure(const BytecodeReader& bcr)
    {
        using namespace internal;

        const auto [major, minor, patch] = bcr.version();
        if (major != ARK_VERSION_MAJOR)
        {
            std::string str_version = std::to_string(major) + "." +
                std::to_string(minor) + "." +
                std::to_string(patch);
            throwStateError(fmt::format("Compiler and VM versions don't match: got {} while running {}", str_version, ARK_VERSION));
        }

        const auto bytecode_hash = bcr.sha256();

        std::vector<unsigned char> hash(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + 18 + picosha2::k_digest_size, m_bytecode.end(), hash);
        // checking integrity
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            if (hash[j] != bytecode_hash[j])
                throwStateError("Integrity check failed");
#endif
        }

        // FIXME: we're going to read the symbols 3 times and the values twice
        // because code calls values which calls symbols
        m_symbols = bcr.symbols().symbols;
        m_constants = bcr.values().values;
        m_pages = bcr.code().pages;
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
