/**
 * @file Config.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript configuration macros
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef INCLUDE_ARK_CONFIG_HPP
#define INCLUDE_ARK_CONFIG_HPP

#if defined(_WIN32) || defined(_WIN64)
#define ARK_OS_WINDOWS
#else  // Linux, FreeBSD, Mac OS X
#define ARK_OS_LINUX
#endif

#ifndef ARK_STATIC
#ifdef ARK_OS_WINDOWS
// Windows compilers need specific (and different) keywords for export and import
#define ARK_API_EXPORT __declspec(dllexport)
#define ARK_API_IMPORT __declspec(dllimport)

// For Visual C++ compilers, we also need to turn off this annoying C4251 warning
#ifdef _MSC_VER
#pragma warning(disable : 4251)
#endif
#else
#if __GNUC__ >= 4
// GCC 4 has special keywords for showing/hidding symbols,
// the same keyword is used for both importing and exporting
#define ARK_API_EXPORT __attribute__((__visibility__("default")))
#define ARK_API_IMPORT __attribute__((__visibility__("default")))
#else
// GCC < 4 has no mechanism to explicitely hide symbols, everything's exported
#define ARK_API_EXPORT
#define ARK_API_IMPORT
#endif
#endif
#else
#define ARK_API_EXPORT
#define ARK_API_IMPORT
#endif

#endif
