/**
 * @file Builtins.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Host the declaration of all the ArkScript builtins
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
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

    extern const std::vector<std::pair<std::string, Value>> builtins;

    // ------------------------------
    // builtins functions: we must use the instruction `BUILTIN index`
    // ------------------------------
    namespace List
    {
        Value reverseList(std::vector<Value>& n, VM* vm);  // list:reverse, single arg
        Value findInList(std::vector<Value>& n, VM* vm);   // list:find, 2 arguments
        Value sliceList(std::vector<Value>& n, VM* vm);    // list:slice, 4 arguments
        Value sort_(std::vector<Value>& n, VM* vm);        // list:sort, 1 argument
        Value fill(std::vector<Value>& n, VM* vm);         // list:fill, 2 arguments
        Value setListAt(std::vector<Value>& n, VM* vm);    // list:setAt, 3 arguments
    }

    namespace IO
    {
        Value print(std::vector<Value>& n, VM* vm);        // print, multiple arguments
        Value puts_(std::vector<Value>& n, VM* vm);        // puts, multiple arguments
        Value input(std::vector<Value>& n, VM* vm);        // input, 0 or 1 argument
        Value writeFile(std::vector<Value>& n, VM* vm);    // io:writeFile, 2 or 3 arguments
        Value readFile(std::vector<Value>& n, VM* vm);     // io:readFile, 1 argument
        Value fileExists(std::vector<Value>& n, VM* vm);   // io:fileExists?, 1 argument
        Value listFiles(std::vector<Value>& n, VM* vm);    // io:listFiles, 1 argument
        Value isDirectory(std::vector<Value>& n, VM* vm);  // io:isDir?, 1 argument
        Value makeDir(std::vector<Value>& n, VM* vm);      // io:makeDir, 1 argument
        Value removeFiles(std::vector<Value>& n, VM* vm);  // io:removeFiles, multiple arguments
    }

    namespace Time
    {
        Value timeSinceEpoch(std::vector<Value>& n, VM* vm);  // time, 0 argument
    }

    namespace System
    {
        Value system_(std::vector<Value>& n, VM* vm);  // sys:exec, 1 argument
        Value sleep(std::vector<Value>& n, VM* vm);    // sleep, 1 argument
        Value exit_(std::vector<Value>& n, VM* vm);    // sys:exit, 1 argument
    }

    namespace String
    {
        Value format(std::vector<Value>& n, VM* vm);       // str:format, multiple arguments
        Value findSubStr(std::vector<Value>& n, VM* vm);   // str:find, 2 arguments
        Value removeAtStr(std::vector<Value>& n, VM* vm);  // str:removeAt, 2 arguments
        Value ord(std::vector<Value>& n, VM* vm);          // str:ord, 1 arguments
        Value chr(std::vector<Value>& n, VM* vm);          // str:chr, 1 arguments
    }

    namespace Mathematics
    {
        Value exponential(std::vector<Value>& n, VM* vm);  // math:exp, 1 argument
        Value logarithm(std::vector<Value>& n, VM* vm);    // math:ln, 1 argument
        Value ceil_(std::vector<Value>& n, VM* vm);        // math:ceil, 1 argument
        Value floor_(std::vector<Value>& n, VM* vm);       // math:floor, 1 argument
        Value round_(std::vector<Value>& n, VM* vm);       // math:round, 1 argument
        Value isnan_(std::vector<Value>& n, VM* vm);       // math:NaN?, 1 argument
        Value isinf_(std::vector<Value>& n, VM* vm);       // math:Inf?, 1 argument

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        Value cos_(std::vector<Value>& n, VM* vm);    // math:cos, 1 argument
        Value sin_(std::vector<Value>& n, VM* vm);    // math:sin, 1 argument
        Value tan_(std::vector<Value>& n, VM* vm);    // math:tan, 1 argument
        Value acos_(std::vector<Value>& n, VM* vm);   // math:arccos, 1 argument
        Value asin_(std::vector<Value>& n, VM* vm);   // math:arcsin, 1 argument
        Value atan_(std::vector<Value>& n, VM* vm);   // math:arctan, 1 argument
        Value cosh_(std::vector<Value>& n, VM* vm);   // math:cosh, 1 argument
        Value sinh_(std::vector<Value>& n, VM* vm);   // math:sinh, 1 argument
        Value tanh_(std::vector<Value>& n, VM* vm);   // math:tanh, 1 argument
        Value acosh_(std::vector<Value>& n, VM* vm);  // math:acosh, 1 argument
        Value asinh_(std::vector<Value>& n, VM* vm);  // math:asinh, 1 argument
        Value atanh_(std::vector<Value>& n, VM* vm);  // math:atanh, 1 argument
    }

    namespace Async
    {
        Value async(std::vector<Value>& n, VM* vm);  // async, 1+ arguments
        Value await(std::vector<Value>& n, VM* vm);  // await, 1 argument
    }
}

#endif
