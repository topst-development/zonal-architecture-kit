// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gdma_ip_test.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_GDMA_IP_TEST_HEADER
#define MCU_BSP_GDMA_IP_TEST_HEADER

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#include <sal_internal.h>

//#define GDMA_IP_TEST_CODE

//============================================================================================================================
// Controller hardware access definitions
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// DMA Controller - Configuration Register
#define bwDMA_CTRLR_ENABLED         1   // DMA Ctrlr enabled flag
#define bwDMA_AHB1_ENDIAN           1   // AHB Bus 1 Endianness
#define bwDMA_AHB2_ENDIAN           1   // AHB Bus 2 Endianness

#define bsDMA_CTRLR_ENABLED         0
#define bsDMA_AHB1_ENDIAN           1
#define bsDMA_AHB2_ENDIAN           2

// DMA Controller - Protection Register
#define bwDMA_PROTECTED_USE         1   // Protected access bit

#define bsDMA_PROTECTED_USE         0

// DMA Controller - DMA Channel LLI Register
#define bwDMA_NEXT_LLI_BUS          1   // AHB Bus of next LLI
#define bwDMA_LLI_RESERVED          1   // Reserved (set to 0 on write)
#define bwDMA_NEXT_LLI              30  // Address of next LLI

#define bsDMA_NEXT_LLI_BUS          0
#define bsDMA_LLI_RESERVED          1
#define bsDMA_NEXT_LLI              2

// DMA Controller - DMA Channel Control Register
#define bwDMA_TRANSFER_SIZE         12  // Transfer size
#define bwDMA_SRC_BURST_SIZE        3   // Source Burst Size
#define bwDMA_DEST_BURST_SIZE       3   // Destination Burst Size
#define bwDMA_SRC_WIDTH             3   // Source Width
#define bwDMA_DEST_WIDTH            3   // Destination Width
#define bwDMA_SRC_BUS               1   // Source AHB Bus
#define bwDMA_DEST_BUS              1   // Destination AHB Bus
#define bwDMA_SRC_INCREMENT         1   // Source auto Increment
#define bwDMA_DEST_INCREMENT        1   // Destination auto Increment
#define bwDMA_PROTECTION            3   // Bus Protection Lines
#define bwDMA_TC_INTERRUPT_ENABLE   1   // TC Interrupt enable

#define bsDMA_TRANSFER_SIZE         0
#define bsDMA_SRC_BURST_SIZE        12
#define bsDMA_DEST_BURST_SIZE       15
#define bsDMA_SRC_WIDTH             18
#define bsDMA_DEST_WIDTH            21
#define bsDMA_SRC_BUS               24
#define bsDMA_DEST_BUS              25
#define bsDMA_SRC_INCREMENT         26
#define bsDMA_DEST_INCREMENT        27
#define bsDMA_PROTECTION            28
#define bsDMA_TC_INTERRUPT_ENABLE   31

// DMA Controller - DMA Channel Configuration Register
#define bwDMA_CHANNEL_ENABLED       1   // Channel Enable
#define bwDMA_SRC_PERIPHERAL        5   // Source peripheral Id (0-31)
#define bwDMA_DEST_PERIPHERAL       5   // Destination peripheral Id (0-31)
#define bwDMA_FLOW_CONTROL          3   // Flow Control
#define bwDMA_ERROR_INTERRUPT_MASK  1   // Error Interrupt Mask
#define bwDMA_TC_INTERRUPT_MASK     1   // Terminal Count Interrupt Mask
#define bwDMA_BUS_LOCK              1   // Bus Lock
#define bwDMA_ACTIVE                1   // FIFO Active
#define bwDMA_HALT                  1   // Halt
#define bwDMA_CONFIG_RESERVED       13  // Reserved

#define bsDMA_CHANNEL_ENABLED       0
#define bsDMA_SRC_PERIPHERAL        1
#define bsDMA_DEST_PERIPHERAL       6
#define bsDMA_FLOW_CONTROL          11
#define bsDMA_ERROR_INTERRUPT_MASK  14
#define bsDMA_TC_INTERRUPT_MASK     15
#define bsDMA_BUS_LOCK              16
#define bsDMA_ACTIVE                17
#define bsDMA_HALT                  18
#define bsDMA_CONFIG_RESERVED       19

#define DMA_MAX_CHANNELS            2
#define DMA_MAX_AHB_MASTERS         2




//============================================================================================================================
// Common type definitions
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Channel index
typedef enum dma_xCHANNEL_INDEX
{
    dma_CHANNEL_INDEX_0     = 0,    // Channel-0
    dma_CHANNEL_INDEX_1     = 1,    // Channel-1
    dma_CHANNEL_INDEX_2     = 2,    // Channel-2
    dma_CHANNEL_INDEX_3     = 3,    // Channel-3
    dma_CHANNEL_INDEX_4     = 4,    // Channel-4
    dma_CHANNEL_INDEX_5     = 5,    // Channel-5
    dma_CHANNEL_INDEX_6     = 6,    // Channel-6
    dma_CHANNEL_INDEX_7     = 7,    // Channel-7
    dma_CHANNEL_INDEX_ALL   = 8     // Channel-0~7
} dma_eCHANNEL_INDEX;

// Request index
typedef enum dma_xREQUEST_INDEX
{
    dma_REQUEST_INDEX_0     = 0,    // Channel-0
    dma_REQUEST_INDEX_1     = 1,    // Channel-1
    dma_REQUEST_INDEX_2     = 2,    // Channel-2
    dma_REQUEST_INDEX_3     = 3,    // Channel-3
    dma_REQUEST_INDEX_4     = 4,    // Channel-4
    dma_REQUEST_INDEX_5     = 5,    // Channel-5
    dma_REQUEST_INDEX_6     = 6,    // Channel-6
    dma_REQUEST_INDEX_7     = 7,    // Channel-7
    dma_REQUEST_INDEX_8     = 8,    // Channel-8
    dma_REQUEST_INDEX_9     = 9,    // Channel-9
    dma_REQUEST_INDEX_10    = 10,   // Channel-10
    dma_REQUEST_INDEX_11    = 11,   // Channel-11
    dma_REQUEST_INDEX_12    = 12,   // Channel-12
    dma_REQUEST_INDEX_13    = 13,   // Channel-13
    dma_REQUEST_INDEX_14    = 14,   // Channel-14
    dma_REQUEST_INDEX_15    = 15,   // Channel-15
    dma_REQUEST_INDEX_ALL   = 16    // Channel-0~15
} dma_eREQUEST_INDEX;

// AHB endianness
typedef enum dma_xAHB_ENDIAN
{
    dma_LITTLE_ENDIAN       = 0,    // Litle endian
    dma_BIG_ENDIAN          = 1     // Big   endian
} dma_eAHB_ENDIAN;


// Id used to refer to AHB bus
typedef enum dma_xAHB_BUS
{
    dma_AHB_BUS_1           = 0,    // AHB Bus 1
    dma_AHB_BUS_2           = 1     // AHB Bus 2
} dma_eAHB_BUS;

// Id used to refer to AHB bus
typedef enum dma_xEN
{
    dma_DISABLE             = 0,    // Disable
    dma_ENABLE              = 1     // Eanble
} dma_eEN;

// Width of the AHB bus(es)
typedef enum dma_xAHB_WIDTH
{
    dma_AHB_WIDTH_8_BIT     = 0,    //   32 Bits
    dma_AHB_WIDTH_16_BIT    = 1,    //   32 Bits
    dma_AHB_WIDTH_32_BIT    = 2     //   32 Bits
} dma_eAHB_WIDTH;

