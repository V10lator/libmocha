#include "devoptab_fs.h"
#include "logger.h"
#include <coreinit/cache.h>
#include <coreinit/filesystem_fsa.h>
#include <mocha/mocha.h>
#include <mutex>

static const devoptab_t fsa_default_devoptab = {
        .structSize   = sizeof(__fsa_file_t),
        .open_r       = __fsa_open,
        .close_r      = __fsa_close,
        .write_r      = __fsa_write,
        .read_r       = __fsa_read,
        .seek_r       = __fsa_seek,
        .fstat_r      = __fsa_fstat,
        .stat_r       = __fsa_stat,
        .link_r       = __fsa_link,
        .unlink_r     = __fsa_unlink,
        .chdir_r      = __fsa_chdir,
        .rename_r     = __fsa_rename,
        .mkdir_r      = __fsa_mkdir,
        .dirStateSize = sizeof(__fsa_dir_t),
        .diropen_r    = __fsa_diropen,
        .dirreset_r   = __fsa_dirreset,
        .dirnext_r    = __fsa_dirnext,
        .dirclose_r   = __fsa_dirclose,
        .statvfs_r    = __fsa_statvfs,
        .ftruncate_r  = __fsa_ftruncate,
        .fsync_r      = __fsa_fsync,
        .chmod_r      = __fsa_chmod,
        .fchmod_r     = __fsa_fchmod,
        .rmdir_r      = __fsa_rmdir,
};

static bool fsa_initialised = false;
static FSADeviceData fsa_mounts[8];

static void fsaResetMount(FSADeviceData *mount, uint32_t id) {
    *mount = {};
    memcpy(&mount->device, &fsa_default_devoptab, sizeof(fsa_default_devoptab));
    mount->device.name         = mount->name;
    mount->device.deviceData   = mount;
    mount->id                  = id;
    mount->setup               = false;
    mount->mounted             = false;
    mount->clientHandle        = -1;
    mount->deviceSizeInSectors = 0;
    mount->deviceSectorSize    = 0;
    memset(mount->mount_path, 0, sizeof(mount->mount_path));
    memset(mount->name, 0, sizeof(mount->name));
    DCFlushRange(mount, sizeof(*mount));
}

void fsaInit() {
    if (!fsa_initialised) {
        uint32_t total = sizeof(fsa_mounts) / sizeof(fsa_mounts[0]);
        for (uint32_t i = 0; i < total; i++) {
            fsaResetMount(&fsa_mounts[i], i);
        }
        fsa_initialised = true;
    }
}

std::mutex fsaMutex;

FSADeviceData *fsa_alloc() {
    uint32_t i;
    uint32_t total = sizeof(fsa_mounts) / sizeof(fsa_mounts[0]);
    FSADeviceData *mount;

    fsaInit();

    for (i = 0; i < total; i++) {
        mount = &fsa_mounts[i];
        if (!mount->setup) {
            return mount;
        }
    }

    return nullptr;
}

static void fsa_free(FSADeviceData *mount) {
    int res;
    if (mount->mounted) {
        DEBUG_FUNCTION_LINE_ERR("Call FSAUnmount for %s with clientHandle 0x%08X", mount->mount_path, mount->clientHandle);
        // 2 = force?
        if ((res = FSAUnmount(mount->clientHandle, mount->mount_path, static_cast<FSAUnmountFlags>(2))) < 0) {
            DEBUG_FUNCTION_LINE_ERR("FSAUnmount failed: %d", res);
        }
    }
    res = FSADelClient(mount->clientHandle);
    if (res < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSADelClient failed: %d", res);
    }
    fsaResetMount(mount, mount->id);
}

int32_t fsaUnmount(const char *virt_name) {
    uint32_t total = sizeof(fsa_mounts) / sizeof(fsa_mounts[0]);

    fsaInit();

    for (uint32_t i = 0; i < total; i++) {
        FSADeviceData *mount = &fsa_mounts[i];
        if (!mount->setup) {
            continue;
        }
        if (strcmp(mount->name, virt_name) == 0) {
            fsa_free(mount);
            RemoveDevice(mount->name);
            return 0;
        }
    }

    DEBUG_FUNCTION_LINE_ERR("Failed to find fsa mount data for %s", virt_name);

    return -1;
}
extern int mochaInitDone;
int32_t fsaMount(const char *virt_name, const char *dev_path, const char *mount_path) {
    if (!mochaInitDone) {
        if (Mocha_InitLibrary() != MOCHA_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("Mocha_InitLibrary failed");
            return -80;
        }
        FSAInit();
    }
    std::lock_guard<std::mutex> lock(fsaMutex);

    DEBUG_FUNCTION_LINE_ERR("Mount %s %s %s", virt_name, dev_path, mount_path);

    FSADeviceData *mount = fsa_alloc();
    if (mount == nullptr) {
        DEBUG_FUNCTION_LINE_ERR("fsa_alloc() failed");
        OSMemoryBarrier();
        return -99;
    }

    mount->clientHandle = FSAAddClient(nullptr);
    if (mount->clientHandle < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAAddClient() failed: %d", mount->clientHandle);
        fsa_free(mount);
        return -98;
    }

    if (Mocha_UnlockFSClientEx(mount->clientHandle) != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Mocha_UnlockFSClientEx(mount->clientHandle) failed");
        return -79;
    }

    mount->mounted = false;

    strncpy(mount->name, virt_name, sizeof(mount->name) - 1);
    strncpy(mount->mount_path, mount_path, sizeof(mount->mount_path) - 1);
    FSError res;
    if (dev_path) {
        res = FSAMount(mount->clientHandle, dev_path, mount_path, FSA_MOUNT_FLAG_GLOBAL_MOUNT, nullptr, 0);
        if (res != 0) {
            DEBUG_FUNCTION_LINE_ERR("FSAMount(0x%08X, %s, %s, FSA_MOUNT_FLAG_GLOBAL_MOUNT, nullptr, 0) failed: %s", mount->clientHandle, dev_path, mount_path, FSAGetStatusStr(res));
            fsa_free(mount);
            return -96;
        }
        mount->mounted = true;
    } else {
        DEBUG_FUNCTION_LINE_ERR("To not mount, use existing mount");
        mount->mounted = false;
    }

    if ((res = FSAChangeDir(mount->clientHandle, mount->mount_path)) < 0) {
        DEBUG_FUNCTION_LINE_ERR("FSAChangeDir(0x%08X, %s) failed: %s", mount->clientHandle, mount->mount_path, FSAGetStatusStr(res));
    }

    FSADeviceInfo deviceInfo;
    if ((res = FSAGetDeviceInfo(mount->clientHandle, mount_path, &deviceInfo)) >= 0) {
        mount->deviceSizeInSectors = deviceInfo.deviceSizeInSectors;
        mount->deviceSectorSize    = deviceInfo.deviceSectorSize;
    } else {
        mount->deviceSizeInSectors = 0;
        mount->deviceSectorSize    = 0;
        DEBUG_FUNCTION_LINE_ERR("Failed to get DeviceInfo for %s: %s", mount_path, FSAGetStatusStr(res));

        //TODO fallback to other values?
        return -97;
    }


    if (AddDevice(&mount->device) < 0) {
        DEBUG_FUNCTION_LINE_ERR("AddDevice failed");
        fsa_free(mount);
        return -96;
    }

    mount->setup = true;


    return 0;
}
