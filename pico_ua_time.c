#include <sys/time.h>
#include "pico/time.h"
#include <time.h>
#include "open62541/types.h"

// 1. Provide POSIX clock_gettime for eventloop_lwip.c
int clock_gettime(clockid_t clock_id, struct timespec *tp) {
    (void)clock_id; // Unused on bare-metal
    uint64_t now_us = time_us_64();
    tp->tv_sec = now_us / 1000000;
    tp->tv_nsec = (now_us % 1000000) * 1000;
    return 0;
}

// OPC UA time is hundreds of nanoseconds since Jan 1, 1601
#define UA_UNIX_EPOCH_OFFSET 11644473600LL

UA_DateTime UA_DateTime_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + UA_UNIX_EPOCH_OFFSET) * 10000000LL + (tv.tv_usec * 10LL);
}

UA_DateTime UA_DateTime_nowMonotonic(void) {
    // Pico's monotonic uptime in microseconds converted to 100ns ticks
    uint64_t us = to_us_since_boot(get_absolute_time());
    return (us * 10LL);
}

UA_DateTime UA_DateTime_localTimeUtcOffset(void) {
    return 0; // Return UTC for now
}