/**
 * @file Platform.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript configuration macros
 * @version 0.2
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef INCLUDE_ARK_PLATFORM_HPP
#define INCLUDE_ARK_PLATFORM_HPP

#if defined(_WIN32) || defined(_WIN64)
#    define ARK_OS_WINDOWS
#else  // Linux, FreeBSD, Mac OS X
#    define ARK_OS_LINUX
#endif

#ifndef ARK_STATIC
#    ifdef ARK_OS_WINDOWS
// Windows compilers need specific (and different) keywords for export and import
#        ifdef ARK_EXPORT
#            define ARK_API __declspec(dllexport)
#        else
#            define ARK_API __declspec(dllimport)
#        endif

// For Visual C++ compilers, we also need to turn off this annoying C4251 warning
#        ifdef _MSC_VER
#            pragma warning(disable : 4251)
#        endif
#    else
#        if __GNUC__ >= 4
// GCC 4 has special keywords for showing/hidding symbols,
// the same keyword is used for both importing and exporting
#            define ARK_API __attribute__((__visibility__("default")))
#        else
// GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
#            define ARK_API
#        endif
#    endif
#else
#    define ARK_API
#endif

#ifdef ARK_OS_WINDOWS
#    define ARK_API_INLINE ARK_API
#else
#    define ARK_API_INLINE ARK_API inline
#endif

#endif
