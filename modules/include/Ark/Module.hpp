#ifndef ark_module
#define ark_module

#include <Ark/VM/VM.hpp>
using namespace Ark;

using Mapping_t = std::unordered_map<std::string, Value::ProcType>;

#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS

    // Windows compilers need specific (and different) keywords for export and import
    #define ARK_API_EXPORT extern "C" __declspec(dllexport)
    #define ARK_API_IMPORT extern "C" __declspec(dllimport)

    // For Visual C++ compilers, we also need to turn off this annoying C4251 warning
    #ifdef _MSC_VER
        #pragma warning(disable: 4251)
    #endif
#else // Linux, FreeBSD, Mac OS X
    #define OS_LINUX
    
    #if __GNUC__ >= 4
        // GCC 4 has special keywords for showing/hidding symbols,
        // the same keyword is used for both importing and exporting
        #define ARK_API_EXPORT extern "C" __attribute__ ((__visibility__ ("default")))
        #define ARK_API_IMPORT extern "C" __attribute__ ((__visibility__ ("default")))
    #else
        // GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
        #define ARK_API_EXPORT extern "C"
        #define ARK_API_IMPORT extern "C"
    #endif
#endif

#endif