// Burst Transfer Size
typedef enum dma_xBURST_SIZE
{
    dma_BURST_1             = 0,    //   1 transfer  per burst
    dma_BURST_4             = 1,    //   4 transfers per burst
    dma_BURST_8             = 2,    //   8 transfers per burst
    dma_BURST_16            = 3,    //  16 transfers per burst
    dma_BURST_32            = 4,    //  32 transfers per burst
    dma_BURST_64            = 5,    //  64 transfers per burst
    dma_BURST_128           = 6     // 128 transfers per burst
} dma_eBURST_SIZE;

// Transfer Width
// : Do not specify a greater width than actual width of the AHB bus(es)
typedef enum dma_xWIDTH
{
    dma_WIDTH_8_BIT         = 0,    //    8 Bits per transfer
    dma_WIDTH_16_BIT        = 1,    //   16 Bits per transfer
    dma_WIDTH_32_BIT        = 2,    //   32 Bits per transfer
    dma_WIDTH_64_BIT        = 3,    //   64 Bits per transfer
    dma_WIDTH_128_BIT       = 4,    //  128 Bits per transfer
    dma_WIDTH_256_BIT       = 5,    //  256 Bits per transfer
    dma_WIDTH_512_BIT       = 6,    //  512 Bits per transfer
    dma_WIDTH_1024_BIT      = 7     // 1024 Bits per transfer
} dma_eWIDTH;

typedef enum dma_xSync
{
    dma_SYNC_ENABLE   = 0,
    dma_SYNC_DISABLE  = 1
} dma_eSync;

// Settings for AHB Bus Protection Lines
// : The desired protection should be created by binary ORing one entry from each pair.
// ex) dma_PROT_SUPER | dma_PROT_BUFFERABLE | dma_PROT_CACHEABLE
typedef enum dma_xPROTECTION_BITS
{
    dma_PROT_USER           = 0x00, // User access
    dma_PROT_SUPER          = 0x01, // Supervisor access

    dma_PROT_NON_BUFFERABLE = 0x00, // Non bufferable data
    dma_PROT_BUFFERABLE     = 0x02, // Bufferable data

    dma_PROT_NON_CACHEABLE  = 0x00, // Non cacheable data
    dma_PROT_CACHEABLE      = 0x04, // Cacheable data

    dma_PROTECTION_VALID    = ( dma_PROT_CACHEABLE  |
                                dma_PROT_BUFFERABLE |
                                dma_PROT_SUPER )
} dma_ePROTECTION_BITS;

// Enum of possible DMA flow directions & flow controllers
typedef enum dma_xFLOW_CONTROL
{
    dma_MEM_TO_MEM_DMA_CTRL                            = 0,
    dma_MEM_TO_PERIPHERAL_DMA_CTRL                     = 1,
    dma_PERIPHERAL_TO_MEM_DMA_CTRL                     = 2,
    dma_PERIPHERAL_TO_PERIPHERAL_DMA_CTRL              = 3,
    dma_PERIPHERAL_TO_PERIPHERAL_DEST_PERIPHERAL_CTRL  = 4,
    dma_MEM_TO_PERIPHERAL_PERIPHERAL_CTRL              = 5,
    dma_PERIPHERAL_TO_MEM_PERIPHERAL_CTRL              = 6,
    dma_PERIPHERAL_TO_PERIPHERAL_SRC_PERIPHERAL_CTRL   = 7

} dma_eFLOW_CONTROL;

// Interrupt status / clear, enabled channel register structure
typedef struct  {
    uint32    CH0         : 1;    // [00]
    uint32    CH1         : 1;    // [01]
    uint32    CH2         : 1;    // [02]
    uint32    CH3         : 1;    // [03]
    uint32    CH4         : 1;    // [04]
    uint32    CH5         : 1;    // [05]
    uint32    CH6         : 1;    // [06]
    uint32    CH7         : 1;    // [07]
    uint32    reserved0   : 24;   // [31:08]
} DMA_sCH;

typedef union {
    uint32        nReg;
    DMA_sCH     bReg;
} DMA_uCH;

// Software burst/single/last burst/last/single request, synchronization register structure
typedef struct  {
    uint32    REQ0        : 1;    // [00]
    uint32    REQ1        : 1;    // [01]
    uint32    REQ2        : 1;    // [02]
    uint32    REQ3        : 1;    // [03]
    uint32    REQ4        : 1;    // [04]
    uint32    REQ5        : 1;    // [05]
    uint32    REQ6        : 1;    // [06]
    uint32    REQ7        : 1;    // [07]
    uint32    REQ8        : 1;    // [08]
    uint32    REQ9        : 1;    // [09]
    uint32    REQ10       : 1;    // [10]
    uint32    REQ11       : 1;    // [11]
    uint32    REQ12       : 1;    // [12]
    uint32    REQ13       : 1;    // [13]
    uint32    REQ14       : 1;    // [14]
    uint32    REQ15       : 1;    // [15]
    uint32    reserved0   : 16;   // [31:16]
} DMA_sREQ;

typedef union {
    uint32        nReg;
    DMA_sREQ    bReg;
} DMA_uREQ;

// Configuration register structure
typedef struct  {
    uint32    CTRL_ENABLE     : bwDMA_CTRLR_ENABLED;
    uint32    AHB1_ENDIAN     : bwDMA_AHB1_ENDIAN;
    uint32    AHB2_ENDIAN     : bwDMA_AHB2_ENDIAN;
    uint32    reserved0       : (32-bwDMA_CTRLR_ENABLED
                                 -bwDMA_AHB1_ENDIAN
                                 -bwDMA_AHB2_ENDIAN
                              );
} DMA_sCFG;

typedef union {
    uint32        nReg;
    DMA_sCFG    bReg;
} DMA_uCFG;

// Channel linked list item register structure
typedef struct  {
    uint32    NEXT_LLI_BUS    : bwDMA_NEXT_LLI_BUS;
    uint32    reserved0       : bwDMA_LLI_RESERVED;
    uint32    NEXT_LLI        : bwDMA_NEXT_LLI;
} DMA_sCH_LLI;

typedef union {
    uint32        nReg;
    DMA_sCH_LLI bReg;
} DMA_uCH_LLI;

// Channel control register structure
typedef struct  {
    uint32    TRANSFER_SIZE       : bwDMA_TRANSFER_SIZE;
    uint32    SRC_BURST_SIZE      : bwDMA_SRC_BURST_SIZE;
    uint32    DEST_BURST_SIZE     : bwDMA_DEST_BURST_SIZE;
    uint32    SRC_WIDTH           : bwDMA_SRC_WIDTH;
    uint32    DEST_WIDTH          : bwDMA_DEST_WIDTH;
    uint32    SRC_BUS             : bwDMA_SRC_BUS;
    uint32    DEST_BUS            : bwDMA_DEST_BUS;
    uint32    SRC_INCREMENT       : bwDMA_SRC_INCREMENT;
    uint32    DEST_INCREMENT      : bwDMA_DEST_INCREMENT;
    uint32    PROTECTION          : bwDMA_PROTECTION;
    uint32    TC_INTERRUPT_ENABLE : bwDMA_TC_INTERRUPT_ENABLE;
} DMA_sCH_CTRL;

typedef union {
    uint32            nReg;
    DMA_sCH_CTRL    bReg;
} DMA_uCH_CTRL;

// Channel configuration register structure
typedef struct  {
    uint32    CHANNEL_ENABLED      : bwDMA_CHANNEL_ENABLED;
    uint32    SRC_PERIPHERAL       : bwDMA_SRC_PERIPHERAL;
    uint32    DEST_PERIPHERAL      : bwDMA_DEST_PERIPHERAL;
    uint32    FLOW_CONTROL         : bwDMA_FLOW_CONTROL;
    uint32    ERROR_INTERRUPT_MASK : bwDMA_ERROR_INTERRUPT_MASK;
    uint32    TC_INTERRUPT_MASK    : bwDMA_TC_INTERRUPT_MASK;
    uint32    BUS_LOCK             : bwDMA_BUS_LOCK;
    uint32    ACTIVE               : bwDMA_ACTIVE;
    uint32    HALT                 : bwDMA_HALT;
    uint32    reserved0            : bwDMA_CONFIG_RESERVED;
} DMA_sCH_CFG;

