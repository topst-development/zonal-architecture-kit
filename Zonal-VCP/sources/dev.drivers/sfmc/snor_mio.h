// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : snor_mio.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef MCU_BSP_SNOR_MIO_HEADER
#define MCU_BSP_SNOR_MIO_HEADER

#if ( MCU_BSP_SUPPORT_DRIVER_SFMC == 1 )

#define NOR_FLASH_BASE_ADDR(DRVIDX)         ((0x40000000u) + (((DRVIDX) * 0x8000000u)))
#define SFMC_BASE_ADDR(DRVIDX)              ((0xA0F00000u) + (((DRVIDX) * 0x8000u)))

#define SFMC_REG_MAGIC(DRVIDX)              ((SFMC_BASE_ADDR(DRVIDX))+(0x0000u))
#define SFMC_REG_VERSION(DRVIDX)            ((SFMC_BASE_ADDR(DRVIDX))+(0x0004u))
#define SFMC_REG_PARAM(DRVIDX)              ((SFMC_BASE_ADDR(DRVIDX))+(0x0008u))
#define SFMC_REG_STATUS(DRVIDX)             ((SFMC_BASE_ADDR(DRVIDX))+(0x000Cu))
#define SFMC_REG_RUN(DRVIDX)                ((SFMC_BASE_ADDR(DRVIDX))+(0x0010u))
#define SFMC_REG_INT_PEND(DRVIDX)           ((SFMC_BASE_ADDR(DRVIDX))+(0x0014u))
#define SFMC_REG_INT_ENB(DRVIDX)            ((SFMC_BASE_ADDR(DRVIDX))+(0x0018u))
#define SFMC_REG_BADDR_MANU(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x001Cu))
#define SFMC_REG_BADDR_AUTO(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x0020u))
#define SFMC_REG_MODE(DRVIDX)               ((SFMC_BASE_ADDR(DRVIDX))+(0x0024u))
#define SFMC_REG_TIMING(DRVIDX)             ((SFMC_BASE_ADDR(DRVIDX))+(0x0028u))
#define SFMC_REG_DELAY_SO(DRVIDX)           ((SFMC_BASE_ADDR(DRVIDX))+(0x002Cu))
#define SFMC_REG_DELAY_CLK(DRVIDX)          ((SFMC_BASE_ADDR(DRVIDX))+(0x0030u))
#define SFMC_REG_DELAY_WBD0(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x0034u))
#define SFMC_REG_DELAY_WBD1(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x0038u))
#define SFMC_REG_DELAY_RBD0(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x003Cu))
#define SFMC_REG_DELAY_RBD1(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x0040u))
#define SFMC_REG_DELAY_WOEBD0(DRVIDX)       ((SFMC_BASE_ADDR(DRVIDX))+(0x0044u))
#define SFMC_REG_DELAY_WOEBD1(DRVIDX)       ((SFMC_BASE_ADDR(DRVIDX))+(0x0048u))
#define SFMC_REG_DELAY_TIMEOUT(DRVIDX)      ((SFMC_BASE_ADDR(DRVIDX))+(0x004Cu))
#define SFMC_REG_CODE_TABLE(DRVIDX)         ((SFMC_BASE_ADDR(DRVIDX))+(0x0800u))

#define SFMC_REG_CODE_VERINFO(DRVIDX)       ((SFMC_BASE_ADDR(DRVIDX))+(0x09E0u))
#define SFMC_REG_CODE_VERINFO_SIZE          (0x1Fu)


#define SFMC_REG_RUN_MAN_IDLE               ( 0x0u )
#define SFMC_REG_RUN_MAN_RUN                ( 0x1u )
#define SFMC_REG_RUN_MAN_STOP               ( 0x3u )

#define SFMC_REG_RUN_AUTO_IDLE              ( 0x0u)
#define SFMC_REG_RUN_AUTO_RUN               ( 0x10u)
#define SFMC_REG_RUN_AUTO_STOP              ( 0x30u)

#define SFMC_REG_RUN_SOFT_RESET             ( 0x100u)//0x1u << 8u)

