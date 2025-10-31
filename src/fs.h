/*UNCRUSTIFY-OFF*/
/**
 * @file fs.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Filesystem abstraction layer API header
 * @details
 * Defines volume structures, file handles, and function prototypes for FAT32 filesystem operations including mounting, file I/O, and directory navigation.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/
#ifndef FS_H_

  #define FS_H_

  #include "config.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "defines.h"

    #include "types.h"

    #include "console.h"

    #include "device.h"

    #include "fat.h"

    #include "mem.h"

    #include "port.h"

    #include "posix.h"

    #include "queue.h"

    #include "streams.h"

    #include "sys.h"

    #include "task.h"

    #include "timer.h"

    #if defined(FS_MODE_READ)

      #undef FS_MODE_READ

    #endif /* if defined(FS_MODE_READ) */

    #define FS_MODE_READ 0x01u

    #if defined(FS_MODE_WRITE)

      #undef FS_MODE_WRITE

    #endif /* if defined(FS_MODE_WRITE) */

    #define FS_MODE_WRITE 0x02u

    #if defined(FS_MODE_APPEND)

      #undef FS_MODE_APPEND

    #endif /* if defined(FS_MODE_APPEND) */

    #define FS_MODE_APPEND 0x04u

    #if defined(FS_MODE_CREATE)

      #undef FS_MODE_CREATE

    #endif /* if defined(FS_MODE_CREATE) */

    #define FS_MODE_CREATE 0x08u

    #if defined(FS_SEEK_SET)

      #undef FS_SEEK_SET

    #endif /* if defined(FS_SEEK_SET) */

    #define FS_SEEK_SET 0x00u

    #if defined(FS_SEEK_CUR)

      #undef FS_SEEK_CUR

    #endif /* if defined(FS_SEEK_CUR) */

    #define FS_SEEK_CUR 0x01u

    #if defined(FS_SEEK_END)

      #undef FS_SEEK_END

    #endif /* if defined(FS_SEEK_END) */

    #define FS_SEEK_END 0x02u

    #if !defined(CONFIG_FS_MAX_PATH_LENGTH)

      #define CONFIG_FS_MAX_PATH_LENGTH 256u

    #endif /* if !defined(CONFIG_FS_MAX_PATH_LENGTH) */

    #ifdef __cplusplus

      extern "C" {

    #endif /* ifdef __cplusplus */
    /**
     * @brief Mounts a FAT32 filesystem volume
     * @details Allocates and initializes a volume structure by reading the boot sector and FAT.
     *
     * @param[out] volume_ Pointer to store the created volume handle
     *
     * @return ReturnOK if volume was mounted successfully
     * @return ReturnError if mount failed, allocation failed, or invalid filesystem
     *
     * @warning Caller is responsible for unmounting with xFSUnmount()
     */
    Return_t xFSMount(Volume_t **volume_);

    /**
     * @brief Unmounts a FAT32 filesystem volume
     * @details Flushes any pending writes and frees the volume structure.
     *
     * @param[in,out] volume_ Pointer to the volume to unmount
     *
     * @return ReturnOK if volume was unmounted successfully
     * @return ReturnError if volume is invalid or deallocation failed
     *
     * @warning All open files and directories must be closed before unmounting
     */
    Return_t xFSUnmount(Volume_t *volume_);

    /**
     * @brief Gets volume information and statistics
     * @details Allocates and returns detailed information about the volume including capacity and free space.
     *
     * @param[in] volume_ Pointer to the volume to query
     * @param[out] info_ Pointer to store allocated volume information structure
     *
     * @return ReturnOK if information was retrieved successfully
     * @return ReturnError if volume is invalid, allocation failed, or invalid parameter
     *
     * @warning Caller is responsible for freeing the allocated structure
     */
    Return_t xFSGetVolumeInfo(const Volume_t *volume_, VolumeInfo_t **info_);

    /**
     * @brief Formats a volume with FAT32 filesystem
     * @details Creates a new FAT32 filesystem on the default block device.
     *
     * @param[in] volumeLabel_ Volume label string (11 characters max)
     *
     * @return ReturnOK if format was successful
     * @return ReturnError if format failed or device unavailable
     *
     * @warning This operation destroys all existing data on the volume
     */
    Return_t xFSFormat(const Byte_t *volumeLabel_);

    /**
     * @brief Opens a file
     * @details Allocates and initializes a file handle for the specified path with the given mode.
     *
     * @param[out] file_ Pointer to store the created file handle
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ File path (null-terminated string)
     * @param[in] mode_ Access mode flags (FS_MODE_READ, FS_MODE_WRITE, FS_MODE_APPEND, FS_MODE_CREATE)
     *
     * @return ReturnOK if file was opened successfully
     * @return ReturnError if file not found, allocation failed, or invalid parameters
     *
     * @warning Caller is responsible for closing the file with xFileClose()
     */
    Return_t xFileOpen(File_t **file_, Volume_t *volume_, const Byte_t *path_, const Byte_t mode_);

    /**
     * @brief Closes a file
     * @details Flushes any pending writes and frees the file handle.
     *
     * @param[in,out] file_ Pointer to the file to close
     *
     * @return ReturnOK if file was closed successfully
     * @return ReturnError if file is invalid or deallocation failed
     */
    Return_t xFileClose(File_t *file_);

    /**
     * @brief Reads data from a file
     * @details Allocates and returns data read from the current file position.
     *
     * @param[in,out] file_ Pointer to the file to read from
     * @param[in] size_ Number of bytes to read
     * @param[out] data_ Pointer to store allocated data buffer
     *
     * @return ReturnOK if read was successful
     * @return ReturnError if read failed, allocation failed, or invalid parameters
     *
     * @warning Caller is responsible for freeing the allocated buffer
     */
    Return_t xFileRead(File_t *file_, const Size_t size_, Byte_t **data_);

    /**
     * @brief Writes data to a file
     * @details Writes data to the file at the current position.
     *
     * @param[in,out] file_ Pointer to the file to write to
     * @param[in] size_ Number of bytes to write
     * @param[in] data_ Pointer to data buffer
     *
     * @return ReturnOK if write was successful
     * @return ReturnError if write failed, disk full, or invalid parameters
     */
    Return_t xFileWrite(File_t *file_, const Size_t size_, const Byte_t *data_);

    /**
     * @brief Seeks to a position in a file
     * @details Changes the file position for subsequent read/write operations.
     *
     * @param[in,out] file_ Pointer to the file
     * @param[in] offset_ Byte offset for the seek operation
     * @param[in] origin_ Seek origin (FS_SEEK_SET, FS_SEEK_CUR, FS_SEEK_END)
     *
     * @return ReturnOK if seek was successful
     * @return ReturnError if seek failed or invalid parameters
     */
    Return_t xFileSeek(File_t *file_, const Word_t offset_, const Byte_t origin_);

    /**
     * @brief Gets the current file position
     * @details Returns the byte offset of the current read/write position.
     *
     * @param[in] file_ Pointer to the file
     * @param[out] position_ Pointer to store the current position
     *
     * @return ReturnOK if position was retrieved successfully
     * @return ReturnError if file is invalid or invalid parameter
     */
    Return_t xFileTell(const File_t *file_, Word_t *position_);

    /**
     * @brief Gets the size of a file
     * @details Returns the total size of the file in bytes.
     *
     * @param[in] file_ Pointer to the file
     * @param[out] size_ Pointer to store the file size
     *
     * @return ReturnOK if size was retrieved successfully
     * @return ReturnError if file is invalid or invalid parameter
     */
    Return_t xFileGetSize(const File_t *file_, Word_t *size_);

    /**
     * @brief Synchronizes file data to disk
     * @details Flushes all pending writes to ensure data is written to the storage device.
     *
     * @param[in,out] file_ Pointer to the file to sync
     *
     * @return ReturnOK if sync was successful
     * @return ReturnError if sync failed or file is invalid
     */
    Return_t xFileSync(File_t *file_);

    /**
     * @brief Truncates a file to specified size
     * @details Resizes the file, either expanding or shrinking it.
     *
     * @param[in,out] file_ Pointer to the file to truncate
     * @param[in] size_ New file size in bytes
     *
     * @return ReturnOK if truncate was successful
     * @return ReturnError if truncate failed or file is invalid
     */
    Return_t xFileTruncate(File_t *file_, const Word_t size_);

    /**
     * @brief Checks if file position is at end-of-file
     * @details Returns true if the current position is at or beyond the end of the file.
     *
     * @param[in] file_ Pointer to the file to check
     * @param[out] eof_ Pointer to store the result (true if at EOF, false otherwise)
     *
     * @return ReturnOK if check was successful
     * @return ReturnError if file is invalid or invalid parameter
     */
    Return_t xFileEOF(const File_t *file_, Base_t *eof_);

    /**
     * @brief Opens a directory for reading
     * @details Allocates and initializes a directory handle for iteration.
     *
     * @param[out] dir_ Pointer to store the created directory handle
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ Directory path (null-terminated string)
     *
     * @return ReturnOK if directory was opened successfully
     * @return ReturnError if directory not found, allocation failed, or invalid parameters
     *
     * @warning Caller is responsible for closing with xDirClose()
     */
    Return_t xDirOpen(Dir_t **dir_, Volume_t *volume_, const Byte_t *path_);

    /**
     * @brief Closes a directory
     * @details Frees the directory handle.
     *
     * @param[in,out] dir_ Pointer to the directory to close
     *
     * @return ReturnOK if directory was closed successfully
     * @return ReturnError if directory is invalid or deallocation failed
     */
    Return_t xDirClose(Dir_t *dir_);

    /**
     * @brief Reads the next directory entry
     * @details Allocates and returns the next entry in the directory.
     *
     * @param[in,out] dir_ Pointer to the directory
     * @param[out] entry_ Pointer to store allocated directory entry
     *
     * @return ReturnOK if entry was read successfully
     * @return ReturnError if end of directory, allocation failed, or invalid parameters
     *
     * @warning Caller is responsible for freeing the allocated entry
     */
    Return_t xDirRead(Dir_t *dir_, DirEntry_t **entry_);

    /**
     * @brief Rewinds directory to first entry
     * @details Resets the directory position to the beginning.
     *
     * @param[in,out] dir_ Pointer to the directory to rewind
     *
     * @return ReturnOK if rewind was successful
     * @return ReturnError if directory is invalid
     */
    Return_t xDirRewind(Dir_t *dir_);

    /**
     * @brief Creates a new directory
     * @details Creates a directory at the specified path.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ Directory path to create (null-terminated string)
     *
     * @return ReturnOK if directory was created successfully
     * @return ReturnError if creation failed, path exists, or invalid parameters
     */
    Return_t xDirMake(Volume_t *volume_, const Byte_t *path_);

    /**
     * @brief Removes a directory
     * @details Deletes an empty directory at the specified path.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ Directory path to remove (null-terminated string)
     *
     * @return ReturnOK if directory was removed successfully
     * @return ReturnError if removal failed, directory not empty, or invalid parameters
     */
    Return_t xDirRemove(Volume_t *volume_, const Byte_t *path_);

    /**
     * @brief Checks if a file exists
     * @details Returns true if a file or directory exists at the specified path.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ File path to check (null-terminated string)
     * @param[out] exists_ Pointer to store the result (true if exists, false otherwise)
     *
     * @return ReturnOK if check was successful
     * @return ReturnError if volume is invalid or invalid parameters
     */
    Return_t xFileExists(Volume_t *volume_, const Byte_t *path_, Base_t *exists_);

    /**
     * @brief Deletes a file
     * @details Removes the file at the specified path.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ File path to delete (null-terminated string)
     *
     * @return ReturnOK if file was deleted successfully
     * @return ReturnError if deletion failed, file not found, or invalid parameters
     */
    Return_t xFileUnlink(Volume_t *volume_, const Byte_t *path_);

    /**
     * @brief Renames or moves a file
     * @details Changes the path of a file or directory.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] oldPath_ Current file path (null-terminated string)
     * @param[in] newPath_ New file path (null-terminated string)
     *
     * @return ReturnOK if rename was successful
     * @return ReturnError if rename failed, file not found, or invalid parameters
     */
    Return_t xFileRename(Volume_t *volume_, const Byte_t *oldPath_, const Byte_t *newPath_);

    /**
     * @brief Gets file or directory information
     * @details Allocates and returns metadata about a file or directory.
     *
     * @param[in] volume_ Pointer to the mounted volume
     * @param[in] path_ File or directory path (null-terminated string)
     * @param[out] entry_ Pointer to store allocated directory entry
     *
     * @return ReturnOK if information was retrieved successfully
     * @return ReturnError if file not found, allocation failed, or invalid parameters
     *
     * @warning Caller is responsible for freeing the allocated entry
     */
    Return_t xFileGetInfo(Volume_t *volume_, const Byte_t *path_, DirEntry_t **entry_);

    #ifdef __cplusplus

      }

    #endif /* ifdef __cplusplus */

  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */

#endif /* ifndef FS_H_ */