typedef union {
    uint32            nReg;
    DMA_sCH_CFG     bReg;
} DMA_uCH_CFG;

// DMA Controller - DMA Channel Registers - template structure
typedef volatile struct DMA_xCHANNEL
{
    uint32            SRC_ADDR;
    uint32            DEST_ADDR;
    DMA_uCH_LLI     LLI;
    DMA_uCH_CTRL    CTRL;
    DMA_uCH_CFG     CFG;
    uint32            Reserved0;
    uint32            Reserved1;
    uint32            Reserved2;

} DMA_sCHANNEL;

// Constants defining the size (in 32 bit words) of reserved spaces in the DMA controllers address space
#define DMA_NUM_RSRVD_WRDS_BEFORE_CHANNELS      ( (0x100 - 0x034) >> 2 )
#define DMA_NUM_RSRVD_WRDS_BEFORE_PERIPHERAL_ID ( (0xfe0 - 0x130) >> 2 )

// DMA Controller - Template structure of entire address space of a DMA controller
typedef volatile struct DMA_xPort
{
    DMA_uCH         IRQ_STATUS;                                             // 0x0000
    DMA_uCH         IRQ_ITC_STATUS;                                         // 0x0004
    DMA_uCH         IRQ_ITC_CLEAR;                                          // 0x0008
    DMA_uCH         IRQ_ERR_STATUS;                                         // 0x000c
    DMA_uCH         IRQ_ERR_CLEAR;                                          // 0x0010
    DMA_uCH         RAW_IRQ_ITC_STATUS;                                     // 0x0014
    DMA_uCH         RAW_IRQ_ERR_STATUS;                                     // 0x0018
    DMA_uCH         ENABLED_CHANNEL;                                        // 0x001c
    DMA_uREQ        SW_BURST_REQ;                                           // 0x0020
    DMA_uREQ        SW_SINGLE_REQ;                                          // 0x0024
    DMA_uREQ        SW_LAST_BURST_REQ;                                      // 0x0028
    DMA_uREQ        SW_LAST_SINGLE_REQ;                                     // 0x002c
    DMA_uCFG        CONFIG;                                                 // 0x0030
    //DMA_uREQ        SYNCH;                                                  // 0x0034

    uint32            Reserved0[DMA_NUM_RSRVD_WRDS_BEFORE_CHANNELS];          // 0x0034 ~ 0x00fc

    DMA_sCHANNEL    DMA_CHANNEL[2];                                         // 0x0100 ~ 0x0130

    uint32            Reserved1[DMA_NUM_RSRVD_WRDS_BEFORE_PERIPHERAL_ID];     // 0x0130 ~ 0x0fdc

    uint32            PERI_Id0;                                               // 0x0fe0
    uint32            PERI_Id1;                                               // 0x0fe4
    uint32            PERI_Id2;                                               // 0x0fe8
    uint32            PERI_Id3;                                               // 0x0fec
    uint32            CELL_Id0;                                               // 0x0ff0
    uint32            CELL_Id1;                                               // 0x0ff4
    uint32            CELL_Id2;                                               // 0x0ff8
    uint32            CELL_Id3;                                               // 0x0ffc
} DMA_sPort;

// DMA LLI format
typedef struct dma_xLLI_FORMAT
{
    uint32            SRC_ADDR;
    uint32            DEST_ADDR;
    DMA_uCH_LLI     LLI;
    DMA_uCH_CTRL    CTRL;
} dma_sLLI_FORMAT;

// Micom XIN frequency
#define MC_XIN_FREQ_MHZ     24
#define MC_XIN_FREQ_KHZ     24000
#define MC_XIN_FREQ_HZ      24000000
#define MC_PLL_MIN_P        2                   // 4MHz <= (Fin/P) <= 12MHz
#define MC_PLL_MAX_P        6

// Micom peripheral clock source select value(MC_PERI_CLK_CFG.CLK_SEL)
#define MC_CCLK_XIN         0
#define MC_CCLK_PLL0        1
#define MC_CCLK_PLL1        2
#define MC_CCLK_XIN_DIV     3
#define MC_CCLK_PLL0_DIV    4
#define MC_CCLK_PLL1_DIV    5
#define MC_CCLK_EXT_CLK0    6
#define MC_CCLK_EXT_CLK1    7

// Micom peripheral clock source select value(MC_PERI_CLK_CFG.CLK_SEL)
#define MC_PCLK_PLL0_FOUT   0
#define MC_PCLK_PLL1_FOUT   1
#define MC_PCLK_XIN         5
#define MC_PCLK_PLL0_DIV    10
#define MC_PCLK_PLL1_DIV    11
#define MC_PCLK_XIN_DIV     23

// Micom peripheral clock ID
#define MC_PCLK_SFMC         0
#define MC_PCLK_CAN0         1
#define MC_PCLK_CAN1         2
#define MC_PCLK_CAN2         3
#define MC_PCLK_GPSB0        4
#define MC_PCLK_GPSB1        5
#define MC_PCLK_GPSB2        6
#define MC_PCLK_GPSB3        7
#define MC_PCLK_GPSB4        8
#define MC_PCLK_GPSB5        9
#define MC_PCLK_UART0       10
#define MC_PCLK_UART1       11
#define MC_PCLK_UART2       12
#define MC_PCLK_UART3       13
#define MC_PCLK_UART4       14
#define MC_PCLK_UART5       15
#define MC_PCLK_I2CM0       16
#define MC_PCLK_I2CM1       17
#define MC_PCLK_I2CM2       18
#define MC_PCLK_PDM0        19
#define MC_PCLK_PDM1        20
#define MC_PCLK_PDM2        21
#define MC_PCLK_ICTC0       22
#define MC_PCLK_ICTC1       23
#define MC_PCLK_ICTC2       24
#define MC_PCLK_ADC         25
#define MC_PCLK_TIMER0      26
#define MC_PCLK_TIMER1      27
#define MC_PCLK_TIMER2      28
#define MC_PCLK_TIMER3      29
#define MC_PCLK_TIMER4      30
#define MC_PCLK_TIMER5      31

//----------------------------------------------------------------------------------------------------------------------------
// Macro
//----------------------------------------------------------------------------------------------------------------------------
#define MC_PLL_P(x)        (x <<  0)
#define MC_PLL_M(x)        (x <<  6)
#define MC_PLL_S(x)        (x << 16)
#define MC_PLL_PWRDN(x)    (x << 31)

//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
// Micom PLL configuration register
typedef struct {
    uint32            P               :  6;   // [05:00], RW
    uint32            M               : 10;   // [15:06], RW
    uint32            S               :  3;   // [18:16], RW
    uint32            FIN_SEL         :  2;   // [20:19], RW
    uint32            BYPASS          :  1;   // [   21], RW
    uint32                            :  1;   // [   22], Reserved
    uint32            LOCK            :  1;   // [   23], RO
    uint32            ICP             :  2;   // [25:24], RW
    uint32            LOCK_ENABLE     :  1;   // [   26], RW
    uint32            RSEL            :  4;   // [30:27], RW
    uint32            RST             :  1;   // [   31], RW
} MC_PLL_CFG;

typedef union {
    uint32            nReg;
    MC_PLL_CFG      bReg;
} MC_PLL_CFG_U;

// Micom PLL control register
typedef struct {
    uint32            AFC_ENB         :  1;   // [   00], RW
    uint32            EXT_AFC         :  5;   // [05:01], RW
    uint32            LOCK_CON_IN     :  2;   // [07:06], RW
    uint32            LOCK_CON_OUT    :  2;   // [09:08], RW
    uint32            LOCK_CON_DLY    :  2;   // [11:10], RW
    uint32            LOCK_CON_REV    :  2;   // [13:12], RW
    uint32                            : 18;   // [31:14], Reserved
} MC_PLL_CTRL;

