#pragma once
#include <coreinit/debug.h>
#include <cstring>

#define __FILENAME__ ({                            \
    const char *__filename = __FILE__;               \
    const char *__pos      = strrchr(__filename, '/'); \
    if (!__pos) __pos = strrchr(__filename, '\\');       \
    __pos ? __pos + 1 : __filename;                      \
})

#define DEBUG_FUNCTION_LINE_ERR(FMT, ARGS...)                                                                                         \
    do {                                                                                                                              \
        OSReport("[(%s)%18s][%23s]%30s@L%04d: ## ERROR ## " FMT "\n", "L", "libmocha", __FILENAME__, __FUNCTION__, __LINE__, ##ARGS); \
    } while (0)
