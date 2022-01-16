#include <Ark/Builtins/Builtins.hpp>

#include <Ark/String.hpp>
#include <Ark/Utils.hpp>
#include <utf8_decoder.h>

#include <Ark/Builtins/BuiltinsErrors.inl>
#include <Ark/VM/VM.hpp>

namespace Ark::internal::Builtins::String
{
    /**
     * @name str:format
     * @brief Format a String given replacements
     * @details The format is %% for anything, and %x for hex numbers
     * @param format the String to format
     * @param values as any argument as you need, of any valid ArkScript type
     * =begin
     * (str:format "Hello %%, my name is %%" "world" "ArkScript")
     * # Hello world, my name is ArkScript
     * 
     * (str:format "Test %% with %%" "1")
     * # Test 1 with %%
     * =end
     * @author https://github.com/SuperFola
     */
    Value format(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        // TODO find a way to use BetterTypeError with variadic functions
        if (n.size() == 0)
            throw std::runtime_error(STR_FORMAT_ARITY);
        if (n[0].valueType() != ValueType::String)
            throw TypeError(STR_FORMAT_TE0);

        ::String f(n[0].string().c_str());

        for (Value::Iterator it = n.begin() + 1, it_end = n.end(); it != it_end; ++it)
        {
            if (it->valueType() == ValueType::String)
            {
                ::String& obj = it->stringRef();
                f.format(f.size() + obj.size(), obj.c_str());
            }
            else if (it->valueType() == ValueType::Number)
            {
                double obj = it->number();
                f.format(f.size() + Utils::digPlaces(obj) + Utils::decPlaces(obj) + 1, obj);
            }
            else if (it->valueType() == ValueType::Nil)
                f.format(f.size() + 5, std::string_view("nil"));
            else if (it->valueType() == ValueType::True)
                f.format(f.size() + 5, std::string_view("true"));
            else if (it->valueType() == ValueType::False)
                f.format(f.size() + 5, std::string_view("false"));
            else
            {
                std::stringstream ss;
                ss << (*it);
                f.format(f.size() + ss.str().size(), std::string_view(ss.str().c_str()));
            }
        }
        n[0].stringRef() = f;
        return n[0];
    }

    /**
     * @name str:find
     * @brief Search a substring in a given String
     * @details The original String is not modified. Return -1 when not found
     * @param string the String to search in
     * @param substr the substring to search for
     * =begin
     * (str:find "hello world" "hello")  # 0
     * (str:find "hello world" "aworld")  # -1
     * =end
     * @author https://github.com/SuperFola
     */
    Value findSubStr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2 || n[0].valueType() != ValueType::String ||
            n[1].valueType() != ValueType::String)
            throw BetterTypeError("str:find", 2, n)
                .withArg("string", ValueType::String)
                .withArg("substr", ValueType::String);

        return Value(n[0].stringRef().find(n[1].stringRef()));
    }

    /**
     * @name str:removeAt
     * @brief Remove a character from a String given an index
     * @details The original String is not modified
     * @param string the String to modify
     * @param index the index of the character to remove (can be negative to search from the end)
     * =begin
     * (str:removeAt "hello world" 0)  # "ello world"
     * (str:removeAt "hello world" -1)  # "hello worl"
     * =end
     * @author https://github.com/SuperFola
     */
    Value removeAtStr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 2 || n[0].valueType() != ValueType::String ||
            n[1].valueType() != ValueType::Number)
            throw BetterTypeError("str:removeAt", 2, n)
                .withArg("string", ValueType::String)
                .withArg("index", ValueType::Number);

        long id = static_cast<long>(n[1].number());
        if (id < 0 || static_cast<std::size_t>(id) >= n[0].stringRef().size())
            throw std::runtime_error(STR_RM_OOR);

        n[0].stringRef().erase(id, id + 1);
        return n[0];
    }

    /**
     * @name str:ord
     * @brief Get the ordinal of a given character
     * @param char a String with a single UTF8 character
     * =begin
     * (str:ord "h")  # 104
     * (str:ord "Ô")  # 212
     * =end
     * @author https://github.com/SuperFola
     */
    Value ord(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("str:ord", 1, n)
                .withArg("string", ValueType::String);

        int ord = utf8codepoint(n[0].stringRef().c_str());

        return Value(ord);
    }

    /**
     * @name str:chr
     * @brief Create a character from an UTF8 codepoint
     * @param codepoint an UTF8 codepoint (Number)
     * =begin
     * (str:chr 104)  # "h"
     * (str:chr 212)  # "Ô"
     * =end
     * @author https://github.com/SuperFola
     */
    Value chr(std::vector<Value>& n, VM* vm [[maybe_unused]])
    {
        if (n.size() != 1 || n[0].valueType() != ValueType::String)
            throw BetterTypeError("str:chr", 1, n)
                .withArg("codepoint", ValueType::Number);

        std::array<char, 5> sutf8;

        utf8chr(static_cast<int>(n[0].number()), sutf8.data());
        return Value(std::string(sutf8.data()));
    }
}
