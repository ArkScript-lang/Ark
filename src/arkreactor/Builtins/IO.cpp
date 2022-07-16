#include <Ark/Builtins/Builtins.hpp>

#include <cstdio>
#include <iostream>
#include <filesystem>

#include <Ark/Files.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Exceptions.hpp>
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
    Value print(std::vector<Value>& n, VM* vm)
    {
        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
            it->toString(std::cout, *vm);
        std::cout << '\n';

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
    Value puts_(std::vector<Value>& n, VM* vm)
    {
        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
            it->toString(std::cout, *vm);

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
    Value input(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (types::check(n, ValueType::String))
            std::printf("%s", n[0].string().c_str());
        else if (n.size() != 0)
            types::generateError("input", { { types::Contract {}, types::Contract { { types::Typedef("prompt", ValueType::String) } } } }, n);

        std::string line = "";
        std::getline(std::cin, line);

        return Value(line);
    }

    /**
     * @name io:writeFile
     * @brief Write content to a file, given an optional mode (default: "w"). Return nil
     * @param filename path to the file to open
     * @param mode (optional), either "a" (append) or "w" (write)
     * @param content can be any valid ArkScript value
     * =begin
     * (io:writeFile "hello.json" "{\"key\": 12}")
     * (io:writeFile "truc.txt" "a" 12)
     * =end
     * @author https://github.com/SuperFola
     */
    Value writeFile(std::vector<Value>& n, VM* vm)
    {
        if (types::check(n, ValueType::String, ValueType::Any))
        {
            std::ofstream f(n[0].string().c_str());
            if (f.is_open())
            {
                n[1].toString(f, *vm);
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].stringRef().toString() + "\"");
        }
        else if (types::check(n, ValueType::String, ValueType::String, ValueType::Any))
        {
            auto mode = n[1].string();
            if (mode != "w" && mode != "a")
                throw std::runtime_error("io:writeFile: mode must be equal to \"a\" or \"w\"");

            auto ios_mode = std::ios::out | std::ios::trunc;
            if (mode == "a")
                ios_mode = std::ios::out | std::ios::app;

            std::ofstream f(n[0].string().c_str(), ios_mode);
            if (f.is_open())
            {
                n[2].toString(f, *vm);
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].stringRef().toString() + "\"");
        }
        else
            types::generateError(
                "io:writeFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("content", ValueType::Any) } },
                    types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("mode", ValueType::String), types::Typedef("content", ValueType::Any) } } } },
                n);

        return nil;
    }

    /**
     * @name io:readFile
     * @brief Read the content from a file as a String
     * @param filename the path of the file to read
     * =begin
     * (io:readFile "hello.json")
     * =end
     * @author https://github.com/SuperFola
     */
    Value readFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "io:readFile",
                { { types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

        auto filename = n[0].string().c_str();
        if (!Utils::fileExists(filename))
            throw std::runtime_error("Couldn't read file \"" + std::string(filename) + "\": it doesn't exist");

        return Value(Utils::readFile(filename));
    }

    /**
     * @name io:fileExists?
     * @brief Check if a file exists, return True or False
     * @param filename the path of the file
     * =begin
     * (io:fileExists? "hello.json")
     * =end
     * @author https://github.com/SuperFola
     */
    Value fileExists(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "io:fileExists?",
                { { types::Contract { { types::Typedef("filename", ValueType::String) } } } },
                n);

        return Utils::fileExists(n[0].string().c_str()) ? trueSym : falseSym;
    }

    /**
     * @name io:listFiles
     * @brief List files in a folder, as a List of String
     * @param path A directory
     * =begin
     * (io:listFiles "/tmp/hello")
     * =end
     * @author https://github.com/SuperFola
     */
    Value listFiles(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "io:listFiles",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        std::vector<Value> r;
        for (const auto& entry : std::filesystem::directory_iterator(n[0].string().c_str()))
            r.emplace_back(entry.path().string().c_str());

        return Value(std::move(r));
    }

    /**
     * @name io:dir?
     * @brief Check if a path represents a directory
     * @param path A directory
     * =begin
     * (io:dir? "/tmp/hello")
     * =end
     * @author https://github.com/SuperFola
     */
    Value isDirectory(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "io:dir?",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        return (std::filesystem::is_directory(std::filesystem::path(n[0].string().c_str()))) ? trueSym : falseSym;
    }

    /**
     * @name io:makeDir
     * @brief Create a directory
     * @param path A directory
     * =begin
     * (io:makeDir "/tmp/myDir")
     * =end
     * @author https://github.com/SuperFola
     */
    Value makeDir(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (!types::check(n, ValueType::String))
            types::generateError(
                "io:makeDir",
                { { types::Contract { { types::Typedef("path", ValueType::String) } } } },
                n);

        std::filesystem::create_directories(std::filesystem::path(n[0].string().c_str()));
        return nil;
    }

    /**
     * @name io:removeFiles
     * @brief Delete files
     * @details Take multiple arguments, all String, each one representing a path to a file
     * @param filenames path to file
     * =begin
     * (io:removeFiles "/tmp/test.ark" "hello.json")
     * =end
     * @author https://github.com/SuperFola
     */
    Value removeFiles(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() < 1 || n[0].valueType() != ValueType::String)
            types::generateError(
                "io:removeFiles",
                { { types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("filenames", ValueType::String, /* variadic */ true) } } } },
                n);

        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
        {
            if (it->valueType() != ValueType::String)
                types::generateError(
                    "io:removeFiles",
                    { { types::Contract { { types::Typedef("filename", ValueType::String), types::Typedef("filenames", ValueType::String, /* variadic */ true) } } } },
                    n);

            std::filesystem::remove_all(std::filesystem::path(it->string().c_str()));
        }

        return nil;
    }
}
