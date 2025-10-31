#ifndef CONSOLE_H_

  #define CONSOLE_H_

  #include "config.h"

  #if defined(CONFIG_ENABLE_IO_SUBSYSTEM)

    #include "defines.h"

    #include "types.h"

    #include "device.h"

    #include "fat.h"

    #include "fs.h"

    #include "mem.h"

    #include "port.h"

    #include "posix.h"

    #include "queue.h"

    #include "streams.h"

    #include "sys.h"

    #include "task.h"

    #include "timer.h"

    #define CONSOLE_BANNER \

            "\r\n" \

            "   _    _      _ _  ____   _____ \r\n" \

            "  | |  | |    | (_)/ __ \\ / ____|\r\n" \

            "  | |__| | ___| |_| |  | | (___  \r\n" \

            "  |  __  |/ _ \\ | | |  | |\\___ \\ \r\n" \

            "  | |  | |  __/ | | |__| |____) |\r\n" \

            "  |_|  |_|\\___|_|_|\\____/|_____/ \r\n" \

            "\r\n" \

            "  HeliOS Embedded Operating System\r\n" \

            "  Version " OS_VERSION_STRING "\r\n" \

            "  (C) 2020-2026 Manny Peterson <manny@heliosproj.org>\r\n" \

            "  Licensed under GPL-2.0-or-later\r\n" \

            "\r\n"

    #if defined(CONSOLE_OK)

      #undef CONSOLE_OK

    #endif 

    #define CONSOLE_OK 0x00u

    #if defined(CONSOLE_ERROR)

      #undef CONSOLE_ERROR

    #endif 

    #define CONSOLE_ERROR 0x01u

    #if defined(CONSOLE_NOT_READY)

      #undef CONSOLE_NOT_READY

    #endif 

    #define CONSOLE_NOT_READY 0x02u

    typedef struct ConsoleState_s {

      Base_t deviceReady;

      Base_t echoEnabled;

      Byte_t commandBuffer[CONFIG_CONSOLE_MAX_COMMAND_LENGTH];

      HalfWord_t bufferPosition;

      Byte_t currentWorkingDirectory[CONFIG_FS_MAX_PATH_LENGTH];

    } ConsoleState_t;

    #ifdef __cplusplus

      extern "C" {

    #endif 

    Return_t xConsoleInit(void);

    void vConsoleTask(Task_t *task_, TaskParm_t *parm_);

    #if defined(POSIX_ARCH_OTHER)

      void __ConsoleStateClear__(void);

    #endif 

    Size_t __strlen__(const Byte_t *str_);

    Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);

    Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);

    Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_);

    Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);

    Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);

    Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_);

    Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_);

    Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);

    Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);

    Base_t __path_is_absolute__(const Byte_t *path_);

    Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);

    Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);

    #ifdef __cplusplus

      }

    #endif 

  #endif 

#endif 

