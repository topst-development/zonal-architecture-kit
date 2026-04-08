// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : uart_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description : Demo Application for console function
*
*
***************************************************************************************************
*/

#ifndef  MCU_BSP_UART_TEST_HEADER
#define  MCU_BSP_UART_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#if ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )
    #error MCU_BSP_SUPPORT_DRIVER_UART value must be 1.
#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART != 1 )

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define UART_FALSE                      (0UL)
#define UART_TRUE                       (1UL)

#define UART_BASE_ADDR                  (0xA0200000UL)

#define UART_WriteReg(p, a, v)          SAL_WriteReg(v, UART_GET_BASE(p) + (a))
#define UART_ReadReg(p, a)              SAL_ReadReg(UART_GET_BASE(p) + (a))

// UART Register Offset
#define OFFSET_DR                       (0x00UL)        // Data register
#define OFFSET_ECR                      (0x04)          // Error Clear Register
#define OFFSET_CR                       (0x30UL)        // Control register
#define OFFSET_IBRD                     (0x24UL)        // Integer Baud rate register
#define OFFSET_FBRD                     (0x28UL)        // Fractional Baud rate register
#define OFFSET_LCRH                     (0x2cUL)        // Line Control register
#define OFFSET_IMSC                     (0x38UL)        // Interrupt Mask Set/Clear Register

// UART Control Register Bitset
#define CR_RTSEn                        (1UL << 14UL)   // RTS hardware flow control enable
#define CR_RXE                          (1UL << 9UL)    // Receive enable
#define CR_TXE                          (1UL << 8UL)    // Transmit enable
#define CR_LBE                          (1UL << 7UL)    // Loopback enable
#define CR_UARTEN                       (1UL << 0UL)    // UART enable

/*
***************************************************************************************************
*                                           FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                       UART_ConsoleTxTest
*
* @param:  ucTxCh    : UART channel to send Tx data
*          uiPortCfg : Uart PortCfg value
*          pucBuf    : Input data buffer
*          len       : Input data length
* @return: sint32
*
* Notes
*
***************************************************************************************************
*/
sint32 UART_ConsoleTxTest
(
    uint8                               ucTxCh,
    uint8                               uiPortCfg,
    const uint8 *                       pucBuf,
    uint32                              len
);

/*
***************************************************************************************************
*                                       UART_SelectTestCase
*
* @param    test case number
* @return
*
* Notes
*
***************************************************************************************************
*/
void UART_SelectTestCase
(
    uint32 uiTestCase
);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_UART == 1 )

#endif  // MCU_BSP_UART_TEST_HEADER