typedef union {
    uint32            nReg;
    MC_PLL_CTRL     bReg;
} MC_PLL_CTRL_U;

// Micom PLL monitor register
typedef struct {
    uint32            AFC_CODE        :  5;   // [04:00], RO
    uint32                            :  3;   // [07:05], Reserved
    uint32            PBIAS_CTRL      :  1;   // [   08], RW
    uint32            PBIAS_CTRL_EN   :  1;   // [   09], RW
    uint32            VCO_BOOST       :  1;   // [   10], RW
    uint32            LRD_EN          :  1;   // [   11], RW
    uint32            FSEL            :  1;   // [   12], RW
    uint32            FEED_EN         :  1;   // [   13], RW
    uint32            FOUT_MASK       :  1;   // [   14], RW
    uint32            AFCINIT_SEL     :  1;   // [   15], RW
    uint32                            : 16;   // [31:16], Reserved
} MC_PLL_MON;

typedef union {
    uint32            nReg;
    MC_PLL_MON      bReg;
} MC_PLL_MON_U;

// Micom clock divider configuration register
typedef struct {
    uint32                            :  8;   // [07:00], Reserved
    uint32            XIN_DIV         :  6;   // [13:08], RW
    uint32            XIN_DIV_STS     :  1;   // [   14], RO
    uint32            XIN_DIV_EN      :  1;   // [   15], RW
    uint32            PLL1_DIV        :  6;   // [21:16], RW
    uint32            PLL1_DIV_STS    :  1;   // [   22], RO
    uint32            PLL1_DIV_EN     :  1;   // [   23], RW
    uint32            PLL0_DIV        :  6;   // [29:24], RW
    uint32            PLL0_DIV_STS    :  1;   // [   30], RO
    uint32            PLL0_DIV_EN     :  1;   // [   31], RW
} MC_CLK_DIV_CFG;

typedef union {
    uint32            nReg;
    MC_CLK_DIV_CFG  bReg;
} MC_CLK_DIV_CFG_U;

// Micom CPU clock configuration register
typedef struct {
    uint32            CLK_SEL         :  3;   // [02:00], RW
    uint32                            : 28;   // [30:03], Reserved
    uint32            CHG_REQ         :  1;   // [   31], RO
} MC_CPU_CLK_CFG;

typedef union {
    uint32            nReg;
    MC_CPU_CLK_CFG  bReg;
} MC_CPU_CLK_CFG_U;

// Micom bus clock configuration register
typedef struct {
    uint32            nSYSPORESET     :  1;   // [   00], RW
    uint32            nRESET          :  1;   // [   01], RW
    uint32            DBGRESETn       :  1;   // [   02], RW
    uint32            PRESETDBGn      :  1;   // [   03], RW
    uint32                            : 28;   // [31:04], Reserved
} MC_CPU_SW_RST;

typedef union {
    uint32            nReg;
    MC_CPU_SW_RST   bReg;
} MC_CPU_SW_RST_U;

typedef struct {
    uint32            CLK_DIV         : 12;   // [11:00], RW
    uint32                            : 12;   // [23:12], Reserved
    uint32            CLK_SEL         :  5;   // [28:24], RW
    uint32            DIV_EN          :  1;   // [   29], RW
    uint32            OUT_EN          :  1;   // [   30], RW
    uint32            OUT_EN_SYNC     :  1;   // [   31], R
} MC_PCLK_CFG;

typedef union {
    uint32            nReg;
    MC_PCLK_CFG     bReg;
} MC_PCLK_CFG_U;

typedef struct {
    uint32            AUTO_RST_EN     :  4;   // [03:00], RW
    uint32            reserved0       : 12;   // [15:04]
    uint32            MAX_CNT         :  8;   // [23:16], RW
    uint32            reserved1       :  8;   // [31:24]
} MC_CPU_RST_CFG;

typedef union {
    uint32            nReg;
    MC_CPU_RST_CFG  bReg;
} MC_CPU_RST_CFG_U;

// Micom configuration soft fault status
typedef struct {
    uint32    REG_PLL0_PMS            :  1;   // [0][   00], 0x0_0,   PLL-0 configuration-0
    uint32    REG_PLL0_CON            :  1;   // [0][   01], 0x0_4,   PLL-0 configuration-1
    uint32    REG_PLL0_MON            :  1;   // [0][   02], 0x0_8,   PLL-0 configuration-2
    uint32    REG_PLL1_PMS            :  1;   // [0][   03], 0x0_c,   PLL-1 configuration-0
    uint32    REG_PLL1_CON            :  1;   // [0][   04], 0x1_0,   PLL-1 configuration-1
    uint32    REG_PLL1_MON            :  1;   // [0][   05], 0x1_4,   PLL-1 configuration-2
    uint32    REG_DIV_CFG             :  1;   // [0][   06], 0x1_8,   XIN, PLL-0, PLL-1 divider configuration
    uint32    REG_CLK_CHG             :  1;   // [0][   07], 0x1_c,   Clock changer source configuration
    uint32    REG_CPU_SW_RST          :  1;   // [0][   08], 0x2_0,   CPU SW reset configuration
    uint32    REG_PCLK_SFMC           :  1;   // [0][   09], 0x2_4,   SFMC    peripheral clock configuration
    uint32    REG_PCLK_CAN0           :  1;   // [0][   10], 0x2_8,   CAN-0   peripheral clock configuration
    uint32    REG_PCLK_CAN1           :  1;   // [0][   11], 0x2_c,   CAN-1   peripheral clock configuration
    uint32    REG_PCLK_CAN2           :  1;   // [0][   12], 0x3_0,   CAN-2   peripheral clock configuration
    uint32    REG_PCLK_GPSB0          :  1;   // [0][   13], 0x3_4,   GPSB-0  peripheral clock configuration
    uint32    REG_PCLK_GPSB1          :  1;   // [0][   14], 0x3_8,   GPSB-1  peripheral clock configuration
    uint32    REG_PCLK_GPSB2          :  1;   // [0][   15], 0x3_c,   GPSB-2  peripheral clock configuration
    uint32    REG_PCLK_GPSB3          :  1;   // [0][   16], 0x4_0,   GPSB-3  peripheral clock configuration
    uint32    REG_PCLK_GPSB4          :  1;   // [0][   17], 0x4_4,   GPSB-4  peripheral clock configuration
    uint32    REG_PCLK_GPSB5          :  1;   // [0][   18], 0x4_8,   GPSB-5  peripheral clock configuration
    uint32    REG_PCLK_UART0          :  1;   // [0][   19], 0x4_c,   UART-0  peripheral clock configuration
    uint32    REG_PCLK_UART1          :  1;   // [0][   20], 0x5_0,   UART-1  peripheral clock configuration
    uint32    REG_PCLK_UART2          :  1;   // [0][   21], 0x5_4,   UART-2  peripheral clock configuration
    uint32    REG_PCLK_UART3          :  1;   // [0][   22], 0x5_8,   UART-3  peripheral clock configuration
    uint32    REG_PCLK_UART4          :  1;   // [0][   23], 0x5_c,   UART-4  peripheral clock configuration
    uint32    REG_PCLK_I2CM0          :  1;   // [0][   24], 0x6_0,   I2CM-0  peripheral clock configuration
    uint32    REG_PCLK_I2CM1          :  1;   // [0][   25], 0x6_4,   I2CM-1  peripheral clock configuration
    uint32    REG_PCLK_I2CM2          :  1;   // [0][   26], 0x6_8,   I2CM-2  peripheral clock configuration
    uint32    REG_PCLK_PDM0           :  1;   // [0][   27], 0x6_c,   PDM-0   peripheral clock configuration
    uint32    REG_PCLK_PDM1           :  1;   // [0][   28], 0x7_0,   PDM-1   peripheral clock configuration
    uint32    REG_PCLK_PDM2           :  1;   // [0][   29], 0x7_4,   PDM-2   peripheral clock configuration
    uint32    REG_PCLK_ICTC0          :  1;   // [0][   30], 0x7_8,   ICTC-0  peripheral clock configuration
    uint32    REG_PCLK_ICTC1          :  1;   // [0][   31], 0x7_c,   ICTC-1  peripheral clock configuration
    uint32    REG_PCLK_ICTC2          :  1;   // [1][   00], 0x8_0,   ICTC-2  peripheral clock configuration
    uint32    REG_PCLK_ADC            :  1;   // [1][   01], 0x8_4,   ADC     peripheral clock configuration
    uint32    REG_PCLK_TIMER0         :  1;   // [1][   02], 0x8_8,   Timer-0 peripheral clock configuration
    uint32    REG_PCLK_TIMER1         :  1;   // [1][   03], 0x8_c,   Timer-1 peripheral clock configuration
    uint32    REG_PCLK_TIMER2         :  1;   // [1][   04], 0x9_0,   Timer-2 peripheral clock configuration
    uint32    REG_PCLK_TIMER3         :  1;   // [1][   05], 0x9_4,   Timer-3 peripheral clock configuration
    uint32    REG_PCLK_TIMER4         :  1;   // [1][   06], 0x9_8,   Timer-4 peripheral clock configuration
    uint32    REG_PCLK_TIMER5         :  1;   // [1][   07], 0x9_c,   Timer-5 peripheral clock configuration
    uint32    REG_CPU_RST_CFG         :  1;   // [1][   08], 0xa_0,   CPU auto reset configuration
    uint32    REG_CCU_CFG_WR_PW       :  1;   // [1][   09], 0xa_4,   CCU configuration write password
    uint32    REG_CCU_CFG_WR_LOCK     :  1;   // [1][   10], 0xa_8,   CCU configuration write lock
    uint32                            : 21;   // [1][31:11]
} MC_CCU_SOFT_FAULT_EN;

