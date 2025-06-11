// Inspired by https://aras-p.info/blog/2018/01/12/Minimizing-windows.h/

#ifndef MINIWINDOWS_H
#define MINIWINDOWS_H

#if defined(_WIN32) || defined(_WIN64)

#    if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#        define _AMD64_
#    elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(_M_IX86)
#        define _X86_
#    elif defined(__arm__) || defined(_M_ARM) || defined(_M_ARMT)
#        define _ARM_
#    endif

// Do not define NOMINMAX, if it's already defined (without, the
// #define causes issues on MingW)
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif

// https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
#    include <errhandlingapi.h>

// FreeLibrary
// LoadLibrary
// GetProcAddress
// -> https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/
#    include <process.h>
#    include <libloaderapi.h>
#endif

#endif  // MINIWINDOWS_H
