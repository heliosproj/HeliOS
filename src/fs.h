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

    #endif 

    #define FS_MODE_READ 0x01u 

    #if defined(FS_MODE_WRITE)

      #undef FS_MODE_WRITE

    #endif 

    #define FS_MODE_WRITE 0x02u 

    #if defined(FS_MODE_APPEND)

      #undef FS_MODE_APPEND

    #endif 

    #define FS_MODE_APPEND 0x04u 

    #if defined(FS_MODE_CREATE)

      #undef FS_MODE_CREATE

    #endif 

    #define FS_MODE_CREATE 0x08u 

    #if defined(FS_SEEK_SET)

      #undef FS_SEEK_SET

    #endif 

    #define FS_SEEK_SET 0x00u 

    #if defined(FS_SEEK_CUR)

      #undef FS_SEEK_CUR

    #endif 

    #define FS_SEEK_CUR 0x01u 

    #if defined(FS_SEEK_END)

      #undef FS_SEEK_END

    #endif 

    #define FS_SEEK_END 0x02u 

    #if !defined(CONFIG_FS_MAX_PATH_LENGTH)

      #define CONFIG_FS_MAX_PATH_LENGTH 256u 

    #endif 

    #ifdef __cplusplus

      extern "C" {

    #endif 

    Return_t xFSMount(Volume_t **volume_);

    Return_t xFSUnmount(Volume_t *volume_);

    Return_t xFSGetVolumeInfo(const Volume_t *volume_, VolumeInfo_t **info_);

    Return_t xFSFormat(const Byte_t *volumeLabel_);

    Return_t xFileOpen(File_t **file_, Volume_t *volume_, const Byte_t *path_, const Byte_t mode_);

    Return_t xFileClose(File_t *file_);

    Return_t xFileRead(File_t *file_, const Size_t size_, Byte_t **data_);

    Return_t xFileWrite(File_t *file_, const Size_t size_, const Byte_t *data_);

    Return_t xFileSeek(File_t *file_, const Word_t offset_, const Byte_t origin_);

    Return_t xFileTell(const File_t *file_, Word_t *position_);

    Return_t xFileGetSize(const File_t *file_, Word_t *size_);

    Return_t xFileSync(File_t *file_);

    Return_t xFileTruncate(File_t *file_, const Word_t size_);

    Return_t xFileEOF(const File_t *file_, Base_t *eof_);

    Return_t xDirOpen(Dir_t **dir_, Volume_t *volume_, const Byte_t *path_);

    Return_t xDirClose(Dir_t *dir_);

    Return_t xDirRead(Dir_t *dir_, DirEntry_t **entry_);

    Return_t xDirRewind(Dir_t *dir_);

    Return_t xDirMake(Volume_t *volume_, const Byte_t *path_);

    Return_t xDirRemove(Volume_t *volume_, const Byte_t *path_);

    Return_t xFileExists(Volume_t *volume_, const Byte_t *path_, Base_t *exists_);

    Return_t xFileUnlink(Volume_t *volume_, const Byte_t *path_);

    Return_t xFileRename(Volume_t *volume_, const Byte_t *oldPath_, const Byte_t *newPath_);

    Return_t xFileGetInfo(Volume_t *volume_, const Byte_t *path_, DirEntry_t **entry_);

    #ifdef __cplusplus

      }

    #endif 

  #endif 

#endif 

