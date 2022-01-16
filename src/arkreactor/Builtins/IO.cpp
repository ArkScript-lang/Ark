#include <Ark/Builtins/Builtins.hpp>

#include <cstdio>
#include <iostream>
#include <filesystem>

#include <Ark/Files.hpp>
#include <Ark/VM/VM.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Builtins/BuiltinsErrors.inl>

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
    Value print(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
            std::cout << (*it);
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
    Value puts_(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
            std::cout << (*it);

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
        if (n.size() == 1)
        {
            if (n[0].valueType() != ValueType::String)
                throw BetterTypeError("input", 1, n).withArg("prompt", ValueType::String);
            std::printf("%s", n[0].string().c_str());
        }

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
    Value writeFile(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        // filename, content
        if (n.size() == 2)
        {
            if (n[0].valueType() != ValueType::String)
                throw BetterTypeError("io:writeFile", 2, n)
                    .withArg("filename", ValueType::String)
                    .withArg("content", {});

            std::ofstream f(n[0].string().c_str());
            if (f.is_open())
            {
                f << n[1];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].stringRef().toString() + "\"");
        }
        // filename, mode (a or w), content
        else if (n.size() == 3)
        {
            if (n[0].valueType() != ValueType::String || n[1].valueType() != ValueType::String)
                throw BetterTypeError("io:writeFile", 3, n)
                    .withArg("filename", ValueType::String)
                    .withArg("mode", ValueType::String)
                    .withArg("content", {});

            auto mode = n[1].string();
            if (mode != "w" && mode != "a")
                throw std::runtime_error(IO_WRITE_VE_1);

            auto ios_mode = std::ios::out | std::ios::trunc;
            if (mode == "a")
                ios_mode = std::ios::out | std::ios::app;

            std::ofstream f(n[0].string().c_str(), ios_mode);
            if (f.is_open())
            {
                f << n[2];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].stringRef().toString() + "\"");
        }
        else
            throw BetterTypeError("io:writeFile", 3, n)
                .withArg("filename", ValueType::String)
                .withArg("mode", ValueType::String)
                .withArg("content", {});
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
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("io:readFile", 1, n)
                .withArg("filename", ValueType::String);

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
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("io:fileExists?", 1, n)
                .withArg("filename", ValueType::String);

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
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("io:listFiles", 1, n)
                .withArg("path", ValueType::String);

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
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("io:dir?", 1, n)
                .withArg("path", ValueType::String);

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
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("io:makeDir", 1, n)
                .withArg("name", ValueType::String);

        std::filesystem::create_directories(std::filesystem::path(n[0].string().c_str()));
        return nil;
    }

    /**
     * @name io:removeFiles
     * @brief Delete files
     * @details Take multiple arguments, all String, each one representing a path to a file
     * @param values path to file
     * =begin
     * (io:removeFiles "/tmp/test.ark" "hello.json")
     * =end
     * @author https://github.com/SuperFola
     */
    Value removeFiles(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() == 0)
            throw BetterTypeError("io:removeFiles", 1, n)
                .withArg("filename", ValueType::String);

        for (Value::Iterator it = n.begin(), it_end = n.end(); it != it_end; ++it)
        {
            if (it->valueType() != ValueType::String)
                // TODO find a way to make it work with variadic argument functions
                throw BetterTypeError("io:listFiles", n.size(), n)
                    .withArg("filename", ValueType::String);
            std::filesystem::remove_all(std::filesystem::path(it->string().c_str()));
        }

        return nil;
    }
}
