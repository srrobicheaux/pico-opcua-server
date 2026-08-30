#include "pico_lwip_posix_fixes.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <lwip/sockets.h>

#include <sys/stat.h>

int _stat(const char *file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFREG; // Mock everything as a standard data file stream
    return 0;
}