typedef union {
    uint32                    nReg[2];
    MC_CCU_SOFT_FAULT_EN    bReg;
} MC_CCU_SOFT_FAULT_EN_U;

// Micom configuration soft control
typedef struct {
    uint32                SOFT_FAULT_EN       :  1;   // [   00]
    uint32                reserved0           :  3;   // [04:01]
    uint32                SOFT_FAULT_TEST_EN  :  1;   // [   05]
    uint32                reserved1           :  3;   // [07:06]
    uint32                SOFT_FAULT_REQ_INIT :  1;   // [   08]
    uint32                reserved2           :  7;   // [15:09]
    uint32                TIMEOUT_VALUE       : 12;   // [27:16]
    uint32                reserved3           :  4;   // [31:28]
} MC_CCU_SOFT_FAULT_CTRL;

typedef union {
    uint32                    nReg;
    MC_CCU_SOFT_FAULT_CTRL  bReg;
} MC_CCU_SOFT_FAULT_CTRL_U;

// Micom configuration soft control status
typedef struct {
    uint32                SOFT_FAULT_STS      :  1;   // [   00]
    uint32                reserved0           : 15;   // [15:01]
    uint32                SOFT_FAULT_REQ      :  1;   // [   16]
    uint32                reserved1           :  7;   // [23:17]
    uint32                SOFT_FAULT_ACK      :  1;   // [   24]
    uint32                reserved2           :  7;   // [31:25]
} MC_CCU_SOFT_FAULT_STS;

typedef union {
    uint32                    nReg;
    MC_CCU_SOFT_FAULT_STS   bReg;
} MC_CCU_SOFT_FAULT_STS_U;

//----------------------------------------------------------------------------------------------------------------------------
// Micom CCU(Clock Control Unit) registers
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_PLL_CFG_U                uPLL0_CFG;              // 0x00
    MC_PLL_CTRL_U               uPLL0_CTRL;             // 0x04
    MC_PLL_MON_U                uPLL0_MON;              // 0x08
    MC_PLL_CFG_U                uPLL1_CFG;              // 0x0c
    MC_PLL_CTRL_U               uPLL1_CTRL;             // 0x10
    MC_PLL_MON_U                uPLL1_MON;              // 0x14
    MC_CLK_DIV_CFG_U            uCLK_DIV_CFG;           // 0x18
    MC_CPU_CLK_CFG_U            uCPU_CLK_CFG;           // 0x1c
    MC_CPU_SW_RST_U             uCPU_SW_RST;            // 0x20
    MC_PCLK_CFG_U               uSFMC_PCLK_CFG;         // 0x24
    MC_PCLK_CFG_U               uCAN0_PCLK_CFG;         // 0x28
    MC_PCLK_CFG_U               uCAN1_PCLK_CFG;         // 0x2c
    MC_PCLK_CFG_U               uCAN2_PCLK_CFG;         // 0x30
    MC_PCLK_CFG_U               uGPSB0_PCLK_CFG;        // 0x34
    MC_PCLK_CFG_U               uGPSB1_PCLK_CFG;        // 0x38
    MC_PCLK_CFG_U               uGPSB2_PCLK_CFG;        // 0x3C
    MC_PCLK_CFG_U               uGPSB3_PCLK_CFG;        // 0x40
    MC_PCLK_CFG_U               uGPSB4_PCLK_CFG;        // 0x44
    MC_PCLK_CFG_U               uGPSB5_PCLK_CFG;        // 0x48
    MC_PCLK_CFG_U               uUART0_PCLK_CFG;        // 0x4c
    MC_PCLK_CFG_U               uUART1_PCLK_CFG;        // 0x50
    MC_PCLK_CFG_U               uUART2_PCLK_CFG;        // 0x54
    MC_PCLK_CFG_U               uUART3_PCLK_CFG;        // 0x58
    MC_PCLK_CFG_U               uUART4_PCLK_CFG;        // 0x5c
    MC_PCLK_CFG_U               uUART5_PCLK_CFG;        // 0x60
    MC_PCLK_CFG_U               uI2C0_PCLK_CFG;         // 0x64
    MC_PCLK_CFG_U               uI2C1_PCLK_CFG;         // 0x68
    MC_PCLK_CFG_U               uI2C2_PCLK_CFG;         // 0x6c
    MC_PCLK_CFG_U               uPDM0_PCLK_CFG;         // 0x70
    MC_PCLK_CFG_U               uPDM1_PCLK_CFG;         // 0x74
    MC_PCLK_CFG_U               uPDM2_PCLK_CFG;         // 0x78
    MC_PCLK_CFG_U               uICTC0_PCLK_CFG;        // 0x7c
    MC_PCLK_CFG_U               uICTC1_PCLK_CFG;        // 0x80
    MC_PCLK_CFG_U               uICTC2_PCLK_CFG;        // 0x84
    MC_PCLK_CFG_U               uADC_PCLK_CFG;          // 0x88
    MC_PCLK_CFG_U               uTIMER0_PCLK_CFG;       // 0x8c
    MC_PCLK_CFG_U               uTIMER1_PCLK_CFG;       // 0x90
    MC_PCLK_CFG_U               uTIMER2_PCLK_CFG;       // 0x94
    MC_PCLK_CFG_U               uTIMER3_PCLK_CFG;       // 0x98
    MC_PCLK_CFG_U               uTIMER4_PCLK_CFG;       // 0x9c
    MC_PCLK_CFG_U               uTIMER5_PCLK_CFG;       // 0xa0
    MC_CPU_RST_CFG_U            uCPU_RST_CFG;           // 0xa4
    uint32                        CFG_WR_PW;              // 0xa8
    uint32                        CFG_WR_LOCK;            // 0xac
    uint32                        reserved0[8];           // 0xb0 ~ 0xcc
    MC_CCU_SOFT_FAULT_EN_U      uSOFT_FAULT_EN;         // 0xd0 ~ 0xd4
    uint32                        reserved1[2];           // 0xd8 ~ 0xdc
    MC_CCU_SOFT_FAULT_EN_U      uSOFT_FAULT_STS;        // 0xe0 ~ 0xe4
    uint32                        reserved2[2];           // 0xe8 ~ 0xec
    MC_CCU_SOFT_FAULT_CTRL_U    uSOFT_FAULT_CTRL;       // 0xf0
    MC_CCU_SOFT_FAULT_STS_U     uSOFT_FAULT_CTRL_STS;   // 0xf4
} MC_CCU;