#define SFMC_REG_INT_ENB_USER_0             (   1u << 0u)
#define SFMC_REG_INT_ENB_USER_1             (   1u << 1u)
#define SFMC_REG_INT_ENB_USER_2             (   1u << 2u)
#define SFMC_REG_INT_ENB_USER_3             (   1u << 3u)

#define SFMC_REG_INT_ENB_CORE_ERR_EN        (   1u << 4u)
#define SFMC_REG_INT_ENB_TIMEOUT_EN         (   1u << 5u)

#define SFMC_REG_MODE_FLASH_RESET           (   1u )
#define SFMC_REG_MODE_MODE_DQUAD            (   1u << 1u) // Dual Quad mode enable (Active High)
#define SFMC_REG_MODE_FIFO_CTRL_EN          (   1u << 2u) // Enable DQS Clock
#define SFMC_REG_MODE_FIFO_CTRL_DIS         (   0u)
#define SFMC_REG_MODE_SERIAL_0              ( 0x1u << 4u) // Serial Mode 0
#define SFMC_REG_MODE_SERIAL_3              ( 0x2u << 4u) // Serial Mode 3
#define SFMC_REG_MODE_CROSS_WR              (   1u << 6u) // Write Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_CROSS_RD              (   1u << 7u) // Read Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_SIO_OFF_QUAD          ( 0xF0u<< 8u) // Quad Memory
#define SFMC_REG_MODE_SIO_OFF_OCTA          ( 0x00u<< 8u) // Octa or Dual Quad Memory

#define SFMC_REG_TIMING_SEL_P_180           (   1u << 0u) // Select rd_p_180 data
#define SFMC_REG_TIMING_SEL_N_180           (   1u << 1u) // Select rd_n_180 data
#define SFMC_REG_TIMING_SEL_PN              (   1u << 2u) // Secect p data (must 1 at DTR mode)
#define SFMC_REG_TIMING_SEL_PN_DT(x)        ( (x) << 2u) // Secect p data (must 1 at DTR mode)

#define SFMC_REG_TIMING_SEL_DQS_FCLK        (0x0u << 4u) // fclk
#define SFMC_REG_TIMING_SEL_DQS_FCLK_PAD    (0x1u << 4u) // fclk pad input
#define SFMC_REG_TIMING_SEL_DQS_I_DQS_OCTA  (0x2u << 4u) // i_dqs octa only
#define SFMC_REG_TIMING_SEL_DQS_N_FCLK      (0x3u << 4u) // ~fclk
#define SFMC_REG_TIMING_SEL_DQS(x)          ((x) << 4u) // ~fclk

#define SFMC_REG_TIMING_READ_LATENCY(x)     (((x) << 8u) & 0x00000F00u)
#define SFMC_REG_TIMING_CS_TO_CS(x)         (((x) << 16u) & 0x000F0000u)
#define SFMC_REG_TIMING_SC_EXTND(x)         (((x) << 20u) & 0x00300000u)

#define SFMC_REG_DELAY_SO_SLDH(x)           (((x) << 0u) & 0x000000FFu)   // CS Low to SO Latency (0 ~7)
#define SFMC_REG_DELAY_SO_SLCH(x)           (((x) << 8u) & 0x00000300u)   // CS Low to SCLK Latency (0 ~2)
#define SFMC_REG_DELAY_SO_INV_SCLK(x)       ((x) << 10u) // Invert SCLK


#define SFMC_REG_DELAY_CLK_WD(x)            (((x) <<  0u) & 0x000000FFu)  //SCLK Clock Delay (0xE7) [7:6] Delay ctrl, [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_BUF(x)        (((x) <<  0u) & 0x0000003Fu)  //SCLK Clock Delay [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_CTRL(x)       (((x) <<  6u) & 0x000000C0u)  //SCLK Clock Delay [7:6] Delay ctrl


#define SFMC_REG_DELAY_CLK_RD(x)            (((x) <<  8u) & 0x0000FF00u)  //Read Clock Delay  [15:14] Delay ctrl, [13:8] clk buf sel

