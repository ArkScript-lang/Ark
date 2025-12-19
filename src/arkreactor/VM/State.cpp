#include <Ark/VM/State.hpp>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Compiler/Welder.hpp>

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4996)
#endif

#include <Proxy/Picosha2.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <fmt/core.h>
#include <fmt/color.h>

namespace Ark
{
    State::State(const std::vector<std::filesystem::path>& libenv) noexcept :
        m_debug_level(0),
        m_libenv(libenv),
        m_filename(ARK_NO_NAME_FILE),
        m_max_page_size(0)
    {
        // default value for builtin__sys:args is empty list
        const Value val(ValueType::List);
        m_binded[std::string(internal::Language::SysArgs)] = val;

        m_binded[std::string(internal::Language::SysProgramName)] = Value("");
    }

    bool State::feed(const std::string& bytecode_filename, const bool fail_with_exception)
    {
        if (!Utils::fileExists(bytecode_filename))
            return false;

        return feed(Utils::readFileAsBytes(bytecode_filename), fail_with_exception);
    }

    bool State::feed(const bytecode_t& bytecode, const bool fail_with_exception)
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
        catch (const std::exception& e)
        {
            if (fail_with_exception)
                throw;

            fmt::println("{}", e.what());
            return false;
        }
    }

    bool State::compile(const std::string& file, const std::string& output, const uint16_t features) const
    {
        Welder welder(m_debug_level, m_libenv, features);
        for (const auto& p : m_binded)
            welder.registerSymbol(p.first);

        if (!welder.computeASTFromFile(file))
            return false;
        if (!welder.generateBytecode())
            return false;

        const std::string destination = output.empty() ? (file.substr(0, file.find_last_of('.')) + ".arkc") : output;
        if (!welder.saveBytecodeToFile(destination))
            return false;

        return true;
    }

    bool State::doFile(const std::string& file_path, const uint16_t features)
    {
        if (!Utils::fileExists(file_path))
        {
            fmt::print(fmt::fg(fmt::color::red), "Can not find file '{}'\n", file_path);
            return false;
        }
        m_filename = file_path;
        m_binded[std::string(internal::Language::SysProgramName)] = Value(std::filesystem::path(m_filename).filename().string());

        const bytecode_t bytecode = Utils::readFileAsBytes(file_path);
        BytecodeReader bcr;
        bcr.feed(bytecode);
        if (!bcr.checkMagic())  // couldn't read magic number, it's a source file
        {
            // check if it's in the arkscript cache
            const std::string filename = std::filesystem::path(file_path).filename().replace_extension(".arkc").string();
            const std::filesystem::path cache_directory = std::filesystem::path(file_path).parent_path() / ARK_CACHE_DIRNAME;
            const std::string bytecode_path = (cache_directory / filename).string();

            if (!exists(cache_directory))
                create_directory(cache_directory);

            if (compile(file_path, bytecode_path, features) && feed(bytecode_path))
                return true;
        }
        else if (feed(bytecode))  // it's a bytecode file
            return true;
        return false;
    }

    bool State::doString(const std::string& code, const uint16_t features)
    {
        Welder welder(m_debug_level, m_libenv, features);
        for (const auto& p : m_binded)
            welder.registerSymbol(p.first);

        if (!welder.computeASTFromString(code))
            return false;
        if (!welder.generateBytecode())
            return false;
        return feed(welder.bytecode());
    }

    void State::loadFunction(const std::string& name, Procedure::CallbackType&& function) noexcept
    {
        m_binded[name] = Value(std::move(function));
    }

    void State::setArgs(const std::vector<std::string>& args) noexcept
    {
        Value val(ValueType::List);
        std::ranges::transform(args, std::back_inserter(val.list()), [](const std::string& arg) {
            return Value(arg);
        });

        m_binded[std::string(internal::Language::SysArgs)] = val;
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
            const std::string str_version = fmt::format("{}.{}.{}", major, minor, patch);
            throwStateError(fmt::format("Compiler and VM versions don't match: got {} while running {}", str_version, ARK_VERSION));
        }

        const auto bytecode_hash = bcr.sha256();

        std::vector<unsigned char> hash(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + bytecode::HeaderSize + picosha2::k_digest_size, m_bytecode.end(), hash);
        // checking integrity
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            if (hash[j] != bytecode_hash[j])
                throwStateError("Integrity check failed");
#endif
        }

        const auto syms = bcr.symbols();
        const auto vals = bcr.values(syms);
        const auto files = bcr.filenames(vals);
        const auto inst_locs = bcr.instLocations(files);
        const auto [pages, _] = bcr.code(inst_locs);

        m_symbols = syms.symbols;
        m_constants = vals.values;
        m_filenames = files.filenames;
        m_inst_locations = inst_locs.locations;

        m_max_page_size = 0;
        for (const bytecode_t& page : pages)
        {
            if (page.size() > m_max_page_size)
                m_max_page_size = page.size();
        }

        // Make m_code as a big contiguous chunk of instructions,
        // aligned on the biggest page size.
        // This might have a downside when we have a single big page and
        // a bunch of smaller ones, though I couldn't measure it while testing.
        m_code.resize(m_max_page_size * pages.size(), Instruction::NOP);
        for (std::size_t i = 0, end = pages.size(); i < end; ++i)
        {
            for (std::size_t j = 0, end_j = pages[i].size(); j < end_j; ++j)
                m_code[i * m_max_page_size + j] = pages[i][j];
        }
    }

    void State::reset() noexcept
    {
        m_symbols.clear();
        m_constants.clear();
        m_filenames.clear();
        m_inst_locations.clear();
        m_max_page_size = 0;
        m_code.clear();
        m_binded.clear();

        // default value for builtin__sys:args is empty list
        const Value val(ValueType::List);
        m_binded[std::string(internal::Language::SysArgs)] = val;

        m_binded[std::string(internal::Language::SysProgramName)] = Value("");
    }
}

#ifdef _MSC_VER
#    pragma warning(pop)
#endif
