// This is due to a warning in GCC 14.1.0: https://github.com/okdshin/PicoSHA2/issues/25
#ifndef __clang__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
#include <picosha2.h>
#ifndef __clang__
#    pragma GCC diagnostic pop
#endif