#define SFMC_REG_DELAY_CLK_RD_BUF(x)        (((x) <<  8u) & 0x00003F00u)  //Read Clock Delay [15:14] clk_buf sel
#define SFMC_REG_DELAY_CLK_RD_CTRL(x)       (((x) << 14u) & 0x0000C000u)  //Read Clock Delay [13:8] Delay ctrl

#define SFMC_REG_DELAY_CLK_CSN_BD(x)        (((x) << 16u) & 0x001F0000u)  //CSN Signal Delay
#define SFMC_REG_DELAY_CLK_WR_TAB           (   1u << 21u) // CLK_WD tab_delay chg
#define SFMC_REG_DELAY_CLK_RD_TAB           (   1u << 22u) // CLK_RD tab_delay chg


#define SFMC_REG_DELAY_WBD0_SO_0(x)         (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[0] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_1(x)         (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[1] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_2(x)         (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[2] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_3(x)         (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[3] Write bit delay

#define SFMC_REG_DELAY_WBD1_SO_4(x)         (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[4] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_5(x)         (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[5] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_6(x)         (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[6] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_7(x)         (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[7] Write bit delay


#define SFMC_REG_DELAY_RBD0_SO_0(x)         (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[0] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_1(x)         (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[1] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_2(x)         (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[2] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_3(x)         (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[3] Read bit delay

#define SFMC_REG_DELAY_RBD1_SO_4(x)         (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[4] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_5(x)         (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[5] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_6(x)         (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[6] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_7(x)         (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[7] Read bit delay


#define SFMC_REG_DELAY_WOEBD0_SO_0(x)       (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[0] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_1(x)       (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[1] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_2(x)       (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[2] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_3(x)       (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[3] Write Output Enable bit delay

#define SFMC_REG_DELAY_WOEBD1_SO_4(x)       (((x) <<  0u) & 0x000000FFu)  // Flash Memory SO[4] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_5(x)       (((x) <<  8u) & 0x0000FF00u)  // Flash Memory SO[5] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_6(x)       (((x) << 16u) & 0x00FF0000u)  // Flash Memory SO[6] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_7(x)       (((x) << 24u) & 0xFF000000u)  // Flash Memory SO[7] Write Output Enable bit delay


#define SFMC_REG_DELAY_TIMEOUT_CNT(x)       (((x) << 0u) & 0x0000FFFFu)   // Timeout Counter [15:0]
#define SFMC_REG_DELAY_TIMEOUT_ENB          (   1u << 16u)                // Timeout Enable


#define CMD0_READ               0x0u
#define CMD0_WRITE              0x40000000u //0x1u
#define CMD0_WRITE_BYTE         0x80000000u //0x2u
#define CMD0_OPERATION          0xC0000000u //0x3u

#define CMD1_OPERATION          0x0u
#define CMD1_COMPARE            0x1u
#define CMD1_BR_INT             0x2u
#define CMD1_WAIT_STOP          0x3u

#define CMD2_BRANCH             0x0u
#define CMD2_INT                0x1u
#define CMD2_WAIT               0x0u
#define CMD2_STOP               0x1u

#define LA_KEEP_CS              0x0u
#define LA_DEASSERT_CS          0x20000000u //0x1u

#define D_DTR_DISABLE           0x0u // Double Transfer Rate(DTR) mode disable
#define D_DTR_ENABLE            0x10000000u //0x1u

#define AR_TABLE_WRITE          0x0u //0x0u   << (27)
#define AR_AHB_WRITE            0x08000000u //0x0u   << (27)

#define AA_TABLE_ADDR           0x0u
#define AA_AHB_ADDR             0x08000000u //0x1u

#define DU_REAL_DATA            0x0u
#define DU_DUMMY_DATA           0x04000000u //0x1u << 26

#define IO_NUM_NC               0xFu
#define IO_NUM_SINGLE           0x0u
#define IO_NUM_DUAL             0x01000000u //0x1u  << 24
#define IO_NUM_QUAD             0x02000000u //0x2u  << 24
#define IO_NUM_OCTA             0x03000000u //0x3u  << 24

