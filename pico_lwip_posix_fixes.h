#ifndef PICO_LWIP_POSIX_FIXES_H
#define PICO_LWIP_POSIX_FIXES_H

#ifndef static_assert
#define static_assert(expr, msg) _Static_assert(expr, msg)
#endif

#include <errno.h>
#include <time.h>
#include "lwip/sockets.h"

#ifndef errno
#define errno errno
#endif

/* POSIX socket error code fallbacks for lwIP bare-metal / Newlib nano */
#ifndef EINTR
#define EINTR 4
#endif

#ifndef EAGAIN
#define EAGAIN 11
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK 11
#endif

#ifndef EINPROGRESS
#define EINPROGRESS 115
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 1
#endif

int clock_gettime(clockid_t clk_id, struct timespec *tp);

// Ensure standard socket error mappings exist if not provided by Newlib
#ifndef ENXIO
#define ENXIO 6
#endif
//missing lwIP Error codes
#ifndef ENOMEM
#define ENOMEM -1
#endif
#ifndef ENOBUFS
#define ENOBUFS -2
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH -4
#endif
#ifndef EINVAL
#define EINVAL -6
#endif
#ifndef EADDRINUSE
#define EADDRINUSE -8
#endif
#ifndef EALREADY
#define EALREADY -9
#endif
#ifndef EISCONN
#define EISCONN -10
#endif
#ifndef ENOTCONN
#define ENOTCONN -11
#endif
#ifndef ECONNABORTED
#define ECONNABORTED -13
#endif
#ifndef ECONNRESET
#define ECONNRESET -14
#endif
#ifndef EIO
#define EIO -16
#endif



#ifdef __cplusplus
}
#endif

#endif /* PICO_LWIP_POSIX_FIXES_H */