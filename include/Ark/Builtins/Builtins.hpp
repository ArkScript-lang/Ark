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

namespace Ark
{
    class VM;
}

#define ARK_BUILTIN(name) Value name(std::vector<Value>& n, VM* vm)

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
        ARK_BUILTIN(reverseList);
        ARK_BUILTIN(findInList);
        ARK_BUILTIN(sliceList);
        ARK_BUILTIN(sort_);
        ARK_BUILTIN(fill);
        ARK_BUILTIN(setListAt);
    }

    namespace IO
    {
        ARK_BUILTIN(print);
        ARK_BUILTIN(puts_);
        ARK_BUILTIN(input);
        ARK_BUILTIN(writeFile);
        ARK_BUILTIN(appendToFile);
        ARK_BUILTIN(readFile);
        ARK_BUILTIN(fileExists);
        ARK_BUILTIN(listFiles);
        ARK_BUILTIN(isDirectory);
        ARK_BUILTIN(makeDir);
        ARK_BUILTIN(removeFile);
    }

    namespace Time
    {
        ARK_BUILTIN(timeSinceEpoch);
    }

    namespace System
    {
        ARK_BUILTIN(system_);
        ARK_BUILTIN(sleep);
        ARK_BUILTIN(exit_);
    }

    namespace String
    {
        ARK_BUILTIN(format);
        ARK_BUILTIN(findSubStr);
        ARK_BUILTIN(removeAtStr);
        ARK_BUILTIN(ord);
        ARK_BUILTIN(chr);
        ARK_BUILTIN(setStringAt);
    }

    namespace Mathematics
    {
        ARK_BUILTIN(exponential);
        ARK_BUILTIN(logarithm);
        ARK_BUILTIN(ceil_);
        ARK_BUILTIN(floor_);
        ARK_BUILTIN(round_);
        ARK_BUILTIN(isnan_);
        ARK_BUILTIN(isinf_);

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        ARK_BUILTIN(cos_);
        ARK_BUILTIN(sin_);
        ARK_BUILTIN(tan_);
        ARK_BUILTIN(acos_);
        ARK_BUILTIN(asin_);
        ARK_BUILTIN(atan_);
        ARK_BUILTIN(cosh_);
        ARK_BUILTIN(sinh_);
        ARK_BUILTIN(tanh_);
        ARK_BUILTIN(acosh_);
        ARK_BUILTIN(asinh_);
        ARK_BUILTIN(atanh_);

        ARK_BUILTIN(random);
    }

    namespace Async
    {
        ARK_BUILTIN(async);
        ARK_BUILTIN(await);
    }

    namespace Dict
    {
        ARK_BUILTIN(dict);
        ARK_BUILTIN(get);
        ARK_BUILTIN(add);
        ARK_BUILTIN(contains);
        ARK_BUILTIN(remove);
        ARK_BUILTIN(keys);
        ARK_BUILTIN(size);
    }

    namespace Bytecode
    {
        ARK_BUILTIN(disassemble);
    }
}

#undef ARK_BUILTIN

#endif
