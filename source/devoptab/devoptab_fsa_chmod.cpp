#include "devoptab_fs.h"
#include "logger.h"
#include <mutex>
#include <sys/stat.h>

int __fsa_chmod(struct _reent *r,
                const char *path,
                mode_t mode) {
    FSError status;

    if (!path) {
        r->_errno = EINVAL;
        return -1;
    }

    char *fixedPath = __fsa_fixpath(r, path);
    if (!fixedPath) {
        return -1;
    }

    FSMode translatedMode = __fsa_translate_permission_mode(mode);

    auto *deviceData = (FSADeviceData *) r->deviceData;

    std::lock_guard<FastLockWrapper> lock(deviceData->mutex);

    status = FSAChangeMode(deviceData->clientHandle, fixedPath, translatedMode);
    if (status < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAChangeMode(0x%08X, %s, 0x%X) failed: %s", deviceData->clientHandle, fixedPath, translatedMode, FSAGetStatusStr(status));
        free(fixedPath);
        r->_errno = __fsa_translate_error(status);
        return -1;
    }
    free(fixedPath);

    return 0;
}