#define MK_WRITE_CMD(CODE, DTR, DATA, SIZE, LA, IO_NUM) {\
                (CODE) = ((CMD0_WRITE_BYTE)  \
                        | (LA)  \
                        | (DTR)  \
                        | ((uint32)(SIZE)             << (26u))  \
                        | (IO_NUM)  \
                        | ((uint32)(DATA)             <<  (0u)));\
}

#define MK_WRITE_ADDR_AHB(CODE, DTR, IO_NUM) {      \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_KEEP_CS)  \
                        | (DTR)  \
                        | (AA_AHB_ADDR)  \
                        | ((DU_REAL_DATA))  \
                        | (IO_NUM)  \
                        | (0u));\
}

#define MK_WRITE_ADDR_AHB_3B(CODE, DTR, IO_NUM) {       \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_KEEP_CS)  \
                        | (DTR)  \
                        | (AA_AHB_ADDR)  \
                        | (DU_REAL_DATA)  \
                        | (IO_NUM)  \
                        | (1u));\
}

#define MK_WRITE_ADDR_TABLE(CODE, ADDR, SIZE, IO_NUM)   {   \
                (CODE) = ((CMD0_WRITE_BYTE)  \
                        | (LA_KEEP_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (DU_DUMMY_DATA)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)  << (12u))  \
                        | (uint32)(ADDR));\
}

#define MK_WRITE_ADDR(CODE, ADDR, SIZE, IO_NUM) {   \
                (CODE) = ((CMD0_WRITE_BYTE)  \
                        | (LA_DEASSERT_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (DU_DUMMY_DATA)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u))  \
                        | (uint32)(ADDR));\
}

#define MK_WRITE_DUMMY_CYCLE(CODE, DTR, SIZE, IO_NUM) { \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_KEEP_CS)  \
                        | (DTR)  \
                        | (AA_TABLE_ADDR)  \
                        | (DU_DUMMY_DATA)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u)));\
}

#define MK_READ_DATA_AHB(CODE, DTR, IO_NUM) {           \
                (CODE) = ((CMD0_READ)  \
                        | (LA_DEASSERT_CS)  \
                        | (DTR)  \
                        | (AR_AHB_WRITE)  \
                        | (IO_NUM)  \
                        | (0u));\
}

#define MK_READ_DATA(CODE, SIZE, ADDR, IO_NUM)  {   \
                (CODE) = ((CMD0_READ)  \
                        | (LA_DEASSERT_CS)  \
                        | (D_DTR_DISABLE )  \
                        | (AR_TABLE_WRITE) \
                        | (IO_NUM) \
                        | ((uint32)(SIZE)   << (12u)) \
                        | ((uint32)(ADDR)));\
}

#define MK_WRITE_DATA_AHB(CODE, IO_NUM) {           \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_DEASSERT_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_AHB_ADDR)  \
                        | (IO_NUM)  \
                        | (0u));\
}

#define MK_WRITE_DATA(CODE, SIZE, ADDR, IO_NUM) {   \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_DEASSERT_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u))  \
                        | (ADDR));\
}

#define MK_WRITE_DATA_3B(CODE, SIZE, ADDR, IO_NUM)  {   \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_DEASSERT_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u))  \
                        | ((uint32)(ADDR)   << (1u))  \
                        | (1u);\
}


#define MK_WRITE_PP_ADDR(CODE, SIZE, ADDR, IO_NUM)  {   \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_KEEP_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u))  \
                        | (uint32)(ADDR));\
}

#define MK_WRITE_PP_ADDR_3B(CODE, SIZE, ADDR, IO_NUM)   {   \
                (CODE) = ((CMD0_WRITE)  \
                        | (LA_KEEP_CS)  \
                        | (D_DTR_DISABLE)  \
                        | (AA_TABLE_ADDR)  \
                        | (IO_NUM)  \
                        | ((uint32)(SIZE)   << (12u))  \
                        | ((uint32)(ADDR)   << (1u))  \
                        | (1u));\
}


#define MK_WAIT_CMD(CODE,DELAY) {                   \
                (CODE) = ((0xF0000000u)  \
                        | (uint32)(DELAY));\
}

