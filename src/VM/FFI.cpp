#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <thread>
#include <cstdlib>
#include <Ark/Log.hpp>

#undef abs
#include <cmath>
#include <chrono>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym = Value(NFT::False);
    extern const Value trueSym  = Value(NFT::True);
    extern const Value nil      = Value(NFT::Nil);
    // not assignable value
    extern const Value undefined = Value(NFT::Undefined);

    extern const std::vector<std::pair<std::string, Value>> builtins = {
        { "false",  falseSym },
        { "true",   trueSym },
        { "nil",    nil },
        { "append", Value(append) },
        { "concat", Value(concat) },
        { "list",   Value(list) },
        { "print",  Value(print) },
        { "input",  Value(input) },
        { "writeFile", Value(writeFile) },
        { "readFile", Value(readFile) },
        { "fileExists?", Value(fileExists) },
        { "time", Value(timeSinceEpoch) },
        { "sleep", Value(sleep) },
        { "system", Value(system_) },
        { "format", Value(format) }
    };

    extern const std::vector<std::string> operators = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstOf", "tailOf", "headOf",
        "nil?", "assert",
        "toNumber", "toString",
        "@", "and", "or", "mod",
        "type", "hasField",
    };

    // ------------------------------

    FFI_Function(append)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of append must be a list");
        
        Value r(std::move(n[0]));
        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(concat)
    {
        if (n[0].valueType() != ValueType::List)
            throw Ark::TypeError("First argument of concat should be a list");
        
        Value r(std::move(n[0]));
        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() != ValueType::List)
                throw Ark::TypeError("Arguments of concat must be lists");

            for (Value::Iterator it2=it->const_list().begin(); it2 != it->const_list().end(); ++it2)
                r.push_back(*it2);
        }
        return r;
    }

    FFI_Function(list)
    {
        Value r(ValueType::List);
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            r.push_back(*it);
        return r;
    }

    FFI_Function(print)
    {
        for (Value::Iterator it=n.begin(); it != n.end(); ++it)
            std::cout << (*it);
        std::cout << std::endl;

        return nil;
    }

    FFI_Function(input)
    {
        if (n.size() == 1)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError("Argument of input must be of type String");
            std::cout << n[0].string();
        }

        std::string line = "";
        std::getline(std::cin, line);

        return Value(line);
    }

    FFI_Function(writeFile)
    {
        // filename, content
        if (n.size() == 2)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError("First argument of writeFile (filename) should be a String");
            
            std::ofstream f(n[0].string());
            if (f.is_open())
            {
                f << n[1];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].string() + "\"");
        }
        // filename, mode (a or w), content
        else if (n.size() == 3)
        {
            if (n[0].valueType() != ValueType::String)
                throw Ark::TypeError("First argument of writeFile (filename) should be a String");
            if (n[1].valueType() != ValueType::String)
                throw Ark::TypeError("Second argument of writeFile (mode) should be a String");
            
            auto mode = n[1].string();
            if (mode != "w" && mode != "a")
                throw std::runtime_error("Second argument of writeFile (mode) is incorrect, available modes are \"a\" and \"w\"");
            
            auto ios_mode = std::ios::out | std::ios::trunc;
            if (mode == "a")
                ios_mode = std::ios::out | std::ios::app;
            
            std::ofstream f(n[0].string(), ios_mode);
            if (f.is_open())
            {
                f << n[2];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].string() + "\"");
        }
        else
            throw std::runtime_error("Got too many argument for writeFile: need a filename, an optional mode and a content");
        return nil;
    }

    FFI_Function(readFile)
    {
        if (n.size() != 1)
            throw std::runtime_error("readFile need 1 argument: filename");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument of readFile should be a String");
        
        auto filename = n[0].string();
        if (!Ark::Utils::fileExists(filename))
            throw std::runtime_error("Couldn't read file \"" + filename + "\": it doesn't exist");

        return Value(Ark::Utils::readFile(filename));
    }

    FFI_Function(fileExists)
    {
        if (n.size() != 1)
            throw std::runtime_error("fileExists? can take only 1 argument, a filename (String)");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument of fileExists? must be of type String");
        
        return Value(Ark::Utils::fileExists(n[0].string()) ? NFT::True : NFT::False);
    }

    FFI_Function(timeSinceEpoch)
    {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        return Value(static_cast<double>(milliseconds.count()) / 1000);
    }

    FFI_Function(sleep)
    {
        if (n.size() != 1)
            throw std::runtime_error("sleep can take only one argument, a duration (milliseconds)");
        if (n[0].valueType() != ValueType::Number)
            throw std::runtime_error("Argument of sleep must be of type Number");
        
        auto duration = std::chrono::duration<double, std::ratio<1, 1000>>(n[0].number());
        std::this_thread::sleep_for(duration);
        
        return nil;
    }

    FFI_Function(system_)
    {
        if (n.size() != 1)
            throw std::runtime_error("system can take only one argument, a command");
        if (n[0].valueType() != ValueType::String)
            throw std::runtime_error("Argument of system must be of type String");
        
        #if ARK_ENABLE_SYSTEM != 0
            std::system(n[0].string().c_str());
        #endif  // ARK_ENABLE_SYSTEM
        
        return nil;
    }

    FFI_Function(format)
    {
        if (n.size() == 0)
            throw std::runtime_error("format take at least one argument");
        if (n[0].valueType() != ValueType::String)
            throw std::runtime_error("Argument 1 of format must be of type String");

        rj::format f(n[0].string());

        for (Value::Iterator it=n.begin()+1; it != n.end(); ++it)
        {
            if (it->valueType() == ValueType::String)
                f.args(it->string());
            else if (it->valueType() == ValueType::Number)
                f.args(it->number());
            else
                throw std::runtime_error("Argument of format must be of type String or Number");
        }

        return Value(std::string(f));
    }
}
