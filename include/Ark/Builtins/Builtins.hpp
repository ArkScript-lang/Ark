/**
 * @file Builtins.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Host the declaration of all the ArkScript builtins
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef ARK_BUILTINS_BUILTINS_HPP
#define ARK_BUILTINS_BUILTINS_HPP

#include <vector>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal::Builtins
{
    extern const Value falseSym;
    extern const Value trueSym;
    extern const Value nil;
    extern const Value platform;

    ARK_API extern const std::vector<std::pair<std::string, Value>> builtins;

    // ------------------------------
    // builtins functions: we must use the instruction `BUILTIN index`
    // ------------------------------
    namespace List
    {
        Value reverseList(std::vector<Value>& n, VM* vm);  // builtin__list:reverse, single arg
        Value findInList(std::vector<Value>& n, VM* vm);   // builtin__list:find, 2 arguments
        Value sliceList(std::vector<Value>& n, VM* vm);    // builtin__list:slice, 4 arguments
        Value sort_(std::vector<Value>& n, VM* vm);        // builtin__list:sort, 1 argument
        Value fill(std::vector<Value>& n, VM* vm);         // builtin__list:fill, 2 arguments
        Value setListAt(std::vector<Value>& n, VM* vm);    // builtin__list:setAt, 3 arguments
    }

    namespace IO
    {
        Value print(std::vector<Value>& n, VM* vm);         // print, multiple arguments
        Value puts_(std::vector<Value>& n, VM* vm);         // puts, multiple arguments
        Value input(std::vector<Value>& n, VM* vm);         // input, 0 or 1 argument
        Value writeFile(std::vector<Value>& n, VM* vm);     // builtin__io:writeFile, 2 arguments
        Value appendToFile(std::vector<Value>& n, VM* vm);  // builtin__io:appendToFile, 2 arguments
        Value readFile(std::vector<Value>& n, VM* vm);      // builtin__io:readFile, 1 argument
        Value fileExists(std::vector<Value>& n, VM* vm);    // builtin__io:fileExists?, 1 argument
        Value listFiles(std::vector<Value>& n, VM* vm);     // builtin__io:listFiles, 1 argument
        Value isDirectory(std::vector<Value>& n, VM* vm);   // builtin__io:isDir?, 1 argument
        Value makeDir(std::vector<Value>& n, VM* vm);       // builtin__io:makeDir, 1 argument
        Value removeFile(std::vector<Value>& n, VM* vm);    // builtin__io:removeFile, multiple arguments
    }

    namespace Time
    {
        Value timeSinceEpoch(std::vector<Value>& n, VM* vm);  // time, 0 argument
    }

    namespace System
    {
        Value system_(std::vector<Value>& n, VM* vm);  // builtin__sys:exec, 1 argument
        Value sleep(std::vector<Value>& n, VM* vm);    // builtin__sys:sleep, 1 argument
        Value exit_(std::vector<Value>& n, VM* vm);    // builtin__sys:exit, 1 argument
    }

    namespace String
    {
        Value format(std::vector<Value>& n, VM* vm);       // format, multiple arguments
        Value findSubStr(std::vector<Value>& n, VM* vm);   // builtin__string:find, 2 arguments
        Value removeAtStr(std::vector<Value>& n, VM* vm);  // builtin__string:removeAt, 2 arguments
        Value ord(std::vector<Value>& n, VM* vm);          // builtin__string:ord, 1 arguments
        Value chr(std::vector<Value>& n, VM* vm);          // builtin__string:chr, 1 arguments
        Value setStringAt(std::vector<Value>& n, VM* vm);  // builtin__string::setAt, 3 arguments
    }

    namespace Mathematics
    {
        Value exponential(std::vector<Value>& n, VM* vm);  // builtin__math:exp, 1 argument
        Value logarithm(std::vector<Value>& n, VM* vm);    // builtin__math:ln, 1 argument
        Value ceil_(std::vector<Value>& n, VM* vm);        // builtin__math:ceil, 1 argument
        Value floor_(std::vector<Value>& n, VM* vm);       // builtin__math:floor, 1 argument
        Value round_(std::vector<Value>& n, VM* vm);       // builtin__math:round, 1 argument
        Value isnan_(std::vector<Value>& n, VM* vm);       // builtin__math:NaN?, 1 argument
        Value isinf_(std::vector<Value>& n, VM* vm);       // builtin__math:Inf?, 1 argument

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        Value cos_(std::vector<Value>& n, VM* vm);    // builtin__math:cos, 1 argument
        Value sin_(std::vector<Value>& n, VM* vm);    // builtin__math:sin, 1 argument
        Value tan_(std::vector<Value>& n, VM* vm);    // builtin__math:tan, 1 argument
        Value acos_(std::vector<Value>& n, VM* vm);   // builtin__math:arccos, 1 argument
        Value asin_(std::vector<Value>& n, VM* vm);   // builtin__math:arcsin, 1 argument
        Value atan_(std::vector<Value>& n, VM* vm);   // builtin__math:arctan, 1 argument
        Value cosh_(std::vector<Value>& n, VM* vm);   // builtin__math:cosh, 1 argument
        Value sinh_(std::vector<Value>& n, VM* vm);   // builtin__math:sinh, 1 argument
        Value tanh_(std::vector<Value>& n, VM* vm);   // builtin__math:tanh, 1 argument
        Value acosh_(std::vector<Value>& n, VM* vm);  // builtin__math:acosh, 1 argument
        Value asinh_(std::vector<Value>& n, VM* vm);  // builtin__math:asinh, 1 argument
        Value atanh_(std::vector<Value>& n, VM* vm);  // builtin__math:atanh, 1 argument

        Value random(std::vector<Value>& n, VM* vm);  // random, 0-2 args
    }

    namespace Async
    {
        Value async(std::vector<Value>& n, VM* vm);  // async, 1+ arguments
        Value await(std::vector<Value>& n, VM* vm);  // await, 1 argument
    }
}

#endif