#ifdef NEVER
#define MK_WAIT_CMD(CODE,DELAY) {                   \
                (CODE) = ((CMD0_OPERATION)  \
                        | (CMD1_WAIT_STOP   << 28)  \
                        | (CMD2_WAIT        << 26)  \
                        | (DELAY            <<  0));\
}
#endif /* NEVER */

#define MK_STOP_CMD(CODE) {                          \
                    (CODE) = 0xF4000000u;             \
    }

#ifdef NEVER
#define MK_STOP_CMD(CODE) {                             \
                (CODE) = ((CMD0_OPERATION)  \
                        | ((CMD1_WAIT_STOP )  << (28))  \
                        | ((CMD2_STOP      )  << (26))  \
                        | ((0              )  << ( 0)));\
}
#endif /* NEVER */

#define SET_CODE_TABLE(DRVIDX, OFFSET, VAL)         \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + (OFFSET<<2)) = VAL)

#define GET_CODE_TABLE(DRVIDX, OFFSET)      \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + (OFFSET<<2)))

#define GET_CODE_TABLE_ADDR(DRVIDX, OFFSET)         \
    (SFMC_REG_CODE_TABLE(DRVIDX) + (OFFSET<<2));    \


#define SET_CMD_AUTO_ADDR(DRVIDX, OFFSET)       \
    (*(volatile uint32 *)(SFMC_REG_BADDR_AUTO(DRVIDX)) = ((0x800)+(OFFSET<<2)));    \


#define SET_CMD_MANU_ADDR(DRVIDX, OFFSET)       \
    (*(volatile uint32 *)(SFMC_REG_BADDR_MANU(DRVIDX)) = ((0x800)+(OFFSET<<2)));    \

#define SET_CMD_RUN(DRVIDX, VAL)        \
    (*(volatile uint32 *)(SFMC_REG_RUN(DRVIDX)) = VAL); \

#define WAIT_CMD_COMPLETE(DRVIDX) {     \
    while(1) {  \
        if((*(volatile uint32 *)(SFMC_REG_RUN(DRVIDX)) & 0xF)== 0)  \
            break;  \
    };  \
}

#define SET_CODE_TABLE_ERASE_ADDR(DRVIDX, OFFSET, ADDR)         \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + ((OFFSET+1)<<2)) &= ~(0x00FFFFFF)); \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + ((OFFSET+1)<<2)) |= (ADDR&0x00FFFFFF))

#define SET_CODE_TABLE_WRITE_ADDR(DRVIDX, OFFSET, ADDR)         \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + ((OFFSET+1)<<2)) &= ~(0x00FFFFFF)); \
    (*(volatile uint32 *)(SFMC_REG_CODE_TABLE(DRVIDX) + ((OFFSET+1)<<2)) |= (ADDR&0x00FFFFFF))


//==============================================
// Training Parameter Define
//==============================================
#define SFMC_SEL_DQS_NUM            4   // 0 ~ 3
#define SFMC_SEL_PN_NUM             2   // 0 ~ 1
#define SFMC_CS_EXTEND_NUM          4   // 0 ~ 3
#define SFMC_READ_LATENCY_NUM       8   // 0 ~ 7
#define SFMC_INV_SCLK_NUM           2   // 0, 1
#define SFMC_SLCH_NUM               3   // 0 ~ 2
#define SFMC_SLDH_NUM               2   // 0 ~ 1
#define SFMC_CLK_WD_CTRL_NUM        4   // 0 ~ 3
#define SFMC_CLK_WD_BUF_NUM         40  // 0 ~ 39
#define SFMC_CLK_RD_CTRL_NUM        4   // 0 ~ 3
#define SFMC_CLK_RD_BUF_NUM         40  // 0 ~ 39


typedef struct  {
    uint32  offset;
    uint32  size;
} code_table_info_t;

typedef struct SNOR_MIO_PRODUCT_INFO {
    int8    *name;
    uint8   ManufID;
    uint16  DevID;
    uint32  TotalSector;
    uint16  cmd_read;
    uint16  cmd_read_fast;
    uint16  cmd_write;
    uint16  flags;
} snor_product_info_t;

