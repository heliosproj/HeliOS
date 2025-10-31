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


    /**
     * @brief Console state structure
     * @details Maintains the state of the console subsystem including buffer and settings.
     */
    typedef struct ConsoleState_s {


      Base_t deviceReady; /**< Flag indicating if console device is ready for I/O */
      Base_t echoEnabled; /**< Flag indicating if character echo is enabled */
      Byte_t commandBuffer[CONFIG_CONSOLE_MAX_COMMAND_LENGTH]; /**< Command input buffer
                                                                */
      HalfWord_t bufferPosition; /**< Current position in command buffer */
      Byte_t currentWorkingDirectory[CONFIG_FS_MAX_PATH_LENGTH]; /**< Current working directory path */
    } ConsoleState_t;


    #ifdef __cplusplus


      extern "C" {


    #endif /* ifdef __cplusplus */
    Return_t xConsoleInit(void);
    /**
     * @brief Console task callback function
     * @details Processes console input/output and command handling.
     *
     * @param[in,out] task_ Pointer to the task structure
     * @param[in]     parm_ Task parameter (unused)
     *
     * @note This function is designed to be used as a task callback
     */
    void vConsoleTask(Task_t *task_, TaskParm_t *parm_);


    #if defined(POSIX_ARCH_OTHER)
      void __ConsoleStateClear__(void);


    #endif /* if defined(POSIX_ARCH_OTHER) */
    /**
     * @brief Gets the length of a string
     * @details Internal string length implementation.
     *
     * @param[in] str_ Null-terminated string
     *
     * @return         Length of the string in bytes (excluding null terminator)
     *
     * @note This is an internal function similar to standard strlen
     */
    Size_t __strlen__(const Byte_t *str_);
    Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
    Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t n_);
    /**
     * @brief Compares two strings
     * @details Internal string comparison implementation.
     *
     * @param[in] s1_ First string
     * @param[in] s2_ Second string
     *
     * @return        0 if strings are equal, non-zero otherwise
     *
     * @note This is an internal function similar to standard strcmp
     */
    Base_t __strcmp__(const Byte_t *s1_, const Byte_t *s2_);
    /**
     * @brief Compares at most n characters of two strings
     * @details Internal bounded string comparison implementation.
     *
     * @param[in] s1_ First string
     * @param[in] s2_ Second string
     * @param[in] n_  Maximum number of characters to compare
     *
     * @return        0 if strings are equal, non-zero otherwise
     *
     * @note This is an internal function similar to standard strncmp
     */
    Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t n_);
    Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t destSize_);
    /**
     * @brief Finds first occurrence of character in string
     * @details Internal character search from the beginning.
     *
     * @param[in] str_ String to search
     * @param[in] ch_  Character to find
     *
     * @return         Pointer to first occurrence, or NULL if not found
     *
     * @note This is an internal function similar to standard strchr
     */
    Byte_t * __strchr__(const Byte_t *str_, const Byte_t ch_);
    /**
     * @brief Finds last occurrence of character in string
     * @details Internal character search from the end.
     *
     * @param[in] str_ String to search
     * @param[in] ch_  Character to find
     *
     * @return         Pointer to last occurrence, or NULL if not found
     *
     * @note This is an internal function similar to standard strrchr
     */
    Byte_t * __strrchr__(const Byte_t *str_, const Byte_t ch_);
    Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, const Size_t destSize_);
    Return_t __path_normalize__(Byte_t *path_, const Size_t pathSize_);
    /**
     * @brief Checks if path is absolute
     * @details Internal check for absolute vs relative path.
     *
     * @param[in] path_ Path to check
     *
     * @return          Non-zero if path is absolute, 0 if relative
     *
     * @note This is an internal function for filesystem path manipulation
     */
    Base_t __path_is_absolute__(const Byte_t *path_);
    Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);
    Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, const Size_t destSize_);


    #ifdef __cplusplus


      }


    #endif /* ifdef __cplusplus */


  #endif /* if defined(CONFIG_ENABLE_IO_SUBSYSTEM) */


#endif /* ifndef CONSOLE_H_ */