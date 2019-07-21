#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <Ark/Log.hpp>

#undef abs
#include <cmath>

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
        { "append", Value(&append) },
        { "concat", Value(&concat) },
        { "list",   Value(&list) },
        { "print",  Value(&print) },
        { "input",  Value(&input) },
        { "writeFile", Value(&writefile) },
        { "readFile", Value(&readfile) },
        { "fileExists?", Value(&fileexists) }
    };

    extern const std::vector<std::string> operators = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstof", "tailof", "headof",
        "nil?", "assert",
        "toNumber", "toString",
        "@", "and", "or", "mod",
        "type", "hasfield",
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
            std::cout << (*it) << " ";
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

    FFI_Function(writefile)
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
                f << n[1];
                f.close();
            }
            else
                throw std::runtime_error("Couldn't write to file \"" + n[0].string() + "\"");
        }
        else
            throw std::runtime_error("Got too many argument for writeFile: need a filename, an optional mode and a content");
        return nil;
    }

    FFI_Function(readfile)
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

    FFI_Function(fileexists)
    {
        if (n.size() != 1)
            throw std::runtime_error("fileExists? can take only 1 argument, a filename (String)");
        if (n[0].valueType() != ValueType::String)
            throw Ark::TypeError("Argument of fileExists? must be of type String");
        
        return Value(Ark::Utils::fileExists(n[0].string()) ? NFT::True : NFT::False);
    }
}
