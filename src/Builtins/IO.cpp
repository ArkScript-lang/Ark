#include <Ark/Builtins/Builtins.hpp>

#include <iostream>
#include <filesystem>

#include <Ark/Utils.hpp>

#include <Ark/Builtins/BuiltinsErrors.inl>
#define Builtins_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::Builtins::IO
{
    Builtins_Function(print)
    {
        for (Value::Iterator it=n.begin(), it_end=n.end(); it != it_end; ++it)
            std::cout << (*it);
        std::cout << std::endl;

        return nil;
    }

    Builtins_Function(puts_)
    {
        for (Value::Iterator it=n.begin(), it_end=n.end(); it != it_end; ++it)
            std::cout << (*it);

        return nil;
    }

    Builtins_Function(input)
    {
        if (n.size() == 1)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError(IO_INPUT_TE);
            std::cout << n[0].string().c_str();
        }

        std::string line = "";
        std::getline(std::cin, line);

        return Value(line);
    }

    Builtins_Function(writeFile)
    {
        // filename, content
        if (n.size() == 2)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError(IO_WRITE_TE0);

            std::ofstream f(n[0].string().c_str());
            if (f.is_open())
            {
                f << n[1];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].string_ref().toString() + "\"");
        }
        // filename, mode (a or w), content
        else if (n.size() == 3)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError(IO_WRITE_TE0);
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError(IO_WRITE_TE1);

            auto mode = n[1].string().c_str();
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
                throw std::runtime_error("Couldn't write to file \"" + n[0].string_ref().toString() + "\"");
        }
        else
            throw std::runtime_error(IO_WRITE_ARITY);
        return nil;
    }

    Builtins_Function(readFile)
    {
        if (n.size() != 1)
            throw std::runtime_error(IO_READ_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(IO_READ_TE0);

        auto filename = n[0].string().c_str();
        if (!Ark::Utils::fileExists(filename))
            throw std::runtime_error("Couldn't read file \"" + std::string(filename) + "\": it doesn't exist");

        return Value(Ark::Utils::readFile(filename));
    }

    Builtins_Function(fileExists)
    {
        if (n.size() != 1)
            throw std::runtime_error(IO_EXISTS_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(IO_EXISTS_TE0);

        return Ark::Utils::fileExists(n[0].string().c_str()) ? trueSym : falseSym;
    }

    Builtins_Function(listFiles)
    {
        if (n.size() != 1)
            throw std::runtime_error(IO_LS_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(IO_LS_TE0);

        std::vector<Value> r;
        for (const auto& entry: std::filesystem::directory_iterator(n[0].string().c_str()))
            r.emplace_back(entry.path().string().c_str());

        return Value(std::move(r));
    }

    Builtins_Function(isDirectory)
    {
        if (n.size() != 1)
            throw std::runtime_error(IO_ISDIR_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(IO_ISDIR_TE0);
        
        return (std::filesystem::is_directory(std::filesystem::path(n[0].string().c_str()))) ? trueSym : falseSym;
    }

    Builtins_Function(makeDir)
    {
        if (n.size() != 1)
            throw std::runtime_error(IO_MKD_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError(IO_MKD_TE0);
        
        std::filesystem::create_directories(std::filesystem::path(n[0].string().c_str()));
        return nil;
    }

    Builtins_Function(removeFiles)
    {
        if (n.size() == 0)
            throw std::runtime_error(IO_RM_ARITY);
        
        for (Value::Iterator it=n.begin(), it_end=n.end(); it != it_end; ++it)
        {
            if (it->valueType() != ValueType::String)
                throw Ark::TypeError(IO_RM_TE0);
            std::filesystem::remove_all(std::filesystem::path(it->string().c_str()));
        }

        return nil;
    }
}