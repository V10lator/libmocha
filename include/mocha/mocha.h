#pragma once

#include <coreinit/filesystem.h>
#include <mocha/commands.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum MochaUtilsStatus {
    MOCHA_RESULT_SUCCESS                 = 0,
    MOCHA_RESULT_INVALID_ARGUMENT        = -0x01,
    MOCHA_RESULT_MAX_CLIENT              = -0x02,
    MOCHA_RESULT_OUT_OF_MEMORY           = -0x03,
    MOCHA_RESULT_ALREADY_EXISTS          = -0x04,
    MOCHA_RESULT_ADD_DEVOPTAB_FAILED     = -0x05,
    MOCHA_RESULT_NOT_FOUND               = -0x06,
    MOCHA_RESULT_UNSUPPORTED_API_VERSION = -0x10,
    MOCHA_RESULT_UNSUPPORTED_COMMAND     = -0x11,
    MOCHA_RESULT_LIB_UNINITIALIZED       = -0x20,
    MOCHA_RESULT_UNKNOWN_ERROR           = -0x100,
};

/**
 * Initializes the mocha lib. Needs to be called before any other functions can be used
 * @return MOCHA_RESULT_SUCCESS: Library has been successfully initialized <br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Failed to initialize the library caused by an outdated mocha version.
 */
MochaUtilsStatus Mocha_InitLibrary();

/**
 * Deinitializes the mocha lib
 * @return
 */
MochaUtilsStatus Mocha_DeInitLibrary();

/**
 * Retrieves the API Version of the running mocha.
 *
 * @param outVersion pointer to the variable where the version will be stored.
 *
 * @return MOCHA_RESULT_SUCCESS: The API version has been store in the version ptr<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: invalid version pointer<br>
 *         MOCHA_RESULT_UNSUPPORTED_API_VERSION: Failed to get the API version caused by an outdated mocha version.
 */
MochaUtilsStatus Mocha_CheckAPIVersion(uint32_t *outVersion);

/***
 * Returns the path of the currently loaded environment
 * @param environmentPathBuffer: buffer where the result will be stored
 * @param bufferLen: length of the buffer. Required to be >= 0x100
* @return MOCHA_RESULT_SUCCESS: The environment path has been stored in environmentPathBuffer<br>
*         MOCHA_RESULT_INVALID_ARGUMENT: invalid environmentPathBuffer pointer or bufferLen \< 0x100<br>
*         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
*         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
*         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_GetEnvironmentPath(char *environmentPathBuffer, uint32_t bufferLen);

/***
 * Signals Mocha to not redirect the men.rpx to root.rpx anymore
* @return MOCHA_RESULT_SUCCESS <br>
*         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
*         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
*         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_RPXHookCompleted();

/***
* Starts the MCP Thread in mocha to allows usage of /dev/iosuhax and wupclient
* @return MOCHA_RESULT_SUCCESS<br>
*         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
*         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
*         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_StartMCPThread();

/***
* Starts the MCP Thread in mocha to allows usage of /dev/iosuhax and wupclient
* @return MOCHA_RESULT_SUCCESS<br>
*         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
*         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
*         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_StartUSBLogging(bool avoidLogCatchup);

/**
 * Gives a FSClient full permissions. <br>
 * Requires Mocha API Version: 1
 * @param client The FSClient that should have full permission
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully. <br>
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to unlock a given FSClient
 */
MochaUtilsStatus Mocha_UnlockFSClient(FSClient *client);

/**
 * Gives a /dev/fsa handle full permissions. <br>
 * Requires Mocha API Version: 1
 * @param client The /dev/fsa handle that should have full permission
 * @return MOCHA_RESULT_SUCCESS: The has been unlocked successfully. <br>
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FS Clients have been unlocked.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to unlock the given client
 */
MochaUtilsStatus Mocha_UnlockFSClientEx(int clientHandle);

/**
 * After successfully calling this function, the next loaded .rpx will be replaced according to the given loadInfo.<br>
 * <br>
 * Loading a .rpx from within a file (archive e.g. a WUHB) is supported. <br>
 * To achieve this, the fileoffset (offset inside file specified via path) and filesize (size of the .rpx) need to be set. <br>
 * If filesize is set to 0, the whole file (starting at fileoffset) will be loaded as .rpx <br>
 *
 * The path is **relative** to the root of the given target device.
 *
 * @param loadInfo Information about the .rpx replacement.
 * @return MOCHA_RESULT_SUCCESS: Loading the next RPX will be redirected. <br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to setup a redirect of RPX.
 */
MochaUtilsStatus Mocha_LoadRPXOnNextLaunch(MochaRPXLoadInfo *loadInfo);

typedef struct WUDDiscKey {
    uint8_t key[0x10];
} WUDDiscKey;

/**
 * Reads the disc key (used to decrypt the SI partition) of the inserted disc.
 *
 * @param discKey target buffer where the result will be stored.
 * @return MOCHA_RESULT_SUCCESS: The disc key of the inserted disc has been read into the given buffer.<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given discKey buffer was NULL <br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to read the discKey.
 */
MochaUtilsStatus Mocha_ODMGetDiscKey(WUDDiscKey *discKey);

/**
 * Reads *size* bytes from *offset* of the SEEPROM of the console. Total size of SEEPROM is 0x200
 * @param out_buffer buffer where the result will be stored
 * @param offset offset in bytes. Must be an even number.
 * @param size size in bytes
 * @return MOCHA_RESULT_SUCCESS: The SEEPROM has been read into the given buffer.<br>
 *         MOCHA_RESULT_INVALID_ARGUMENT: The given out_buffer was NULL or the offset was < 0 or an odd value<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to read the seeprom.
 */
MochaUtilsStatus Mocha_SEEPROMRead(uint8_t *out_buffer, uint32_t offset, uint32_t size);

/**
 * Mounts a device (dev_path) to a given path (mount_path) and make a accessible via the
 * newlib devoptab (standard POSIX file I/O)
 *
 * Requires Mocha API Version: 1
 * @param virt_name Name which should be used for the devoptab. When choosing e.g. "storage_usb" the mounted device can be accessed via "storage_usb:/".
 * @param dev_path (optional) Cafe OS internal device path (e.g. /dev/slc01). If the given dev_path is NULL, an existing mount will be used (and is expected)
 * @param mount_path Path where CafeOS should mount the device to. Must be globally unique and start with "/vol/storage_"
 * @return MOCHA_RESULT_SUCCESS: The device has been mounted successfully <br>
 *         MOCHA_RESULT_MAX_CLIENT: The maximum number of FSAClients have been registered.<br>
 *         MOCHA_RESULT_LIB_UNINITIALIZED: Library was not initialized. Call Mocha_InitLibrary() before using this function.<br>
 *         MOCHA_RESULT_UNSUPPORTED_COMMAND: Command not supported by the currently loaded mocha version.<br>
 *         MOCHA_RESULT_UNKNOWN_ERROR: Failed to retrieve the environment path.
 */
MochaUtilsStatus Mocha_MountFS(const char *virt_name, const char *dev_path, const char *mount_path);

MochaUtilsStatus Mocha_UnmountFS(const char *virt_name);

#ifdef __cplusplus
} // extern "C"
#endif