/*
    Import MC_CFG
*/
// Micom bus clock/rst mask-0~2
typedef struct {
    uint32                NSAID_FILTER        :  1;   // [0][   00], RW
    uint32                NSAID_FILTER_BUS    :  1;   // [0][   01], RW
    uint32                NSAID_FILTER_CFG    :  1;   // [0][   02], RW
    uint32                SGID_FILTER         :  1;   // [0][   03], RW
    uint32                SGID_FILTER_BUS     :  1;   // [0][   04], RW
    uint32                SGID_FILTER_CFG     :  1;   // [0][   05], RW
    uint32                SFMC_WRAP           :  1;   // [0][   06], RW
    uint32                SFMC_MID_FILTER_BUS :  1;   // [0][   07], RW
    uint32                SFMC_MID_FILTER_CFG :  1;   // [0][   08], RW
    uint32                SFMC_X2H            :  1;   // [0][   09], RW
    uint32                SFMC_CORE_BUS       :  1;   // [0][   10], RW
    uint32                IMC_WRAP            :  1;   // [0][   11], RW
    uint32                IMC_MID_FILTER_BUS  :  1;   // [0][   12], RW
    uint32                IMC_MID_FILTER_CFG  :  1;   // [0][   13], RW
    uint32                IMC_CORE_BUS        :  1;   // [0][   14], RW
    uint32                GIC400              :  1;   // [0][   15], RW
    uint32                GIC400_H2X          :  1;   // [0][   16], RW
    uint32                GPIO                :  1;   // [0][   17], RW
    uint32                SOC400              :  1;   // [0][   18], RW
    uint32                JTAG_DBG_IF         :  1;   // [0][   19], RW
    uint32                DMA                 :  1;   // [0][   20], RW
    uint32                CAN_WRAP            :  1;   // [0][   21], RW
    uint32                CAN0                :  1;   // [0][   22], RW
    uint32                CAN1                :  1;   // [0][   23], RW
    uint32                CAN2                :  1;   // [0][   24], RW
    uint32                CAN_CFG             :  1;   // [0][   25], RW
    uint32                                    :  6;   // [0][31:26], RW
    uint32                UART_WRAP           :  1;   // [1][   00], RW
    uint32                UART0               :  1;   // [1][   01], RW
    uint32                UART1               :  1;   // [1][   02], RW
    uint32                UART2               :  1;   // [1][   03], RW
    uint32                UART3               :  1;   // [1][   04], RW
    uint32                UART4               :  1;   // [1][   05], RW
    uint32                UART5               :  1;   // [1][   06], RW
    uint32                UART_DMA0           :  1;   // [1][   07], RW
    uint32                UART_DMA1           :  1;   // [1][   08], RW
    uint32                UART_DMA2           :  1;   // [1][   09], RW
    uint32                UART_DMA3           :  1;   // [1][   10], RW
    uint32                UART_DMA4           :  1;   // [1][   11], RW
    uint32                UART_DMA5           :  1;   // [1][   12], RW
    uint32                UART_CFG            :  1;   // [1][   13], RW
    uint32                I2C_WRAP            :  1;   // [1][   14], RW
    uint32                I2C0                :  1;   // [1][   15], RW
    uint32                I2C1                :  1;   // [1][   16], RW
    uint32                I2C2                :  1;   // [1][   17], RW
    uint32                I2C_CFG             :  1;   // [1][   18], RW
    uint32                PDM_WRAP            :  1;   // [1][   19], RW
    uint32                PDM0                :  1;   // [1][   20], RW
    uint32                PDM1                :  1;   // [1][   21], RW
    uint32                PDM2                :  1;   // [1][   22], RW
    uint32                PDM_CFG             :  1;   // [1][   23], RW
    uint32                ICTC_WRAP           :  1;   // [1][   24], RW
    uint32                ICTC0               :  1;   // [1][   25], RW
    uint32                ICTC1               :  1;   // [1][   26], RW
    uint32                ICTC2               :  1;   // [1][   27], RW
    uint32                ADC_WRAP            :  1;   // [1][   28], RW
    uint32                MBOX_WRAP           :  1;   // [1][   29], RW
    uint32                                    :  2;   // [1][31:30], RW
    uint32                TIMER_WRAP          :  1;   // [2][   00], RW
    uint32                TIMER0              :  1;   // [2][   01], RW
    uint32                TIMER1              :  1;   // [2][   02], RW
    uint32                TIMER2              :  1;   // [2][   03], RW
    uint32                TIMER3              :  1;   // [2][   04], RW
    uint32                TIMER4              :  1;   // [2][   05], RW
    uint32                TIMER5              :  1;   // [2][   06], RW
    uint32                WATCHDOG            :  1;   // [2][   07], RW
    uint32                X2X_MST             :  1;   // [2][   08], RW
    uint32                X2X_SLV             :  1;   // [2][   09], RW
    uint32                GPSB_WRAP           :  1;   // [2][   10], RW
    uint32                GPSB0               :  1;   // [2][   11], RW
    uint32                GPSB1               :  1;   // [2][   12], RW
    uint32                GPSB2               :  1;   // [2][   13], RW
    uint32                GPSB3               :  1;   // [2][   14], RW
    uint32                GPSB4               :  1;   // [2][   15], RW
    uint32                GPSB5               :  1;   // [2][   16], RW
    uint32                GPSB_CFG            :  1;   // [2][   17], RW
    uint32                GPSB_IO_MON         :  1;   // [2][   18], RW
    uint32                CMU                 :  1;   // [2][   19], RW
    uint32                SYS_CTRL_SM         :  1;   // [2][   20], RW
    uint32                                    : 11;   // [2][31:21], RW
} MC_BCLK_RST_MASK;

typedef union {
    uint32                nReg[3];
    MC_BCLK_RST_MASK    bReg;
} MC_BCLK_RST_MASK_U;

// Micom IMC configuration
typedef struct {
    uint32                RAM_RDW             :  1;   // [   00], RW
    uint32                ROM_RDW             :  1;   // [   01], RW
    uint32                                    : 30;   // [31:02], RW
} MC_IMC_CFG;

typedef union {
    uint32                nReg;
    MC_IMC_CFG          bReg;
} MC_IMC_CFG_U;

// Micom DMA single request configuration
typedef struct {
    uint32                ICTC0               :  1;   // [   00], RW
    uint32                ICTC1               :  1;   // [   01], RW
    uint32                ICTC2               :  1;   // [   02], RW
    uint32                ADC                 :  1;   // [   03], RW
    uint32                CAN0                :  1;   // [   04], RW
    uint32                CAN1                :  1;   // [   05], RW
    uint32                CAN2                :  1;   // [   06], RW
    uint32                                    : 25;   // [31:07], RW
} MC_DMA_REQ_CFG;

typedef union {
    uint32                nReg;
    MC_DMA_REQ_CFG      bReg;
} MC_DMA_REQ_CFG_U;

