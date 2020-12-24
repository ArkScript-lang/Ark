#ifndef ark_profiling
#define ark_profiling

#include <Ark/Constants.hpp>

#if ARK_PROFILER == 1
    #include <coz.h>
#else
    // define the coz macro but don't add content to them
    // so that we can "use" them but they won't take effect
    #define COZ_PROGRESS_NAMED(name)
    #define COZ_PROGRESS
    #define COZ_BEGIN(name)
    #define COZ_END(name)
#endif

#endif  // ark_profiling
