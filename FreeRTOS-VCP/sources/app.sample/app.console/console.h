// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : console.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_CONSOLE_HEADER
#define MCU_BSP_CONSOLE_HEADER

#if ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_UART value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH != 1 )

#define NUL_KEY                         (0x00UL)
#define BACK_SPACE                      (0x08UL)    // Back space
#define DEL_KEY                         (0x7FUL)    // Delete
#define ESC_KEY                         (0x1BUL)    // ESC
#define NAK_KEY                         (0x15UL)
#define SPACE_BAR                       (0x20UL)
#define LBRACKET                        (0x5BUL)
#define RBRACKET                        (0x5DUL)
#define UPPERCASE_A                     (0x41UL)    // ^ (up)
#define UPPERCASE_B                     (0x42UL)    // v (down)
#define UPPERCASE_C                     (0x43UL)    // > (right)
#define UPPERCASE_D                     (0x44UL)    // < (left)
#define ARRIAGE_RETURN                  (uint8)('\r')
#define LINE_FEED                       (uint8)('\n')  // Enter
#define CRLF                            ("\r\n")
#define PUTTY_BS                        (0x7FUL)    // Putty Back space


#define CSL_TASK_STK_SIZE               (512UL)

#define CSL_CMD_BUFF_SIZE               (128UL)
#define CSL_CMD_PROMPT_SIZE             (2UL)
#define CSL_CMD_NUM_MAX                 (38UL)
#define CSL_ARGUMENT_MAX                (10UL)
#define CSL_SPACE_1COUNT                (1UL)
#define CSL_HISTORY_COUNT               (10UL)

#define CSL_LOG_NUM_MAX                 (3UL)

#define CMD_ENABLE                      (1UL)
#define CMD_DISABLE                     (0UL)

#ifndef ON
#define ON                              (1UL)
#define OFF                             (0UL)
#endif

typedef void                            (*CmdLogFunc) (void *pArgv[]);
typedef struct ConsoleLogFunc
{
    CmdLogFunc                          clFunc;
} ConsoleLogFunc_t;

enum
{
    CSL_NOINPUT,
    CSL_INPUTING,
    CSL_EXECUTE
};

typedef void (CmdFuncType)(uint8 argc, void *argv[]);

// Console Command List
typedef struct ConsoleCmdList
{
    uint32                              clEnable;
    const uint8                         *clName;
    CmdFuncType                         *clFunc;

} ConsoleCmdList_t;

// Console History Control
typedef struct CSLHistroyControl
{
    uint8                               hcIndex;
    uint8                               hcFlag;
    uint32                              hcSize;
    uint8                               hcBuf[CSL_CMD_BUFF_SIZE];

} CSLHistroyControl_t;

// Console History Manager
typedef struct CSLHistoryManager
{
    uint32                              hmPtr;
    CSLHistroyControl_t                 hmList[CSL_HISTORY_COUNT];

} CSLHistoryManager_t;

/*
***************************************************************************************************
*                                         EXTERN FUNCTION
***************************************************************************************************
*/

extern void CreateConsoleTask
(
    void
);

#endif  // ( MCU_BSP_SUPPORT_APP_CONSOLE == 1 )

#endif  // MCU_BSP_CONSOLE_HEADER