typedef struct {
    uint32                                    :  8;   // [0][07:00], RW
    uint32                                    :  8;   // [0][15:08], RW
    uint32                                    :  8;   // [0][23:16], RW
    uint32                DMA_REQ7_SEL        :  8;   // [0][31:24], RW
    uint32                DMA_REQ8_SEL        :  8;   // [1][07:00], RW
    uint32                DMA_REQ9_SEL        :  8;   // [1][15:08], RW
    uint32                DMA_REQ10_SEL       :  8;   // [1][23:16], RW
    uint32                DMA_REQ11_SEL       :  8;   // [1][31:24], RW
    uint32                DMA_REQ12_SEL       :  8;   // [2][07:00], RW
    uint32                DMA_REQ13_SEL       :  8;   // [2][15:08], RW
    uint32                DMA_REQ14_SEL       :  8;   // [2][23:16], RW
    uint32                DMA_REQ15_SEL       :  8;   // [2][31:24], RW
} MC_DMA_REQ_SEL;

typedef union {
    uint32                nReg[3];
    MC_DMA_REQ_SEL      bReg;
} MC_DMA_REQ_SEL_U;

// Micom asynchronous bridge power control
typedef struct {
    uint32                MC2MB_PWRDNREQN     :  1;   // [   00], RW
    uint32                MC2MB_PWRDNACKN     :  1;   // [   01], RO
    uint32                MC2MB_CACTIVES      :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                MB2MC_PWRDNREQN     :  1;   // [   04], RW
    uint32                MB2MC_PWRDNACKN     :  1;   // [   05], RO
    uint32                MB2MC_CACTIVEM      :  1;   // [   06], RO
    uint32                                    :  9;   // [15:07], Reserved
    uint32                MC2MB_MST_RST       :  1;   // [   16], RW
    uint32                MC2MB_SLV_RST       :  1;   // [   17], RO
    uint32                                    :  2;   // [19:18], Reserved
    uint32                MB2MC_SLV_RST       :  1;   // [   20], RW
    uint32                MB2MC_MST_RST       :  1;   // [   21], RO
    uint32                                    : 10;   // [31:22], Reserved
} MC_X2X_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_X2X_PWR_CTRL     bReg;
} MC_X2X_PWR_CTRL_U;

// Micom external access filter power control
typedef struct {
    uint32                NSAID_FIL_PWRDNREQN :  1;   // [   00], RW
    uint32                NSAID_FIL_PWRDNACKN :  1;   // [   01], RO
    uint32                NSAID_FIL_CACTIVE   :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                SGID_FIL_PWRDNREQN  :  1;   // [   04], RW
    uint32                SGID_FIL_PWRDNACKN  :  1;   // [   05], RO
    uint32                SGID_FIL_CACTIVE    :  1;   // [   06], RO
    uint32                                    :  1;   // [   07], Reserved
    uint32                NSAID_CFG_PWRDNREQN :  1;   // [   08], RW
    uint32                NSAID_CFG_PWRDNACKN :  1;   // [   09], RO
    uint32                NSAID_CFG_CACTIVE   :  1;   // [   10], RO
    uint32                                    :  1;   // [   11], Reserved
    uint32                SGID_CFG_PWRDNREQN  :  1;   // [   12], RW
    uint32                SGID_CFG_PWRDNACKN  :  1;   // [   13], RO
    uint32                SGID_CFG_CACTIVE    :  1;   // [   14], RO
    uint32                                    :  1;   // [   15], Reserved
    uint32                NSAID_FIL_FPID      :  1;   // [   16], RW
    uint32                                    :  3;   // [19:17], Reserved
    uint32                SGID_FIL_FPID       :  1;   // [   20], RW
    uint32                                    : 11;   // [31:21], RW
} MC_AID_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_AID_PWR_CTRL     bReg;
} MC_AID_PWR_CTRL_U;

// Micom internal access filter power control
typedef struct {
    uint32                SFMC_FIL_PWRDNREQN  :  1;   // [   00], RW
    uint32                SFMC_FIL_PWRDNACKN  :  1;   // [   01], RO
    uint32                SFMC_FIL_CACTIVE    :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                IMC_FIL_PWRDNREQN   :  1;   // [   04], RW
    uint32                IMC_FIL_PWRDNACKN   :  1;   // [   05], RO
    uint32                IMC_FIL_CACTIVE     :  1;   // [   06], RO
    uint32                                    :  1;   // [   07], Reserved
    uint32                SFMC_CFG_PWRDNREQN  :  1;   // [   08], RW
    uint32                SFMC_CFG_PWRDNACKN  :  1;   // [   09], RO
    uint32                SFMC_CFG_CACTIVE    :  1;   // [   10], RO
    uint32                                    :  1;   // [   11], Reserved
    uint32                IMC_CFG_PWRDNREQN   :  1;   // [   12], RW
    uint32                IMC_CFG_PWRDNACKN   :  1;   // [   13], RO
    uint32                IMC_CFG_CACTIVE     :  1;   // [   14], RO
    uint32                                    :  1;   // [   15], Reserved
    uint32                SFMC_FIL_FPID       :  1;   // [   16], RW
    uint32                                    :  3;   // [19:17], Reserved
    uint32                IMC_FIL_FPID        :  1;   // [   20], RW
    uint32                                    : 11;   // [31:21], RW
} MC_MID_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_MID_PWR_CTRL     bReg;
} MC_MID_PWR_CTRL_U;

// Micom debug control
typedef struct {
    uint32                CR5_DBGEN           :  1;   // [   00], RW
    uint32                CR5_NIDEN           :  1;   // [   01], RW
    uint32                                    :  2;   // [03:02], Reserved
    uint32                SOC400_DBGEN        :  1;   // [   04], RW
    uint32                SOC400_NIDEN        :  1;   // [   05], RW
    uint32                SOC400_SPIDEN       :  1;   // [   06], RW
    uint32                                    :  9;   // [15:07], Reserved
    uint32                GIC_CFG_DIS         :  1;   // [   16], RW
    uint32                GIC_DFT_RST_DIS     :  1;   // [   17], RW
    uint32                                    :  6;   // [23:18], Reserved
    uint32                DP_APB_SEL          :  1;   // [   24], RW
    uint32                                    :  7;   // [31:25], Reserved
} MC_DBG_DP_CTRL;

typedef union {
    uint32                nReg;
    MC_DBG_DP_CTRL      bReg;
} MC_DBG_DP_CTRL_U;

// Micom system user mode write disable
typedef struct {
    uint32                NIC400_GPV          :  1;   // [0][   00], RW
    uint32                GIG400              :  1;   // [0][   01], RW
    uint32                SMFC                :  1;   // [0][   02], RW
    uint32                CMU                 :  1;   // [0][   03], RW
    uint32                SYS_SM_CTRL         :  1;   // [0][   04], RW
    uint32                MC_CFG              :  1;   // [0][   05], RW
    uint32                CCU                 :  1;   // [0][   06], RW
    uint32                CR5_CFG             :  1;   // [0][   07], RW
    uint32                TIMER0              :  1;   // [0][   08], RW
    uint32                TIMER1              :  1;   // [0][   09], RW
    uint32                TIMER2              :  1;   // [0][   10], RW
    uint32                TIMER3              :  1;   // [0][   11], RW
    uint32                TIMER4              :  1;   // [0][   12], RW
    uint32                TIMER5              :  1;   // [0][   13], RW
    uint32                WDT                 :  1;   // [0][   14], RW
    uint32                GPIO                :  1;   // [0][   15], RW
    uint32                PMU                 :  1;   // [0][   16], RW
    uint32                FMU                 :  1;   // [0][   17], RW
    uint32                PMIO                :  1;   // [0][   18], RW
    uint32                DEF_SLV_IRQ         :  1;   // [0][   19], RW
    uint32                MID_CFG             :  1;   // [0][   20], RW
    uint32                SFMC_MID_FILTER     :  1;   // [0][   21], RW
    uint32                IMC_MID_FILTER      :  1;   // [0][   22], RW
    uint32                SGID_FILTER         :  1;   // [0][   23], RW
    uint32                NSAID_FILTER        :  1;   // [0][   24], RW
    uint32                LVDS_DEMUX          :  1;   // [0][   25], RW
    uint32                MIPI_WRAP           :  1;   // [0][   26], RW
    uint32                DP_WRAP             :  1;   // [0][   27], RW
    uint32                                    :  4;   // [0][31:28]
    uint32                S_MBOX0             :  1;   // [1][   00], RW
    uint32                NS_MBOX0            :  1;   // [1][   01], RW
    uint32                S_MBOX1             :  1;   // [1][   02], RW
    uint32                NS_MBOX1            :  1;   // [1][   03], RW
    uint32                MBOX2               :  1;   // [1][   04], RW
    uint32                                    : 27;   // [1][31:05]
} MC_SYS_USER_WR_DIS;

