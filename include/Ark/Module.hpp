#ifndef INCLUDE_ARK_MODULE_HPP
#define INCLUDE_ARK_MODULE_HPP

#include <Ark/VM/VM.hpp>
#include <Ark/Platform.hpp>
#include <Ark/TypeChecker.hpp>

namespace Ark
{
    struct mapping
    {
        const char* name;
        Value (*value)(std::vector<Value>&, Ark::VM*);
    };
}

#undef ARK_API

#ifdef ARK_OS_WINDOWS
// Windows compilers need specific (and different) keywords for export and import
#    define ARK_API extern "C" __declspec(dllexport)
#else  // Linux, FreeBSD, Mac OS X
#    if __GNUC__ >= 4
// GCC 4 has special keywords for showing/hiding symbols,
// the same keyword is used for both importing and exporting
#        define ARK_API extern "C" __attribute__((__visibility__("default")))
#    else
// GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
#        define ARK_API extern "C"
#    endif
#endif

#endif
