#ifndef INCLUDE_ARK_PROFILING_HPP
#define INCLUDE_ARK_PROFILING_HPP

#ifdef ARK_PROFILER
#    include <coz.h>
#else
// define the coz macro but don't add content to them
// so that we can "use" them but they won't take effect
#    define COZ_PROGRESS_NAMED(name)
#    define COZ_PROGRESS
#    define COZ_BEGIN(name)
#    define COZ_END(name)
#endif

#endif