typedef union {
    uint32                nReg[2];
    MC_SYS_USER_WR_DIS  bReg;
} MC_SYS_USER_WR_DIS_U;

// Micom configuration soft fault status
typedef struct {
    uint32                REG_HCLK_MASK0          :  1;   // [   00], RW, 0x0_0, HCLK clock mask-0
    uint32                REG_HCLK_MASK1          :  1;   // [   01], RW, 0x0_4, HCLK clock mask-1
    uint32                REG_HCLK_MASK2          :  1;   // [   02], RW, 0x0_8, HCLK clock mask-2
    uint32                REG_SW_HRESET0          :  1;   // [   03], RW, 0x0_c, SW HRESETn mask-0
    uint32                REG_SW_HRESET1          :  1;   // [   04], RW, 0x1_0, SW HRESETn mask-1
    uint32                REG_SW_HRESET2          :  1;   // [   05], RW, 0x1_4, SW HRESETn mask-2
    uint32                REG_IMC_CFG             :  1;   // [   06], RW, 0x1_8, Internal Memory controller config.
    uint32                REG_DMA_SREQ_EN         :  1;   // [   07], RW, 0x1_c, DMA single request enable
    uint32                REG_DMA_REQ_SEL0        :  1;   // [   08], RW, 0x2_0, DMA request select-0
    uint32                REG_DMA_REQ_SEL1        :  1;   // [   09], RW, 0x2_4, DMA request select-1
    uint32                REG_DMA_REQ_SEL2        :  1;   // [   10], RW, 0x2_8, DMA request select-2
    uint32                REG_X2X_PWR_CTRL        :  1;   // [   11], RW, 0x2_c, Axi to Axi Async. Bridge(MICOM/MEMBUS to MEMBUS/MICOM) power management signal
    uint32                REG_AID_PWR_CTRL        :  1;   // [   12], RW, 0x3_0, AID(NSAID, SGID) filter power management signal
    uint32                REG_MID_PWR_CTRL        :  1;   // [   13], RW, 0x3_4, MID( SFMC, IMC ) filter power management signal
    uint32                REG_DEBUG_DP_CTRL       :  1;   // [   14], RW, 0x3_8, DBG_EN/NIDEN/SPIDEN & DP APB control select
    uint32                REG_SYS_USER_WR_DIS0    :  1;   // [   15], RW, 0x3_c, System block user mode write disabla-0
    uint32                REG_SYS_USER_WR_DIS1    :  1;   // [   16], RW, 0x4_0, System block user mode write disable-1
    uint32                REG_MICOM_READY         :  1;   // [   17], RW, 0x4_4, MICOM ready
    uint32                REG_MC_CFG_WR_PW        :  1;   // [   18], RW, 0x4_8, Micom configuration write password
    uint32                REG_MC_CFG_WR_LOCK      :  1;   // [   19], RW, 0x4_c, Micom configuration write lock
    uint32                                        : 12;   // [31:20], Reserved
} MC_CFG_SOFT_FAULT_EN;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_EN    bReg;
} MC_CFG_SOFT_FAULT_EN_U;

// Micom configuration soft control
typedef struct {
    uint32                SOFT_FAULT_EN       :  1;   // [   00]
    uint32                                    :  3;   // [04:01]
    uint32                SOFT_FAULT_TEST_EN  :  1;   // [   05]
    uint32                                    :  3;   // [07:06]
    uint32                SOFT_FAULT_REQ_INIT :  1;   // [   08]
    uint32                                    :  7;   // [15:09]
    uint32                TIMEOUT_VALUE       : 12;   // [27:16]
    uint32                                    :  4;   // [31:28]
} MC_CFG_SOFT_FAULT_CTRL;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_CTRL  bReg;
} MC_CFG_SOFT_FAULT_CTRL_U;

// Micom configuration soft control status
typedef struct {
    uint32                SOFT_FAULT_STS      :  1;   // [   00]
    uint32                                    : 15;   // [15:01]
    uint32                SOFT_FAULT_REQ      :  1;   // [   16]
    uint32                                    :  7;   // [23:17]
    uint32                SOFT_FAULT_ACK      :  1;   // [   24]
    uint32                                    :  7;   // [31:25]
} MC_CFG_SOFT_FAULT_STS;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_STS   bReg;
} MC_CFG_SOFT_FAULT_STS_U;

//----------------------------------------------------------------------------------------------------------------------------
// Micom configuration register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_BCLK_RST_MASK_U          uBCLK_MASK;             // 0x00 ~ 0x08
    MC_BCLK_RST_MASK_U          uSW_RST_MASK;           // 0x0c ~ 0x14
    MC_IMC_CFG_U                uIMC_CFG;               // 0x18
    MC_DMA_REQ_CFG_U            uDMA_REQ_CFG;           // 0x1c
    MC_DMA_REQ_SEL_U            uDMA_REQ_SEL;           // 0x20 ~ 0x28
    MC_X2X_PWR_CTRL_U           uX2X_PWR_CTRL;          // 0x2c
    MC_AID_PWR_CTRL_U           uAID_PWR_CTRL;          // 0x30
    MC_MID_PWR_CTRL_U           uMID_PWR_CTRL;          // 0x34
    MC_DBG_DP_CTRL_U            uDBG_CTRL;           // 0x38
    MC_SYS_USER_WR_DIS_U        uCFG_USER_WR_DIS;       // 0x3c ~ 0x40
    uint32                        MICOM_READY;            // 0x44
    uint32                        CFG_WR_PW;              // 0x48
    uint32                        CFG_WR_LOCK;            // 0x4c
    MC_CFG_SOFT_FAULT_EN_U      uSOFT_FAULT_EN;         // 0x50
    uint32                        reserved1[3];           // 0x54 ~ 0x5c
    MC_CFG_SOFT_FAULT_EN_U      uSOFT_FAULT_STS;        // 0x60
    uint32                        reserved2[3];           // 0x64 ~ 0x6c
    MC_CFG_SOFT_FAULT_CTRL_U    uSOFT_FAULT_CTRL;       // 0x70
    MC_CFG_SOFT_FAULT_STS_U     uSOFT_FAULT_CTRL_STS;   // 0x74
} MC_CFG;

/*API */
//void GDMA_IPTEST_IsrHandler (void * param);
//void GDMA_IPTEST_SetChannelEnable(uint32 control, uint32 ch) ;
//void GDMA_IPTEST_M2MNormalTransferTest(uint32 test_control,
//                            uint32    ch,
//                            uint32    src_addr,
//                            uint32    src_incr,
//                            uint32    dst_addr,
//                            uint32    dst_incr,
//                            uint32    trans_size);

//void GDMA_IPTEST_WaitIrqDone(void);
sint32 GDMA_IPTest(uint32 test_case);

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#endif  // MCU_BSP_GDMA_IP_TEST_HEADER

