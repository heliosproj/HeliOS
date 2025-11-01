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
    #if defined(CHAR_NULL)
      #undef CHAR_NULL
    #endif
    #define CHAR_NULL 0x00u
    #if defined(CHAR_TAB)
      #undef CHAR_TAB
    #endif
    #define CHAR_TAB 0x09u
    #if defined(CHAR_LF)
      #undef CHAR_LF
    #endif
    #define CHAR_LF 0x0Au
    #if defined(CHAR_CR)
      #undef CHAR_CR
    #endif
    #define CHAR_CR 0x0Du
    #if defined(CHAR_SPACE)
      #undef CHAR_SPACE
    #endif
    #define CHAR_SPACE 0x20u
    #if defined(CHAR_SLASH)
      #undef CHAR_SLASH
    #endif
    #define CHAR_SLASH 0x2Fu
    #if defined(CHAR_DOT)
      #undef CHAR_DOT
    #endif
    #define CHAR_DOT 0x2Eu
    #if defined(CHAR_ZERO)
      #undef CHAR_ZERO
    #endif
    #define CHAR_ZERO 0x30u
    #if defined(CHAR_LOWERCASE_X)
      #undef CHAR_LOWERCASE_X
    #endif
    #define CHAR_LOWERCASE_X 0x78u
    #if defined(CHAR_BACKSPACE)
      #undef CHAR_BACKSPACE
    #endif
    #define CHAR_BACKSPACE 0x08u
    #if defined(CHAR_DEL)
      #undef CHAR_DEL
    #endif
    #define CHAR_DEL 0x7Fu
    #if defined(CHAR_PRINTABLE_MIN)
      #undef CHAR_PRINTABLE_MIN
    #endif
    #define CHAR_PRINTABLE_MIN 0x20u
    #if defined(CHAR_PRINTABLE_MAX)
      #undef CHAR_PRINTABLE_MAX
    #endif
    #define CHAR_PRINTABLE_MAX 0x7Eu
    #if defined(HEX_DIGIT_MASK)
      #undef HEX_DIGIT_MASK
    #endif
    #define HEX_DIGIT_MASK 0xFu
    #if defined(PATH_SEGMENTS_DIVISOR)
      #undef PATH_SEGMENTS_DIVISOR
    #endif
    #define PATH_SEGMENTS_DIVISOR 0x2u
    typedef struct ConsoleState_s {
      Base_t deviceReady;
      Base_t echoEnabled;
      Byte_t commandBuffer[CONFIG_CONSOLE_MAX_COMMAND_LENGTH];
      HalfWord_t bufferPosition;
      Byte_t currentWorkingDirectory[CONFIG_FS_MAX_PATH_LENGTH];
    } ConsoleState_t;
    typedef struct ConsoleCommand_s {
      const Byte_t *name;
      Return_t (*handler)(const Byte_t *);
      const Byte_t *description;
    } ConsoleCommand_t;
    #ifdef __cplusplus
      extern "C" {
    #endif
    Return_t xConsoleInit(void);
    void vConsoleTask(Task_t *task_, TaskParm_t *parm_);
    #if defined(POSIX_ARCH_OTHER)
      void __ConsoleStateClear__(void);
    #endif
    Size_t __strnlen__(const Byte_t *str_, const Size_t size_);
    Return_t __strcpy__(Byte_t *dest_, const Byte_t *src_, const Size_t size_);
    Return_t __strncpy__(Byte_t *dest_, const Byte_t *src_, const Size_t size_);
    Base_t __strncmp__(const Byte_t *s1_, const Byte_t *s2_, const Size_t size_);
    Return_t __strcat__(Byte_t *dest_, const Byte_t *src_, const Size_t size_);
    Byte_t * __strnchr__(const Byte_t *str_, const Byte_t ch_, const Size_t size_);
    Byte_t * __strnrchr__(const Byte_t *str_, const Byte_t ch_, const Size_t size_);
    Return_t __path_join__(Byte_t *dest_, const Byte_t *base_, const Byte_t *path_, Size_t destSize, Size_t baseSize, Size_t pathSize);
    Return_t __path_normalize__(Byte_t *path_, Size_t size_);
    Base_t __path_is_absolute__(const Byte_t *path_, Size_t size_);
    Return_t __path_dirname__(Byte_t *dest_, const Byte_t *path_, Size_t destSize, Size_t pathSize);
    Return_t __path_basename__(Byte_t *dest_, const Byte_t *path_, Size_t destSize, Size_t pathSize);
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
    #endif
  #endif
#endif