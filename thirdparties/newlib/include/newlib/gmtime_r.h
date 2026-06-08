#ifndef NEWLIB_GMTIME_R_H
#define NEWLIB_GMTIME_R_H

#include <newlib/local.h>

#ifdef __cplusplus
extern "C" {
#endif
struct nl_tm* nl_gmtime_r(const long long tim_p, struct nl_tm* __restrict res);
#ifdef __cplusplus
}
#endif

#endif  // NEWLIB_GMTIME_R_H