#define SFMC_BUF_SIZE       256u

typedef struct SNOR_MIO_DRV {
    const int8  *name;
    uint8       ManufID;
    uint16      DevID;
    uint8       shift;
    uint16      flags;
    uint32      current_io_mode;
    uint16      max_read_io;

    uint32      size;
    uint32      page_size;
    uint32      sector_size;
    uint32      sector_count;
    uint32      erase_size;

    uint16      cmd_read;
    uint16      cmd_read_fast;
    uint16      cmd_write;
    uint16      erase_cmd;

    uint32      dt_mode;

    code_table_info_t   sfmc_buf;
    code_table_info_t   sfmc_addr;
    code_table_info_t   rdid;
    code_table_info_t   rdsr;
    code_table_info_t   wrsr;
    code_table_info_t   rdcr;
    code_table_info_t   wrcr;
    code_table_info_t   en4b;
    code_table_info_t   ex4b;
    code_table_info_t   ear_mode;

    //code_table_info_t en_m_io;
    //code_table_info_t ex_m_io;
    //code_table_info_t config_dummy;
    code_table_info_t   write_enable;
    code_table_info_t   write_disable;
    code_table_info_t   read;
    code_table_info_t   read_fast;
    code_table_info_t   write;
    code_table_info_t   blk_erase;
    code_table_info_t   sec_erase;
    code_table_info_t   chip_erase;

    code_table_info_t   rdcr1;
    code_table_info_t   rdcr2;
    code_table_info_t   rdsr1;
    code_table_info_t   rdsr2;
    code_table_info_t   wrsr1;
    code_table_info_t   wrsr2;
    code_table_info_t   individual_lock;
    code_table_info_t   individual_unlock;

    code_table_info_t   rdab;
    code_table_info_t   wrab;

    uint32      iSFMC_REG_TIMING;
    uint32      iSFMC_REG_DELAY_SO;
    uint32      iSFMC_REG_DELAY_CLK;
    uint8       uiDataBuff[0x1000];
} snor_mio_drv_t;

//---------------------------------------------
// Erase commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_ERASE_4K                0x20u
#define CMD_ERASE_4K_4B             0x21u
#define CMD_ERASE_32K               0x52u
#define CMD_ERASE_CHIP              0xc7u
#define CMD_ERASE_64K               0xd8u
#define CMD_ERASE_64K_4B            0xDCu

//---------------------------------------------
// Write commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_PP                      0x02u    // page program
#define CMD_4PP                     0x38u    // Quad page program

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_PP4B                    0x12u    // page program 4b
#define CMD_4PP4B                   0x3Eu    // Quad page program 4b
#define CMD_8PP4B                   0x12EDu  // Octa Page program 4b

//---------------------------------------------

#define CMD_WRITE_STATUS            0x01u

#define CMD_WRITE_DISABLE           0x04u
#define CMD_READ_STATUS             0x05u
#define CMD_READ_CONFIG             0x15u

#define CMD_QUAD_PAGE_PROGRAM       0x32u
#define CMD_READ_STATUS1            0x35u
#define CMD_WRITE_ENABLE            0x06u
//#define CMD_READ_CONFIG               0x35
#define CMD_FLAG_STATUS             0x70u

#define CMD_WRITE_CONFIG2_OPI       0x728Du
#define CMD_WRITE_CONFIG2           0x72u

#define CONFIG2_SPI_MODE            0x00u
#define CONFIG2_STR_OPI_MODE        0x01u
#define CONFIG2_DTR_OPI_MODE        0x02u



//---------------------------------------------
// Read commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_READ                0x03u    // Normal read
#define CMD_FAST_READ           0x0Bu    // Fast read
#define CMD_FASTDTRD            0x0Du    // Fast DT read

#define CMD_DREAD               0x3Bu    // Dual read
#define CMD_QREAD               0x6Bu    // Quad read

#define CMD_4READ               0xEBu    // Quad STR read SPI/QPI

