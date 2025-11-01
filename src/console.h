/*UNCRUSTIFY-OFF*/
/**
 * @file console.h
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Console I/O API header
 * @details
 * Defines console structures and function prototypes for character-based input/output operations including formatted printing and line reading.
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
/*UNCRUSTIFY-ON*/


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


    #define CONSOLE_BANNER                                        \
            "\r\n"                                                      \
            "   _    _      _ _  ____   _____ \r\n"                     \
            "  | |  | |    | (_)/ __ \\ / ____|\r\n"                    \
            "  | |__| | ___| |_| |  | | (___  \r\n"                     \
            "  |  __  |/ _ \\ | | |  | |\\___ \\ \r\n"                  \
            "  | |  | |  __/ | | |__| |____) |\r\n"                     \
            "  |_|  |_|\\___|_|_|\\____/|_____/ \r\n"                   \
            "\r\n"                                                      \
            "  HeliOS Embedded Operating System\r\n"                    \
            "  Version " OS_VERSION_STRING                              \
            "\r\n"                                                      \
            "  (C) 2020-2026 Manny Peterson <manny@heliosproj.org>\r\n" \
            "  Licensed under GPL-2.0-or-later\r\n"                     \
            "\r\n"


    #if defined(CONSOLE_OK)


      #undef CONSOLE_OK


    #endif /* if defined(CONSOLE_OK) */


    #define CONSOLE_OK 0x00u


    #if defined(CONSOLE_ERROR)


      #undef CONSOLE_ERROR


    #endif /* if defined(CONSOLE_ERROR) */


    #define CONSOLE_ERROR 0x01u


    #if defined(CONSOLE_NOT_READY)


      #undef CONSOLE_NOT_READY


    #endif /* if defined(CONSOLE_NOT_READY) */


    #define CONSOLE_NOT_READY 0x02u


    #define CHAR_NULL 0x00u


    #define CHAR_TAB 0x09u


    #define CHAR_LF 0x0Au


    #define CHAR_CR 0x0Du


    #define CHAR_SPACE 0x20u


    #define CHAR_SLASH 0x2Fu


    #define CHAR_DOT 0x2Eu


    #define CHAR_ZERO 0x30u


    #define CHAR_LOWERCASE_X 0x78u


    #define CHAR_BACKSPACE 0x08u


    #define CHAR_DEL 0x7Fu


    #define CHAR_PRINTABLE_MIN 0x20u


    #define CHAR_PRINTABLE_MAX 0x7Eu

/**
 * @brief Console state structure
 * @details Maintains the state of the console subsystem including buffer and settings.
 */
    typedef struct ConsoleState_s {
      Base_t deviceReady;                                    /**< Flag indicating if console device is ready for I/O */
      Base_t echoEnabled;                                    /**< Flag indicating if character echo is enabled */
      Byte_t commandBuffer[CONFIG_CONSOLE_MAX_COMMAND_LENGTH]; /**< Command input buffer */
      HalfWord_t bufferPosition;                             /**< Current position in command buffer */
      Byte_t currentWorkingDirectory[CONFIG_FS_MAX_PATH_LENGTH]; /**< Current working directory path */
    } ConsoleState_t;

    typedef struct ConsoleCommand_s {
      const Byte_t *name;

      Return_t (*handler)(const Byte_t *);

      const Byte_t *description;
    } ConsoleCommand_t;

    #ifdef __cplusplus


      extern "C" {


    #endif /* ifdef __cplusplus */
    Return_t xConsoleInit(void);
    void vConsoleTask(Task_t *task_, TaskParm_t *parm_);
    #if defined(POSIX_ARCH_OTHER)
      void __ConsoleStateClear__(void);
    #endif /* if defined(POSIX_ARCH_OTHER) */
    Size_t __strlen__(const Byte_t *str_, const Size_t maxLen_);
    Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
    Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);
    Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t maxLen_);
    Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);
    Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
    Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_, const Size_t maxLen_);
    Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_, const Size_t maxLen_);
    Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);
    Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);
    Base_t __path_is_absolute__(const Byte_t *path_);
    Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
    Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
    Return_t __ConsoleCmdHelp__(const Byte_t *args_);
    Return_t __ConsoleCmdVersion__(const Byte_t *args_);
    Return_t __ConsoleCmdTasks__(const Byte_t *args_);
    Return_t __ConsoleCmdMem__(const Byte_t *args_);
    Return_t __ConsoleCmdClear__(const Byte_t *args_);
    Return_t __ConsoleCmdEcho__(const Byte_t *args_);
    Return_t __ConsoleCmdLs__(const Byte_t *args_);
    Return_t __ConsoleCmdCd__(const Byte_t *args_);
    Return_t __ConsoleCmdPwd__(const Byte_t *args_);
    Return_t __ConsoleCmdCat__(const Byte_t *args_);
    Return_t __ConsoleCmdMv__(const Byte_t *args_);
    Return_t __ConsoleCmdRm__(const Byte_t *args_);
    Return_t __ConsoleCmdMkdir__(const Byte_t *args_);
    Return_t __ConsoleWriteString__(const Byte_t *str_);
    Return_t __ConsoleReadChar__(Byte_t *ch_);
    Return_t __ConsoleCheckDevice__(void);
    Return_t __ConsoleProcessCommand__(void);
    void __ConsolePrintPrompt__(void);
    Return_t __ConsoleHandleBackspace__(void);
    void __SkipWhitespace__(const Byte_t **str_);
    void __uitoah__(Word_t value_, Byte_t *buffer_, Word_t bufferSize_);

    #ifdef __cplusplus
      }


    #endif /* ifdef __cplusplus */


  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */


#endif /* ifndef CONSOLE_H_ */