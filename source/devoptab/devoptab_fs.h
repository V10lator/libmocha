#pragma once
#include <coreinit/filesystem_fsa.h>

#include "FastLockWrapper.h"
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <malloc.h>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/param.h>
#include <unistd.h>

typedef struct FSADeviceData {
    devoptab_t device{};
    bool setup{};
    bool mounted{};
    uint32_t id{};
    char name[32]{};
    char mount_path[256]{};
    uint64_t deviceSizeInSectors{};
    uint32_t deviceSectorSize{};
    FSAClientHandle clientHandle{};
    FastLockWrapper mutex;
} FSADeviceData;

/**
 * Open file struct
 */
typedef struct
{
    //! FS handle
    FSAFileHandle fd;

    //! Flags used in open(2)
    int flags;

    //! Current file offset
    uint32_t offset;

    //! Path stored for internal path tracking
    char path[FS_MAX_PATH + 1];
} __fsa_file_t;

/**
 * Open directory struct
 */
typedef struct
{
    //! Should be set to FS_DIRITER_MAGIC
    uint32_t magic;

    //! FS handle
    FSADirectoryHandle fd;

    //! Temporary storage for reading entries
    FSADirectoryEntry entry_data;
} __fsa_dir_t;

#define FS_DIRITER_MAGIC 0x77696975


#ifdef __cplusplus
extern "C" {
#endif

int __fsa_open(struct _reent *r, void *fileStruct, const char *path,
               int flags, int mode);
int __fsa_close(struct _reent *r, void *fd);
ssize_t __fsa_write(struct _reent *r, void *fd, const char *ptr,
                    size_t len);
ssize_t __fsa_read(struct _reent *r, void *fd, char *ptr, size_t len);
off_t __fsa_seek(struct _reent *r, void *fd, off_t pos, int dir);
int __fsa_fstat(struct _reent *r, void *fd, struct stat *st);
int __fsa_stat(struct _reent *r, const char *file, struct stat *st);
int __fsa_link(struct _reent *r, const char *existing,
               const char *newLink);
int __fsa_unlink(struct _reent *r, const char *name);
int __fsa_chdir(struct _reent *r, const char *name);
int __fsa_rename(struct _reent *r, const char *oldName,
                 const char *newName);
int __fsa_mkdir(struct _reent *r, const char *path, int mode);
DIR_ITER *__fsa_diropen(struct _reent *r, DIR_ITER *dirState,
                        const char *path);
int __fsa_dirreset(struct _reent *r, DIR_ITER *dirState);
int __fsa_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename,
                  struct stat *filestat);
int __fsa_dirclose(struct _reent *r, DIR_ITER *dirState);
int __fsa_statvfs(struct _reent *r, const char *path,
                  struct statvfs *buf);
int __fsa_ftruncate(struct _reent *r, void *fd, off_t len);
int __fsa_fsync(struct _reent *r, void *fd);
int __fsa_chmod(struct _reent *r, const char *path, mode_t mode);
int __fsa_fchmod(struct _reent *r, void *fd, mode_t mode);
int __fsa_rmdir(struct _reent *r, const char *name);

// devoptab_fs_utils.c
char *__fsa_fixpath(struct _reent *r, const char *path);
int __fsa_translate_error(FSError error);
time_t __fsa_translate_time(FSTime timeValue);
FSMode __fsa_translate_permission_mode(int mode);
mode_t __fsa_translate_stat_mode(FSStat fileStat);

#ifdef __cplusplus
}
#endif