#define CMD_4DTRD               0xEDu   // Quad DT read

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_READ4B              0x13u
#define CMD_FAST_READ4B         0x0Cu
#define CMD_FASTDTRD_4B         0x0Eu

#define CMD_DREAD4B             0x3Cu
#define CMD_QREAD4B             0x6Cu

#define CMD_4READ4B             0xECu    // Quad STR read SPI/QPI

#define CMD_4DTRD4B             0xEEu    // Quad DT read


#define CMD_8READ               0xEC13u
#define CMD_8DTRD               0xEE11u

//---------------------------------------------

#define CMD_READ_ID                 0x9fu

#define CMD_EAR                     0xC5u

#define SNOR_SERIAL_MODE_0          (1 << 0)
#define SNOR_SERIAL_MODE_3          (1 << 1)

#define SNOR_PROTOCOL_SINGLE        (1 << 0)
#define SNOR_PROTOCOL_DUAL          (1 << 1)
#define SNOR_PROTOCOL_QUAD          (1 << 2)
#define SNOR_PROTOCOL_OCTA          (1 << 3)

#define SNOR_SECTOR_SIZE            0x1000u
#define SNOR_PAGE_SIZE              0x100u

#ifndef Hw0
#define Hw31    0x80000000u
#define Hw30    0x40000000u
#define Hw29    0x20000000u
#define Hw28    0x10000000u
#define Hw27    0x08000000u
#define Hw26    0x04000000u
#define Hw25    0x02000000u
#define Hw24    0x01000000u
#define Hw23    0x00800000u
#define Hw22    0x00400000u
#define Hw21    0x00200000u
#define Hw20    0x00100000u
#define Hw19    0x00080000u
#define Hw18    0x00040000u
#define Hw17    0x00020000u
#define Hw16    0x00010000u
#define Hw15    0x00008000u
#define Hw14    0x00004000u
#define Hw13    0x00002000u
#define Hw12    0x00001000u
#define Hw11    0x00000800u
#define Hw10    0x00000400u
#define Hw9     0x00000200u
#define Hw8     0x00000100u
#define Hw7     0x00000080u
#define Hw6     0x00000040u
#define Hw5     0x00000020u
#define Hw4     0x00000010u
#define Hw3     0x00000008u
#define Hw2     0x00000004u
#define Hw1     0x00000002u
#define Hw0     0x00000001u
#endif

#ifndef ISZERO
#define ISZERO(X, MASK)     ( !(((uint32)(X)) & ((uint32)(MASK))) )
#endif
/* sf param flags */
enum {
    SECT_4K     = 1 << 0,
    SECT_32K    = 1 << 1,
    E_FSR       = 1 << 2,
    SST_WR      = 1 << 3,
    WR_QPP      = 1 << 4,
    ADDR_4B     = 1 << 5,
};


enum {
    SFMC0    = 0,
    SFMC1    = 1,
};


sint32 SNOR_MIO_Init(uint32 uiDrvIdx);
void SNOR_MIO_Erase(uint32 uiDrvIdx, uint32 address, uint32 size);
sint32 SNOR_MIO_Write(uint32 uiDrvIdx, uint32 address, const uint8 *pBuffer, uint32 length);
sint32 SNOR_MIO_Read(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 length);
sint32 SNOR_MIO_Read_Fast(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 length);
#if 0
sint32 SNOR_MIO_WriteSector(uint32 lba_addr, const void *pBuffer, uint32 nSector);
sint32 SNOR_MIO_ReadSector(unsigned long ulLBA_addr, unsigned long ulSector, void *buff);
#endif
sint32 SNOR_MIO_FWDN_LowFormat(uint32 uiDrvIdx);
sint32 SNOR_MIO_FWDN_Read(uint32 uiDrvIdx, uint32 address, uint32 length, uint8 *buff);
sint32 SNOR_MIO_FWDN_Write(uint32 uiDrvIdx, uint32 address, uint32 length, const uint8 *buf);

void SNOR_MIO_AutoRun(uint32 uiDrvIdx, uint32 uiFlag, uint32 uiFast);

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SFMC == 1 )

#endif  // MCU_BSP_SNOR_MIO_HEADER

