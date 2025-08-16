#include <Ark/Builtins/Builtins.hpp>

#include <iostream>
#include <filesystem>
#include <fmt/core.h>

#include <Ark/Utils/Files.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Error/Exceptions.hpp>
#include <Ark/TypeChecker.hpp>

namespace Ark::internal::Builtins::IO
{
    /**
     * @name print
     * @brief Print value(s) in the terminal
     * @details No separator is put between the values. Adds a \n at the end
     * @param values the values to print
     * =begin
     * (print "hello")
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value print(std::vector<Value>& n, VM* vm)
    {
        for (const auto& value : n)
            fmt::print("{}", value.toString(*vm));
        fmt::println("");

        return nil;
    }

    /**
     * @name puts
     * @brief Print value(s) in the terminal
     * @details No separator is put between the values, no \n at the end
     * @param values the values to print
     * =begin
     * (puts "hello")
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value puts_(std::vector<Value>& n, VM* vm)
    {
        for (const auto& value : n)
            fmt::print("{}", value.toString(*vm));

        return nil;
    }

    /**
     * @name input
     * @brief Request a value from the user
     * @details Return the value as a string
     * @param prompt (optional) printed before asking for the user input
     * =begin
     * (input "put a number> ")
     * =end
     * @author https://github.com/SuperFola
     */
    // cppcheck-suppress constParameterReference
    Value input(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (types::check(n, ValueType::String))
            fmt::print("{}", n[0].string());
        else if (!n.empty())
            throw types::TypeCheckingError("input", { { types::Contract {}, types::Contract { { types::Typedef("prompt", ValueType::String) } } } }, n);

        std::string line;
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        std::getline(std::cin, line);
#else
        line = "fuzzer input";
#endif

        return Value(line);
    }

    // cppcheck-suppress constParameterReference
    Value writeFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (types::check(n, ValueType::String, ValueType::Any))
        {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            std::ofstream f(n[0].string());
            if (f.is_open())
            {
                f << n[1].toString(*vm);
                f.close();
            }
            else
                throw std::runtime_error(fmt::format("io:writeFile: couldn't write to file \"{}\"", n[0].stringRef()));
#endif
        }
        else
            throw types::TypeCheckingError(
                "io:writeFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("content", ValueType::Any) } } } },
                n);

        return nil;
    }

    // cppcheck-suppress constParameterReference
    Value appendToFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (types::check(n, ValueType::String, ValueType::Any))
        {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            std::ofstream f(n[0].string(), std::ios::out | std::ios::app);
            if (f.is_open())
            {
                f << n[1].toString(*vm);
                f.close();
            }
            else
                throw std::runtime_error(fmt::format("io:appendToFile: couldn't write to file \"{}\"", n[0].stringRef()));
#endif
        }
        else
            throw types::TypeCheckingError(
                "io:appendToFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("content", ValueType::Any) } } } },
                n);

        return nil;
    }

    // cppcheck-suppress constParameterReference
    Value readFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:readFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

        std::string filename = n[0].string();
        if (!Utils::fileExists(filename))
            throw std::runtime_error(
                fmt::format("io:readFile: couldn't read file \"{}\" because it doesn't exist", filename));

        return Value(Utils::readFile(filename));
    }

    // cppcheck-suppress constParameterReference
    Value fileExists(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:fileExists?",
                { { types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

        return Utils::fileExists(n[0].string()) ? trueSym : falseSym;
    }

    // cppcheck-suppress constParameterReference
    Value listFiles(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:listFiles",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        std::vector<Value> r;
        for (const auto& entry : std::filesystem::directory_iterator(n[0].string()))
            // cppcheck-suppress useStlAlgorithm
            // We can't use std::transform with a directory_iterator
            r.emplace_back(entry.path().string());

        return Value(std::move(r));
    }

    // cppcheck-suppress constParameterReference
    Value isDirectory(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:dir?",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        return (std::filesystem::is_directory(std::filesystem::path(n[0].string()))) ? trueSym : falseSym;
    }

    // cppcheck-suppress constParameterReference
    Value makeDir(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:makeDir",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        std::filesystem::create_directories(std::filesystem::path(n[0].string()));
        return nil;
    }

    // cppcheck-suppress constParameterReference
    Value removeFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            throw types::TypeCheckingError(
                "io:removeFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        std::filesystem::remove_all(std::filesystem::path(n[0].string()));
#endif
        return nil;
    }
}
