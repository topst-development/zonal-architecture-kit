// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : snor_mio.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_SFMC == 1 )


#include <gpio.h>
#include <sal_internal.h>
#include "snor_mio.h"

#if (DEBUG_ENABLE)
#include "debug.h"

#define SNOR_MIO_DEBUG(fmt, args...)    {LOGD(DBG_TAG_SFMC, fmt, ## args)}//mcu_printf(fmt, ## args)
#define SNOR_MIO_CT_DEBUG(fmt, args...) {LOGD(DBG_TAG_SFMC, fmt, ## args)}//mcu_printf(fmt, ## args)
#define SNOR_MIO_TRACE(fmt, args...)    {LOGD(DBG_TAG_SFMC, fmt, ## args)}//{LOGD(DBG_TAG_SFMC, fmt, ## args)}
#else
#define SNOR_MIO_DEBUG(fmt, args...)
#define SNOR_MIO_CT_DEBUG(fmt, args...)
#define SNOR_MIO_TRACE(fmt, args...)
#endif

static inline uint32 SNOR_READREG(uint32 uiAddr);
static inline void SNOR_WRITEREG(uint32 uiValue, uint32 uiAddr);
static inline void SNOR_cmd_manual_run(uint32 uiDrvIdx, const code_table_info_t * pCmdInfo);
static uint32 SNOR_set_codetable(uint32 uiDrvIdx, const code_table_info_t * pCmdInfo, const uint32 * pCode);


static sint32 SNOR_MIO_ISSI_DefaultConfig(uint32 uiDrvIdx);
static sint32 SNOR_MIO_MICRON_DefaultConfig(uint32 uiDrvIdx);
static sint32 SNOR_MIO_MXIC_DefaultConfig(uint32 uiDrvIdx);
static sint32 SNOR_MIO_WINBOND_DefaultConfig(uint32 uiDrvIdx);
static sint32 SNOR_MIO_CYPRESS_DefaultConfig(uint32 uiDrvIdx);
static sint32 SNOR_MIO_XMC_DefaultConfig(uint32 uiDrvIdx);
static void SNOR_MIO_ReadID(uint32 uiDrvIdx, uint8 *pMID, unsigned short *pDevID);
static uint32 SNOR_MIO_Make_cmd_readid(uint32 uiDrvIdx);
static snor_mio_drv_t snor_mio_drv[2];

/************************************************************************************************************

                                        Serial Nor Flash Product Table

************************************************************************************************************/
static snor_product_info_t gSNOR_MIO_ProductTable[] =
{
    /*
    |Seral Flash                |MID    |DID    |Sector |Read CMD           |F_Read         |Write CMD  |Flag           |
    |NAME                       |       |       |Count  |                   |               |           |               |
    */
    {"MXIC-MX25L1633E",         0xC2,   0x2415, 32,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MXIC, 2MB
    {"MXIC-MX25L3233F",         0xC2,   0x2016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MXIC, 4MB
    {"MXIC-MX25L6435E",         0xC2,   0x2017, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MXIC, 8MB
    {"MXIC-MX25L12835F",        0xC2,   0x2018, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MXIC, 16MB
    {"MXIC-MX25L25645G",        0xC2,   0x2019, 512,    CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MXIC, 32MB
    {"MXIC-MX25L51245G",        0xC2,   0x201A, 1024,   CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MXIC, 64MB
    {"MXIC-MX25LM51245G",       0xC2,   0x853A, 1024,   CMD_FAST_READ4B,    CMD_8READ,      CMD_PP4B,   ADDR_4B|SECT_4K },  // MXIC, 64MB

    {"WINBOND-W25X20CL",        0xEF,   0x3012, 4,      CMD_FAST_READ,      CMD_DREAD,      CMD_PP,     SECT_4K         },  // WINBOND, 256Kbyte
    {"WINBOND-W25X40CL",        0xEF,   0x3013, 8,      CMD_FAST_READ,      CMD_DREAD,      CMD_PP,     SECT_4K         },  // WINBOND, 512Kbyte
    {"WINBOND-W25Q80JV",        0xEF,   0x4014, 16,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // WINBOND, 1MB
    {"WINBOND-W25Q16JV",        0xEF,   0x4015, 32,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // WINBOND, 2MB
    {"WINBOND-W25Q32JV",        0xEF,   0x4016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // WINBOND, 4MB
    {"WINBOND-W25Q64JV",        0xEF,   0x4017, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // WINBOND, 8MB
    {"WINBOND-W25Q128JV",       0xEF,   0x4018, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // WINBOND, 16MB
    {"WINBOND-W25Q256JV",       0xEF,   0x4019, 512,    CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // WINBOND, 32MB
    {"WINBOND-W25Q256JV-IM",    0xEF,   0x7019, 512,    CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // WINBOND, 32MB

    {"MICRON-MT25QL64ABA",      0x20,   0xBA17, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MICRON, 4MB
    {"MICRON-MT25QL128ABA",     0x20,   0xBA18, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MICRON, 16MB
    {"MICRON-MT25QL256ABA",     0x20,   0xBA19, 512,    CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MICRON, 32MB
    {"MICRON-MT25QL512ABB",     0x20,   0xBA20, 1024,   CMD_FAST_READ4B,    CMD_4DTRD4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MICRON, 64MB

    {"MICRON-MT25QU64ABA",      0x20,   0xBB17, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MICRON, 4MB
    {"MICRON-MT25QU128ABA",     0x20,   0xBB18, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // MICRON, 16MB
    {"MICRON-MT25QU256ABA",     0x20,   0xBB19, 512,    CMD_FAST_READ4B,    CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MICRON, 32MB
    {"MICRON-MT25QU512ABB",     0x20,   0xBB20, 1024,   CMD_FAST_READ4B,    CMD_4DTRD4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // MICRON, 64MB

    {"CYPRESS-S25FL116K",       0x01,   0x4015, 32,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 2MB
    {"CYPRESS-S25FL132K",       0x01,   0x4016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 4MB
    {"CYPRESS-S25FL164K",       0x01,   0x4017, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 8MB
    {"CYPRESS-S25FL064L",       0x01,   0x6017, 128,    CMD_READ,           CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 8MB
#if 0
    {"CYPRESS-S25FL064P",       0x01,   0x0216, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 8MB
    {"CYPRESS-S25FL128S",       0x01,   0x2018, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // CYPRESS, 16MB
    {"CYPRESS-S25FL256S",       0x01,   0x0219, 512,    CMD_FAST_READ4B,    CMD_4DTRD4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // CYPRESS, 32MB
#endif
    {"XMC-XM25QH64A",           0x20,   0x7017, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // XMC, 8MB
#if 0
    {"ISSI-IS25LP032",          0x9D,   0x6016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // ISSI, 4MB
#endif
    {"ISSI-IS25LP016D",         0x9D,   0x6015, 32,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // ISSI, 2MB
    {"ISSI-IS25LP032D",         0x9D,   0x6016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // ISSI, 4MB
    {"ISSI-IS25LP256",          0x9D,   0x6019, 512,    CMD_FAST_READ,      CMD_4READ4B,    CMD_PP4B,   ADDR_4B|SECT_4K },  // ISSI, 32MB
    {"ISSI-IS25WP032D",         0x9D,   0x7016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // ISSI, 4MB

    {"GIGADEVICE-GD25Q16C",     0xC8,   0x4015, 32,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // GD, 2MB
    {"GIGADEVICE-GD25Q32C",     0xC8,   0x4016, 64,     CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // GD, 4MB
    {"GIGADEVICE-GD25Q64C",     0xC8,   0x4017, 128,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // GD, 8MB
    {"GIGADEVICE-GD25Q127C",    0xC8,   0x4018, 256,    CMD_FAST_READ,      CMD_4READ,      CMD_PP,     SECT_4K         },  // GD, 16MB
};


static inline uint32 SNOR_READREG(uint32 uiAddr)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

static inline void SNOR_WRITEREG(uint32 uiValue, uint32 uiAddr)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}

static inline void SNOR_cmd_manual_run(uint32 uiDrvIdx, const code_table_info_t * pCmdInfo)
{
    uint32 uiReadedValue = 0u;
    uint32 uiCodeTableAddr = 0u;
    uint32 retry_count = 10000u;


    if((pCmdInfo->size > 0u) && (uiDrvIdx <= (uint32)SFMC1) )
    {
        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((pCmdInfo->offset) << (2u)));

        SNOR_WRITEREG( uiCodeTableAddr,      SFMC_REG_BADDR_MANU(uiDrvIdx));
        SNOR_WRITEREG( SFMC_REG_RUN_MAN_RUN, SFMC_REG_RUN(uiDrvIdx));
        while(retry_count != 0u)
        {
            uiReadedValue = SNOR_READREG(SFMC_REG_RUN(uiDrvIdx));
            if( (uiReadedValue & 0xFu) == 0u)
            {
                break;
            }
            retry_count --;
        }

    }

    if (retry_count == 0u)
    {
        SNOR_MIO_DEBUG("retry_count 0\n");
    }

    return;
}

static inline uint32 SNOR_read_sfmcbuff(uint32 uiDrvIdx)
{
    uint32 uiReadedValue = 0u;
    uint32 uiCodeTableAddr = 0u;

    if((snor_mio_drv[uiDrvIdx].sfmc_buf.size > 0u) && (uiDrvIdx <= (uint32)SFMC1) )
    {
        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((snor_mio_drv[uiDrvIdx].sfmc_buf.offset)<<(2u)));
        uiReadedValue = SNOR_READREG(uiCodeTableAddr);
    }

    return uiReadedValue;
}

static uint32 SNOR_set_codetable(uint32 uiDrvIdx, const code_table_info_t * pCmdInfo, const uint32 * pCode)
{
    uint32 uiCodeTableAddr = 0u;
    uint32 uiI = 0u;
    uint32 uiRetOffset = 0u;

    if((pCmdInfo->offset < 128u) && (pCmdInfo->size < 128u) && (uiDrvIdx <= (uint32)SFMC1))
    {
        uiRetOffset = pCmdInfo->offset + pCmdInfo->size;

        for ( uiI = 0; uiI < pCmdInfo->size; uiI++ )
        {
            uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((pCmdInfo->offset + uiI) << (2u)));
            SNOR_WRITEREG(    pCode[uiI], uiCodeTableAddr);

            SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n", uiCodeTableAddr, pCode[uiI]);

        }
    }

    return uiRetOffset;

}

static void SNOR_MIO_PortConfig(uint32 uiDrvIdx)
{
    if(uiDrvIdx == (uint32)SFMC0)
    {
        (void)GPIO_Config(GPIO_GPC(22u), (uint32)(GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN));//cs
        (void)GPIO_Config(GPIO_GPC(23u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d1
        (void)GPIO_Config(GPIO_GPC(24u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d2
        (void)GPIO_Config(GPIO_GPC(25u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d0
        (void)GPIO_Config(GPIO_GPC(26u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN);//clk
        (void)GPIO_Config(GPIO_GPC(27u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d3
    }
    else
    {
        (void)GPIO_Config(GPIO_GPB(12u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN);//cs
        (void)GPIO_Config(GPIO_GPB(13u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d1
        (void)GPIO_Config(GPIO_GPB(14u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d2
        (void)GPIO_Config(GPIO_GPB(15u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d0
        (void)GPIO_Config(GPIO_GPB(16u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN);//clk
        (void)GPIO_Config(GPIO_GPB(17u), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);              //d3

    }
    return;
}

static void SNOR_MIO_ReadID(uint32 uiDrvIdx, uint8 *pMID, unsigned short *pDevID)
{
    uint32  data = 0u;
    SNOR_MIO_TRACE();

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdid);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    *pMID   = (uint8)((data & 0xFF000000u) >> 24u);
    *pDevID = (unsigned short)((data & 0x00FFFF00u) >> 8u);

    SNOR_MIO_DEBUG("Read ID: 0x%X, MID:0x%02X, DevID:0x%04X\n", data, *pMID, *pDevID);
}

static void SNOR_MIO_4B_Enable(uint32 uiDrvIdx)
{
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b);
}

static uint32 SNOR_MIO_CheckMemoryType(uint32 uiDrvIdx)
{
    uint8 MID = 0u;
    unsigned short DID = 0u;
    sint32 i = 0;
    uint32 uiRet = (uint32)FALSE;

    SNOR_MIO_TRACE();

    SNOR_MIO_ReadID(uiDrvIdx, &MID,&DID);
    SNOR_MIO_DEBUG("SNOR MID: 0x%x, DID: 0x%x\n", MID, DID);

    for ( i=0 ; i<(int)(sizeof(gSNOR_MIO_ProductTable)/sizeof(gSNOR_MIO_ProductTable[0])); i++)
    {
        if ((gSNOR_MIO_ProductTable[i].ManufID == MID) && (gSNOR_MIO_ProductTable[i].DevID == DID))
        {
            snor_mio_drv[uiDrvIdx].name = gSNOR_MIO_ProductTable[i].name;
            snor_mio_drv[uiDrvIdx].ManufID = MID;
            snor_mio_drv[uiDrvIdx].DevID = DID;
            snor_mio_drv[uiDrvIdx].page_size = 256;
            snor_mio_drv[uiDrvIdx].sector_size = (64*1024);
            snor_mio_drv[uiDrvIdx].sector_count = gSNOR_MIO_ProductTable[i].TotalSector;
            snor_mio_drv[uiDrvIdx].size = gSNOR_MIO_ProductTable[i].TotalSector * snor_mio_drv[uiDrvIdx].sector_size;
            SNOR_MIO_DEBUG("SNOR Part name: %s\n", snor_mio_drv[uiDrvIdx].name);
            SNOR_MIO_DEBUG("SNOR sector count: %d\n", snor_mio_drv[uiDrvIdx].sector_count);
            SNOR_MIO_DEBUG("SNOR total size: %d MB\n", (snor_mio_drv[uiDrvIdx].size >> 20));

            snor_mio_drv[uiDrvIdx].flags = gSNOR_MIO_ProductTable[i].flags;

            /* Set Erase sector Size and Command */
            if (( snor_mio_drv[uiDrvIdx].flags & (uint16)SECT_4K) == (uint16)SECT_4K )
            {
                snor_mio_drv[uiDrvIdx].erase_cmd = CMD_ERASE_4K;
                snor_mio_drv[uiDrvIdx].erase_size = 4 * 1024;
            }
            else if ((snor_mio_drv[uiDrvIdx].flags & (uint16)SECT_32K) == (uint16)SECT_32K )
            {
                snor_mio_drv[uiDrvIdx].erase_cmd = CMD_ERASE_32K;
                snor_mio_drv[uiDrvIdx].erase_size = 32 * 1024;
            }
            else
            {
                if (( snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B )
                {
                    snor_mio_drv[uiDrvIdx].erase_cmd = CMD_ERASE_64K_4B;
                }
                else
                {
                    snor_mio_drv[uiDrvIdx].erase_cmd = CMD_ERASE_64K;
                }
                snor_mio_drv[uiDrvIdx].erase_size = snor_mio_drv[uiDrvIdx].sector_size;
            }

            /* Set Read Command*/
            snor_mio_drv[uiDrvIdx].cmd_read     = gSNOR_MIO_ProductTable[i].cmd_read;
            snor_mio_drv[uiDrvIdx].cmd_read_fast    = gSNOR_MIO_ProductTable[i].cmd_read_fast;

            SNOR_MIO_DEBUG("SNOR Read CMD: 0x%04X, Raad Fast CMD: 0x%04X\n", snor_mio_drv[uiDrvIdx].cmd_read, snor_mio_drv[uiDrvIdx].cmd_read_fast);

            /* default Write Command */
            snor_mio_drv[uiDrvIdx].cmd_write = gSNOR_MIO_ProductTable[i].cmd_write;

            SNOR_MIO_DEBUG("SNOR Write CMD: 0x%02X\n", snor_mio_drv[uiDrvIdx].cmd_write);

            snor_mio_drv[uiDrvIdx].current_io_mode = IO_NUM_SINGLE;

            uiRet = (uint32)TRUE;
        }
    }

    return uiRet;
}


static uint32 SNOR_MIO_Make_cmd_readid(uint32 uiDrvIdx)
{
    uint32  code[5] = {0};
    uint32  code_offset = 0u;
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // Buffer & Address variable definess
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].sfmc_buf.offset  = 0u;
    snor_mio_drv[uiDrvIdx].sfmc_buf.size    = (SFMC_BUF_SIZE >> 2u);

    snor_mio_drv[uiDrvIdx].sfmc_addr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].sfmc_buf, &code[0]);

    snor_mio_drv[uiDrvIdx].sfmc_addr.size   = 1u;

    //-------------------------------------------
    // READ ID
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdid.offset  = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].sfmc_addr, &code[0]);


    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_ID, 1u, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_READ_DATA(code[1], 4u, ((snor_mio_drv[uiDrvIdx].sfmc_buf.offset) << (2u)), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdid.size = 3u;

    SNOR_MIO_CT_DEBUG("[READ ID]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdid, &code[0]);

    return code_offset;
}

static uint32 SNOR_MIO_Make_XMC_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[5] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status register (RDSR)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4u, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2u), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].rdsr1.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ Status 2 register (RDSR1)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x09u, 1u, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr1.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
    snor_mio_drv[uiDrvIdx].rdsr2.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr1, &code[0]);

    //-------------------------------------------
    // READ Status 3 register (RDSR2)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x95, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr2.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS2 REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr2, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");

    /* Read Status 2 register is RO type */
    snor_mio_drv[uiDrvIdx].wrsr2.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR2)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xC0, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr2.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS2 REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr2, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);

    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);

    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);

    //-------------------------------------------
    // EAR Write
    //-------------------------------------------


    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);



    return code_offset;
}

static uint32 SNOR_MIO_Make_CYPRESS_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[5] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status-1 register (RDSR1)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ CONFIG-1 register (RDCR1)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x35, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ CONFIG1 REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr1.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr, &code[0]);

    //-------------------------------------------
    // READ CONFIG-2 register (RDCR2)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr1.size = 3;

    SNOR_MIO_CT_DEBUG("[READ CONFIG2 REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr2.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr1, &code[0]);

    //-------------------------------------------
    // READ CONFIG-3 register (RDCR3)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x33, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr2.size = 3;

    SNOR_MIO_CT_DEBUG("[READ CONFIG3 REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr2, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x0002, 2, LA_KEEP_CS, IO_NUM_SINGLE); // Status-1:0x00, Config-1:0x02(QE)
    MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x6078, 2, LA_DEASSERT_CS, IO_NUM_SINGLE); // Config-2:0x60, Config-3:0x78(4 Dummy)
    MK_STOP_CMD(code[3]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 4;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);

    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);

    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);


    //-------------------------------------------
    // EAR Write
    //-------------------------------------------
    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);

    return code_offset;
}

static uint32 SNOR_MIO_Make_WINBOND_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[5] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status register (RDSR)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].rdsr1.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ CONFIG register (RDSR1)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x35, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr1.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
    snor_mio_drv[uiDrvIdx].rdsr2.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr1, &code[0]);

    //-------------------------------------------
    // READ CONFIG register (RDSR2)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr2.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS2 REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr2, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr1.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR1)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x31, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x02, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);   // Status Reg Default value: 0x02(Enable QE bit)
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr1.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS1 REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr2.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr1, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR2)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x11, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x60, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr2.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS2 REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr2, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);

    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);

    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);

    //-------------------------------------------
    // EAR Write
    //-------------------------------------------
    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);


    return code_offset;
}

static uint32 SNOR_MIO_Make_ISSI_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[6] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status register (RDSR)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ AUTOBOOT register (RDAB)
    //-------------------------------------------


    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x14, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ AUTOBOOT REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr, &code[0]);


    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
//  MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40E0, 2, LA_DEASSERT_CS, IO_NUM_SINGLE); // Status Reg: 0x40, Configure Reg: 0x00

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x4000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE); // Status Reg: 0x40, Configure Reg: 0x00
    }
    else
    {
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);   // Status Reg: 0x40, Configure Reg: 0x00
    }
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].wrab.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write AUTOBOOT register (WRAB)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x0000, 2, LA_KEEP_CS, IO_NUM_SINGLE);
        MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x0000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);
        MK_STOP_CMD(code[3]);
        snor_mio_drv[uiDrvIdx].wrab.size = 4;
    }
    else
    {
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
        MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
        MK_WRITE_CMD(code[3], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
        MK_WRITE_CMD(code[4], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
        MK_STOP_CMD(code[5]);
        snor_mio_drv[uiDrvIdx].wrab.size = 6;
    }

    SNOR_MIO_CT_DEBUG("[WRITE AUTOBOOT REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrab, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);

    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);

    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x29, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);

    //-------------------------------------------
    // EAR Write
    //-------------------------------------------

    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);

    return code_offset;
}

static uint32 SNOR_MIO_Make_MICRON_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[5] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status register (RDSR)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ CONFIG register (RDCR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB5, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ CONFIG REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].wrcr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write Configuration register (WRCR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB1, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0xEF8F, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrcr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE CONFIG REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrcr, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);


    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);


    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);


    //-------------------------------------------
    // EAR Write
    //-------------------------------------------

    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);

    return code_offset;
}

static uint32 SNOR_MIO_Make_MXIC_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code[5] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ Status register (RDSR)
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].rdsr.offset  = code_offset;
    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdsr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].rdcr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr, &code[0]);

    //-------------------------------------------
    // READ CONFIG register (RDCR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_CONFIG, 1, LA_KEEP_CS, IO_NUM_SINGLE);

    MK_READ_DATA(code[1], 4, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].rdcr.size = 3;

    SNOR_MIO_CT_DEBUG("[READ CONFIG REG]\n");
    snor_mio_drv[uiDrvIdx].wrsr.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr, &code[0]);

    //-------------------------------------------
    // Write Status register (WRSR)
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
//  MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40E0, 2, LA_DEASSERT_CS, IO_NUM_SINGLE); // Status Reg: 0x40, Configure Reg: 0x00
    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x4000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE); // Status Reg: 0x40, Configure Reg: 0x00
    }
    else{
        MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);   // Status Reg: 0x40, Configure Reg: 0x00
    }
    MK_STOP_CMD(code[2]);
    snor_mio_drv[uiDrvIdx].wrsr.size = 3;

    SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
    snor_mio_drv[uiDrvIdx].write_enable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr, &code[0]);

    //-------------------------------------------
    // Write Enable
    //-------------------------------------------


    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_enable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
    snor_mio_drv[uiDrvIdx].write_disable.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable, &code[0]);

    //-------------------------------------------
    // Write disable
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].write_disable.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
    snor_mio_drv[uiDrvIdx].en4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable, &code[0]);

    //-------------------------------------------
    // enter 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].en4b.size = 2;

    SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ex4b.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].en4b, &code[0]);

    //-------------------------------------------
    // exit 4byte mode
    //-------------------------------------------

    MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ex4b.size = 2;

    SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
    snor_mio_drv[uiDrvIdx].ear_mode.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b, &code[0]);

    //-------------------------------------------
    // EAR Write
    //-------------------------------------------

    code[0] = 0x40002040;
//  MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset << 2), IO_NUM_SINGLE);

    MK_STOP_CMD(code[1]);
    snor_mio_drv[uiDrvIdx].ear_mode.size = 2;

    SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ear_mode, &code[0]);


    return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_erase(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code_count = 0u;
    uint32  addr_io = 0u;
    uint32  cmd_io = 0u;
    uint32  code[10] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // Chip Erase (CE) cmd: 0xC7
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].chip_erase.offset = code_offset;
    code_count = 0;

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)WR_QPP) == (uint16)WR_QPP)
    {
        cmd_io  = IO_NUM_QUAD;
        addr_io = IO_NUM_QUAD;
    }
    else
    {
        cmd_io  = IO_NUM_SINGLE;
        addr_io = IO_NUM_SINGLE;
    }

    MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_CHIP, 1, LA_DEASSERT_CS, cmd_io);
    ++code_count;
    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].chip_erase.size = code_count;

    SNOR_MIO_CT_DEBUG("[CHIP ERASE]\n");
    snor_mio_drv[uiDrvIdx].sec_erase.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].chip_erase, &code[0]);

    //-------------------------------------------
    // Sector Erase (SE) cmd: 0x20 / 4k-byte
    //-------------------------------------------

    code_count = 0;

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)WR_QPP) == (uint16)WR_QPP)
    {
        cmd_io  = IO_NUM_QUAD;
        addr_io = IO_NUM_QUAD;
    }
    else
    {
        cmd_io  = IO_NUM_SINGLE;
        addr_io = IO_NUM_SINGLE;
    }

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_4K_4B, 1, LA_KEEP_CS, cmd_io);
        ++code_count;
        MK_WRITE_DATA(code[code_count], 4, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
    }
    else
    {
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_4K, 1, LA_KEEP_CS, cmd_io);
        ++code_count;
        //MK_WRITE_DATA_3B(code[code_count], 3, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_DEASSERT_CS, addr_io);
    }
    ++code_count;
    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].sec_erase.size = code_count;

    SNOR_MIO_CT_DEBUG("[SECTOR ERASE]\n");
    snor_mio_drv[uiDrvIdx].blk_erase.offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].sec_erase, &code[0]);

    //-------------------------------------------
    // Block Erase (BE) cmd: 0xd8 / 64k-byte
    //-------------------------------------------

    code_count = 0;

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)WR_QPP) == (uint16)WR_QPP)
    {
        cmd_io = IO_NUM_QUAD;
        addr_io= IO_NUM_QUAD;
    }
    else
    {
        cmd_io = IO_NUM_SINGLE;
        addr_io = IO_NUM_SINGLE;
    }

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_64K_4B, 1, LA_KEEP_CS, cmd_io);
        ++code_count;
        MK_WRITE_DATA(code[code_count], 4, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
    }
    else
    {
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_64K, 1, LA_KEEP_CS, cmd_io);
        ++code_count;
        //MK_WRITE_DATA_3B(code[code_count], 3, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_DEASSERT_CS, addr_io);
    }
    ++code_count;

    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].blk_erase.size = code_count;

    SNOR_MIO_CT_DEBUG("[BLOCK ERASE]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].blk_erase, &code[0]);


    return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_write(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code_count = 0u;
    uint32  addr_io = 0u;
    uint32  cmd_io = 0u;
    uint32  write_io = 0u;
    uint32  code[10] = {0};

    SNOR_MIO_TRACE();
    //-------------------------------------------
    // WRITE
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].write.offset = code_offset;
    code_count = 0;

    cmd_io      = IO_NUM_SINGLE;
    addr_io     = IO_NUM_SINGLE;
    write_io    = IO_NUM_SINGLE;

    if (snor_mio_drv[uiDrvIdx].cmd_write == CMD_4PP4B)
    {
        cmd_io      = IO_NUM_SINGLE;
        addr_io     = IO_NUM_QUAD;
        write_io    = IO_NUM_QUAD;
    }
    else if (snor_mio_drv[uiDrvIdx].cmd_write == CMD_PP4B)
    {
        cmd_io      = IO_NUM_SINGLE;
        addr_io     = IO_NUM_SINGLE;
        write_io    = IO_NUM_SINGLE;
    }
    else if (snor_mio_drv[uiDrvIdx].cmd_write == CMD_4PP)
    {
        cmd_io      = IO_NUM_SINGLE;
        addr_io     = IO_NUM_SINGLE;
        write_io    = IO_NUM_QUAD;
    }
    else
    {
        cmd_io      = IO_NUM_SINGLE;
        addr_io     = IO_NUM_SINGLE;
        write_io    = IO_NUM_SINGLE;
    }

    MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv[uiDrvIdx].cmd_write, 1, LA_KEEP_CS, cmd_io);
    ++code_count;

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_PP_ADDR(code[code_count], 4, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
    }
    else
    {
        //MK_WRITE_PP_ADDR_3B(code[code_count], 3, (snor_mio_drv[uiDrvIdx].sfmc_addr.offset), addr_io);
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_KEEP_CS, addr_io);
    }
    ++code_count;
    MK_WRITE_DATA(code[code_count], SFMC_BUF_SIZE, (snor_mio_drv[uiDrvIdx].sfmc_buf.offset), write_io);
    ++code_count;

    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].write.size = code_count;

    SNOR_MIO_CT_DEBUG("[WRITE]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write, &code[0]);


    return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_read(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code_count= 0u;
    uint32  dummy_io= 0u;
    uint32  code[10] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].read.offset = code_offset;

    code_count = 0;

    MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv[uiDrvIdx].cmd_read, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    ++code_count;

    switch (snor_mio_drv[uiDrvIdx].cmd_read)
    {
        case CMD_READ:
        case CMD_READ4B:
            dummy_io    = IO_NUM_NC;
            break;

        case CMD_FAST_READ:
        case CMD_FAST_READ4B:
            dummy_io    = IO_NUM_SINGLE;
            break;

        default:
            dummy_io    = IO_NUM_NC;
            break;
    }

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_ADDR_AHB(code[code_count], D_DTR_DISABLE, IO_NUM_SINGLE);
    }
    else
    {
        MK_WRITE_ADDR_AHB_3B(code[code_count], D_DTR_DISABLE, IO_NUM_SINGLE);
    }

    ++code_count;

    if (dummy_io != IO_NUM_NC)
    {
        MK_WRITE_DUMMY_CYCLE(code[code_count], D_DTR_DISABLE, 1, dummy_io);
        ++code_count;
    }

    MK_READ_DATA_AHB(code[code_count], D_DTR_DISABLE, IO_NUM_SINGLE);
    ++code_count;
    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].read.size = code_count;

    SNOR_MIO_CT_DEBUG("[READ]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].read, &code[0]);


    return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_read_fast(uint32 uiDrvIdx, uint32 code_offset)
{
    uint32  code_count = 0u;
    uint32  addr_io = 0u;
    uint32  read_io = 0u;
    uint32  dummy_io = 0u;
    uint32  cmd_io = 0u;
    uint32  cmd_count = 0u;
    uint32  dummy_count = 0u;
    uint32  performance_enhance = 0u;
    uint32  code[10] = {0};
    SNOR_MIO_TRACE();

    //-------------------------------------------
    // READ
    //-------------------------------------------
    snor_mio_drv[uiDrvIdx].read_fast.offset = code_offset;

    // Set default
    snor_mio_drv[uiDrvIdx].dt_mode = D_DTR_DISABLE;
    cmd_io = IO_NUM_SINGLE;
    dummy_count = 1;
    cmd_count = 1;
    performance_enhance = (uint32)FALSE;

    switch (snor_mio_drv[uiDrvIdx].cmd_read_fast)
    {
        case CMD_READ:
        case CMD_READ4B:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_SINGLE;
            dummy_io    = IO_NUM_NC;
            break;

        case CMD_FAST_READ:
        case CMD_FAST_READ4B:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_SINGLE;
            dummy_io    = IO_NUM_SINGLE;
            break;

        case CMD_FASTDTRD_4B:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_SINGLE;
            dummy_io    = IO_NUM_SINGLE;
            snor_mio_drv[uiDrvIdx].dt_mode = D_DTR_ENABLE;
            break;

        case CMD_DREAD:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_DUAL;
            dummy_io    = IO_NUM_SINGLE;
            break;

        case CMD_QREAD:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_QUAD;
            dummy_io    = IO_NUM_SINGLE;
            cmd_io      = IO_NUM_SINGLE;
            break;

        case CMD_4DTRD:
        case CMD_4DTRD4B:
            addr_io     = IO_NUM_QUAD;
            read_io     = IO_NUM_QUAD;
            dummy_io    = IO_NUM_QUAD;
            cmd_io      = IO_NUM_SINGLE;
            snor_mio_drv[uiDrvIdx].dt_mode = D_DTR_ENABLE;
            dummy_count = 8;
            performance_enhance = (uint32)TRUE;
            break;

        case CMD_4READ:
        case CMD_4READ4B:
            addr_io     = IO_NUM_QUAD;
            read_io     = IO_NUM_QUAD;
            dummy_io    = IO_NUM_QUAD;
            cmd_io      = IO_NUM_SINGLE;
            snor_mio_drv[uiDrvIdx].dt_mode = D_DTR_DISABLE;
            dummy_count = 2;
            performance_enhance = (uint32)TRUE;
            break;

        case CMD_QREAD4B:
            addr_io     = IO_NUM_SINGLE;
            dummy_io    = IO_NUM_SINGLE;
            read_io     = IO_NUM_QUAD;
            cmd_io      = IO_NUM_SINGLE;
            break;

        case CMD_8READ:
            addr_io     = IO_NUM_OCTA;
            read_io     = IO_NUM_OCTA;
            dummy_io    = IO_NUM_OCTA;
            cmd_io      = IO_NUM_OCTA;
            dummy_count = 0x14;
            cmd_count   = 2;
            break;

        case CMD_8DTRD:
            addr_io     = IO_NUM_OCTA;
            read_io     = IO_NUM_OCTA;
            dummy_io    = IO_NUM_OCTA;
            cmd_io      = IO_NUM_OCTA;
            dummy_count = 0x15;
            cmd_count   = 2;
            snor_mio_drv[uiDrvIdx].dt_mode = D_DTR_ENABLE;
            break;

        default:
            addr_io     = IO_NUM_SINGLE;
            read_io     = IO_NUM_SINGLE;
            dummy_io    = IO_NUM_NC;
            break;
    }

    code_count = 0;

    if (cmd_io == IO_NUM_OCTA)
    {
        MK_WRITE_CMD(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, snor_mio_drv[uiDrvIdx].cmd_read_fast, cmd_count, LA_KEEP_CS, cmd_io);
    }
    else
    {
        MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv[uiDrvIdx].cmd_read_fast, cmd_count, LA_KEEP_CS, cmd_io);
    }
    ++code_count;

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        MK_WRITE_ADDR_AHB(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, addr_io);
    }
    else
    {
        MK_WRITE_ADDR_AHB_3B(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, addr_io);
    }

    ++code_count;

    if (performance_enhance == (uint32)TRUE)
    {
        // performance enhance mode
        MK_WRITE_CMD(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, 0x00, 1, LA_KEEP_CS, dummy_io);
        ++code_count;
    }

    if (dummy_io != IO_NUM_NC)
    {
        MK_WRITE_DUMMY_CYCLE(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, dummy_count, dummy_io);
        ++code_count;
    }

    MK_READ_DATA_AHB(code[code_count], snor_mio_drv[uiDrvIdx].dt_mode, read_io);
    ++code_count;
    MK_STOP_CMD(code[code_count]);
    ++code_count;
    snor_mio_drv[uiDrvIdx].read_fast.size = code_count;

    SNOR_MIO_CT_DEBUG("[READ_FAST]\n");
    code_offset = SNOR_set_codetable(uiDrvIdx, &snor_mio_drv[uiDrvIdx].read_fast, &code[0]);


    return code_offset;
}

static void SNOR_MIO_Make_CMD(uint32 uiDrvIdx, uint32 code_offset)
{
    SNOR_MIO_TRACE();
    code_offset = SNOR_MIO_Make_cmd_read(uiDrvIdx, code_offset);
    code_offset = SNOR_MIO_Make_cmd_write(uiDrvIdx, code_offset);
    code_offset = SNOR_MIO_Make_cmd_erase(uiDrvIdx, code_offset);
    (void)SNOR_MIO_Make_cmd_read_fast(uiDrvIdx, code_offset);
}


sint32 SNOR_MIO_Init(uint32 uiDrvIdx)
{
    uint32 code_offset = 0u;
    sint32 res = (sint32)FALSE;
    uint8 ucMemory_type = 0u;

    SNOR_MIO_TRACE("drv idx = %d\n",uiDrvIdx);

    if(uiDrvIdx <= (uint32)SFMC1)
    {
        SNOR_MIO_PortConfig(uiDrvIdx);


        //-------------------------------------------
        // Controller Setting
        //-------------------------------------------
        //SNOR_MIO_DEBUG("SNOR Mode Selection\n");

        //SAL_TaskSleep(10);
        SNOR_WRITEREG(((uint32)(SFMC_REG_RUN_SOFT_RESET)|(uint32)(SFMC_REG_RUN_AUTO_STOP)),  SFMC_REG_RUN(uiDrvIdx));
        SNOR_WRITEREG((uint32)SFMC_REG_MODE_FLASH_RESET,  SFMC_REG_MODE(uiDrvIdx));

        SAL_TaskSleep(10u);

        SNOR_WRITEREG((~(uint32)SFMC_REG_RUN_SOFT_RESET),  SFMC_REG_RUN(uiDrvIdx));

        SAL_TaskSleep(10u);

        /* 80M 100M tested */
        /*
             SFMC_REG_TIMING_SC_EXTND(0) |
             SFMC_REG_TIMING_CS_TO_CS(4) |
             SFMC_REG_TIMING_READ_LATENCY(3) |
             SFMC_REG_TIMING_SEL_DQS_FCLK_PAD
        */
        SNOR_WRITEREG( 0x40310u, (SFMC_REG_TIMING(uiDrvIdx)) );

        /*
             SFMC_REG_DELAY_SO_INV_SCLK(1) |
             SFMC_REG_DELAY_SO_SLCH(1)
        */
        SNOR_WRITEREG( 0x500u, (SFMC_REG_DELAY_SO(uiDrvIdx)) );
        SNOR_WRITEREG( 0x0c0cu, (SFMC_REG_DELAY_CLK(uiDrvIdx)) );

        SNOR_MIO_DEBUG("SPI-STR mode Set Done\n");

        code_offset = SNOR_MIO_Make_cmd_readid(uiDrvIdx);

        if (SNOR_MIO_CheckMemoryType(uiDrvIdx) == (uint32)TRUE)
        {
            switch (snor_mio_drv[uiDrvIdx].ManufID)
            {
                case 0xEFu:
                case 0xC8u: /* GigaDevice is the same as Winbond */
                    code_offset = SNOR_MIO_Make_WINBOND_CMD(uiDrvIdx, code_offset);
                    break;
                case 0x01u:
                    code_offset = SNOR_MIO_Make_CYPRESS_CMD(uiDrvIdx, code_offset);
                    break;
                case 0x20u:
                    ucMemory_type = (uint8)((snor_mio_drv[uiDrvIdx].DevID >> 8u) & 0xFFu);
                    if (ucMemory_type == (uint8)0x70u) /* XMC */
                    {
                        code_offset = SNOR_MIO_Make_XMC_CMD(uiDrvIdx, code_offset);
                    }
                    else /* MICRON */
                    {
                        code_offset = SNOR_MIO_Make_MICRON_CMD(uiDrvIdx, code_offset);
                    }
                    break;
                case 0x9Du:
                    code_offset = SNOR_MIO_Make_ISSI_CMD(uiDrvIdx, code_offset);
                    break;
                default:
                    code_offset = SNOR_MIO_Make_MXIC_CMD(uiDrvIdx, code_offset);
                    break;
            }
#if 0
            if ( snor_mio_drv[uiDrvIdx].flags & ADDR_4B)
                SNOR_MIO_4B_Enable();
#endif
            SNOR_MIO_Make_CMD(uiDrvIdx, code_offset);

            switch (snor_mio_drv[uiDrvIdx].ManufID)
            {
                case 0xEFu:
                case 0xC8u:
                    (void)SNOR_MIO_WINBOND_DefaultConfig(uiDrvIdx);
                    break;
                case 0x01u:
                    (void)SNOR_MIO_CYPRESS_DefaultConfig(uiDrvIdx);
                    break;
                case 0x20u:
                    ucMemory_type = (uint8)((snor_mio_drv[uiDrvIdx].DevID >> 8u) & 0xFFu);
                    if (ucMemory_type == (uint8)0x70u) /* XMC */
                    {
                        (void)SNOR_MIO_XMC_DefaultConfig(uiDrvIdx);
                    }
                    else /* MICRON */
                    {
                        (void)SNOR_MIO_MICRON_DefaultConfig(uiDrvIdx);
                    }
                    break;
                case 0x9Du:
                    (void)SNOR_MIO_ISSI_DefaultConfig(uiDrvIdx);
                    break;
                default:
                    (void)SNOR_MIO_MXIC_DefaultConfig(uiDrvIdx);
                    break;
            }

            SAL_MemSet(snor_mio_drv[uiDrvIdx].uiDataBuff, 0xFF, SNOR_SECTOR_SIZE);

            res = 0;
        }
        else
        {
            SNOR_MIO_DEBUG("Failed to read SNOR ID\n");
            res = -1;
        }
    }
    return res;
}

#if 0
void sfmc_test_SWRST_RC_TIME (uint32 warm_boot, uint32 reset_enable, uint32 reset_start, uint32 cmd_iteration, uint32 run_iteration)
{
    uint32  addr, data_temp, manu_run;
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x00) = reset_enable;
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x04) = 0xF00003E8; //enough, tSHSL : 7ns ~ 30ns
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x08) = reset_start;

        if (warm_boot != 1)
        { // cold boot, just 40us reset recovery time
            *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x0C) = 0xF0002710; // 40us
            *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x10) = 0xF4000000;
        }
        else
        { // warm boot, wait command iteration
            for (addr=0;addr<cmd_iteration;addr++)
            {
                *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x0C+(addr<<2)) = 0xF0002710; // 40us x 255ea(max) -> 10.2ms
            }
            *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x0C+(addr<<2)) = 0xF4000000;
        }

        *(volatile uint32 *)(SFMC_REG_BADDR_MANU) = 0x00000800;
        *(volatile uint32 *)(SFMC_REG_RUN)        = 0x00000001;

            while(1)
            {
                data_temp = *(volatile uint32 *)(SFMC_REG_RUN);
                if (data_temp==0x0)
                        break;
            }

        if (warm_boot == 1)
        { // warm boot, manual run iteration
            for (manu_run=0;manu_run<run_iteration-1;manu_run++)
            {
                *(volatile uint32 *)(SFMC_REG_BADDR_MANU) = 0x0000080C;
                *(volatile uint32 *)(SFMC_REG_RUN)        = 0x00000001;

                while(1)
                {
                    data_temp = *(volatile uint32 *)(SFMC_REG_RUN);
                    if (data_temp==0x0)
                        break;
                }
            } //run iteration
        } //warm boot only

}

void sfmc_test_RST_RC_TIME(uint32 warm_boot, uint32 otp_rst_rcvl)
{
    uint32 reset_enable;
    uint32 reset_start;
    uint32 spi_swreset;
    uint32 qpi_swreset;
    uint32 opi_str_swreset;
    uint32 opi_dtr_swreset;
    uint32 cmd_iteration;
    uint32 run_iteration;
    uint32 swreset_mode;
    uint32  nReg, data_temp, addr, manu_run;
    /* caution!!!!! reset recovery time needs from 40us to 1s
    40us(0xF0002710) -> 40us                                                 for cold boot
    40us(0xF0002710) x command_iteration(255ea) x run_iteration(100ea) -> 1s for warm boot
    */

    cmd_iteration = (otp_rst_rcvl & 0x000000FF) >>  0;
    //DBG_PRINTF("cmd_iteration : %x", cmd_iteration);
    run_iteration = (otp_rst_rcvl & 0x0000FF00) >>  8;
    //DBG_PRINTF("run_iteration : %x", run_iteration);
    swreset_mode  = (otp_rst_rcvl & 0xF0000000) >> 28;
    //DBG_PRINTF("swreset_mode : %x", swreset_mode);

    spi_swreset     = (swreset_mode & 0x1) >> 0;
    qpi_swreset     = (swreset_mode & 0x2) >> 1;
    opi_str_swreset = (swreset_mode & 0x4) >> 2;
    opi_dtr_swreset = (swreset_mode & 0x8) >> 3;

    //////////////////////////////////////////
    //H/W Reset Recovery Time Configuration
    //////////////////////////////////////////
    *(volatile uint32 *)(SFMC_REG_MODE)       = 0x00000000;
    for (nReg=0;nReg<0x20;nReg++); // don't remove!!!! (Reset# low pulse width > 10us)
    *(volatile uint32 *)(SFMC_REG_MODE)       = 0x00000015;

    if (warm_boot != 1)
    { // cold boot, just 40us reset recovery time
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x00) = 0xF0002710; // 40us
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x04) = 0xF4000000;

        *(volatile uint32 *)(SFMC_REG_BADDR_MANU) = 0x00000800;
        *(volatile uint32 *)(SFMC_REG_RUN)        = 0x00000001;

            while(1)
            {
                data_temp = *(volatile uint32 *)(SFMC_REG_RUN);
                if (data_temp==0x0)
                        break;
            }

    }
    else
    { // warm_boot, reset recovery time is configured by otp
        for (addr=0;addr<cmd_iteration;addr++)
        {
            *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x00+(addr<<2)) = 0xF0002710; // 40us x 255ea(max) -> 10.2ms
        }
        *(volatile uint32 *)(SFMC_REG_CODE_TABLE + 0x00+(addr<<2)) = 0xF4000000;

        for (manu_run=0;manu_run<run_iteration;manu_run++)
        { // 10.2ms x 255ea(max) -> 2.6s
            *(volatile uint32 *)(SFMC_REG_BADDR_MANU) = 0x00000800;
            *(volatile uint32 *)(SFMC_REG_RUN)        = 0x00000001;

            while(1)
            {
                data_temp = *(volatile uint32 *)(SFMC_REG_RUN);
                if (data_temp==0x0)
                    break;
            }
        } //run_iteration
    } //cold vs. warm

    //////////////////////////////////////////
    //S/W Reset Recovery Time Configuration
    //////////////////////////////////////////
    //otp_rst_rcvl[28] == 1    -> SPI S/W reset Enable
    if  (spi_swreset)
    {
        reset_enable = 0xA4000066;
        reset_start  = 0xA4000099;

        SNOR_MIO_DEBUG(" SPI S/W Reset\n");
        sfmc_test_SWRST_RC_TIME(warm_boot,reset_enable,reset_start,cmd_iteration,run_iteration);
    }
    //otp_rst_rcvl[29] == 1    -> QPI S/W reset Enable
    if  (qpi_swreset)
    {
        reset_enable = 0xA6000066;
        reset_start  = 0xA6000099;

        SNOR_MIO_DEBUG(" QPI S/W Reset\n");
        sfmc_test_SWRST_RC_TIME(warm_boot,reset_enable,reset_start,cmd_iteration,run_iteration);
    }
    //otp_rst_rcvl[30] == 1    -> OPI_STR S/W reset Enable
    if  (opi_str_swreset)
    {
        reset_enable = 0xAB006699;
        reset_start  = 0xAB009966;

        SNOR_MIO_DEBUG(" OPI_STR S/W Reset\n");
        sfmc_test_SWRST_RC_TIME(warm_boot,reset_enable,reset_start,cmd_iteration,run_iteration);
    }
    //otp_rst_rcvl[31] == 1    -> OPI_DTR S/W reset Enable
    if  (opi_dtr_swreset)
    {
        reset_enable = 0xAB006699; //check!! change to dtr mode
        reset_start  = 0xAB009966; //check!! change to dtr mode

        SNOR_MIO_DEBUG(" OPI_DTR S/W Reset\n");
        sfmc_test_SWRST_RC_TIME(warm_boot,reset_enable,reset_start,cmd_iteration,run_iteration);
    }
}
#endif

static unsigned short SNOR_MIO_ReadConfig_2B(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    unsigned short  config = 0u;
    SNOR_MIO_TRACE();

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    config = (unsigned short)((data & 0xFFFF0000u) >> 16u);

    return config;
}

static uint8 SNOR_MIO_ReadConfig(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   config = 0u;
    SNOR_MIO_TRACE();

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    config = (uint8)(data & 0xFFu);

    return config;
}

static uint8 SNOR_MIO_ReadConfig1(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   config = 0u;
    SNOR_MIO_TRACE();

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr1);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    config = (uint8)(data & 0xFFu);

    return config;
}

static uint8 SNOR_MIO_ReadConfig2(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   config = 0u;
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdcr2);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    config = (uint8)(data & 0xFFu);

    return config;
}

static uint8 SNOR_MIO_ReadStatus(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   status = 0u;
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    status = (uint8)(data & 0xFFu);

    return status;
}

static uint8 SNOR_MIO_ReadStatus1(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   status = 0u;
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr1);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    status = (uint8)(data & 0xFFu);

    return status;
}

static uint8 SNOR_MIO_ReadStatus2(uint32 uiDrvIdx)
{
    uint32  data = 0u;
    uint8   status = 0u;
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].rdsr2);
    data = SNOR_read_sfmcbuff(uiDrvIdx);

    status = (uint8)(data & 0xFFu);

    return status;
}

static sint32 SNOR_MIO_WaitWriteComplete(uint32 uiDrvIdx)
{
    uint8 nStatus = 0u;
    uint32 retry_count = 0u;
    sint32 iRet = 0;

    retry_count = 100000u;
    while(retry_count != 0u)
    {
        nStatus = (uint8)SNOR_MIO_ReadStatus(uiDrvIdx);
        //if ((ISZERO(nStatus,Hw0)) == 1u) //check WIP
        if ((nStatus & Hw0) == 0u) //check WIP
        {
            break;
        }
        --retry_count;

        SAL_TaskSleep(1u);


    }

    if (retry_count == 0u)
    {
        SNOR_MIO_DEBUG("retry_count 0\n");
        iRet = -1;
    }

    return iRet;
}

static void SNOR_MIO_4B_Disable(uint32 uiDrvIdx)
{
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].ex4b);
}


static sint32 SNOR_MIO_WriteEnable(uint32 uiDrvIdx)
{
    uint8 nStatus = 0u;
    uint32 retry_count = 0u;
    sint32 iRet = 0;

    SNOR_MIO_TRACE();
    retry_count = 10000u;
    while(retry_count != 0u)
    {
        SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_enable);

        nStatus = (uint8)SNOR_MIO_ReadStatus(uiDrvIdx);

        if((nStatus & Hw1) == Hw1)
        {
            break;
        }

        --retry_count;
        SAL_TaskSleep(1u);

    }

    if (retry_count == 0u)
    {
        iRet = -1;
    }
    return iRet;
}

static sint32 SNOR_MIO_ISSI_DefaultConfig(uint32 uiDrvIdx)
{
    uint8   status = 0u;
    uint8   config = 0u;
    int     iRet = 0;
    SNOR_MIO_TRACE();

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        status = SNOR_MIO_ReadStatus(uiDrvIdx);
        SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);

        //if ((ISZERO(status, Hw6))== 1u)
        if((status & Hw6) == 0u)
        {
            SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");
            iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

            status = SNOR_MIO_ReadStatus(uiDrvIdx);
            SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);
        }

        config = SNOR_MIO_ReadConfig(uiDrvIdx);
        SNOR_MIO_DEBUG("Read AUTOBOOT Reg - Config: 0x%02X\n", config);
    //    if ((!ISZERO(config, Hw0))== 1u)
        if((config & Hw0) == Hw0)
        {
            SNOR_MIO_DEBUG("Disable AUTOBOOT\n");
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrab);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

            config = SNOR_MIO_ReadConfig(uiDrvIdx);
            SNOR_MIO_DEBUG("Read AUTOBOOT Reg - Config: 0x%02X\n", config);
        }
    }

    return iRet;
}


static sint32 SNOR_MIO_MICRON_DefaultConfig(uint32 uiDrvIdx)
{
    int     iRet = 0;
    SNOR_MIO_TRACE();

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {

        SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));

    //  if (ISZERO(status,BIT(6)))
        {
            SNOR_MIO_DEBUG("Set to default status for FWDN\n");
            iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

            SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
        }

        SNOR_MIO_DEBUG("Set to default configuration for FWDN\n");
        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
        SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrcr);
        iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

        SNOR_MIO_DEBUG("Read Config Reg - Config: 0x%04X\n", SNOR_MIO_ReadConfig_2B(uiDrvIdx));
    }

    return iRet;
}

static sint32 SNOR_MIO_MXIC_DefaultConfig(uint32 uiDrvIdx)
{
    uint8   status = 0u;
    int     iRet = 0;
    SNOR_MIO_TRACE();

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        status = SNOR_MIO_ReadStatus(uiDrvIdx);
        SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);

        //if((ISZERO(status,Hw6)) == 1u)
        if((status & Hw6) == 0u)
        {
            SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");
            iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

            SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
        }

        SNOR_MIO_DEBUG("Read Config Reg - Config: 0x%02X\n", SNOR_MIO_ReadConfig(uiDrvIdx));
    }

    return iRet;
}

static sint32 SNOR_MIO_WINBOND_WRSR1_Default(uint32 uiDrvIdx)
{
    sint32 iRet = 0;
    SNOR_MIO_TRACE();

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);

        if(iRet == 0)
        {
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr1);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
        }
    }

    return iRet;
}

static sint32 SNOR_MIO_WINBOND_WRSR2_Default(uint32 uiDrvIdx)
{
    sint32  iRet = 0;
    SNOR_MIO_TRACE();
    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);

        if(iRet == 0)
        {
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr2);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
        }
    }

    return iRet;
}

static sint32 SNOR_MIO_WINBOND_DefaultConfig(uint32 uiDrvIdx)
{
    sint32  iRet = 0;
    SNOR_MIO_TRACE();
    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Status1 Reg(35h) - status: 0x%02X\n", SNOR_MIO_ReadStatus1(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Status2 Reg(15h) - status: 0x%02X\n", SNOR_MIO_ReadStatus2(uiDrvIdx));

        iRet = SNOR_MIO_WINBOND_WRSR1_Default(uiDrvIdx);
        SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");

        if(iRet == 0)
        {
            iRet = SNOR_MIO_WINBOND_WRSR2_Default(uiDrvIdx);
            SNOR_MIO_DEBUG("Clear WPS (Write Protect Selection) bit\n");

            SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
            SNOR_MIO_DEBUG("Read Status1 Reg(35h) - status: 0x%02X\n", SNOR_MIO_ReadStatus1(uiDrvIdx));
            SNOR_MIO_DEBUG("Read Status2 Reg(15h) - status: 0x%02X\n", SNOR_MIO_ReadStatus2(uiDrvIdx));
        }
    }

    return iRet;
}

static sint32 SNOR_MIO_CYPRESS_DefaultConfig(uint32 uiDrvIdx)
{
    int     iRet = 0;
    SNOR_MIO_TRACE();

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        SNOR_MIO_DEBUG("Read Status-1 Reg - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-1 Reg - status: 0x%02X\n", SNOR_MIO_ReadConfig(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-2 Reg - status: 0x%02X\n", SNOR_MIO_ReadConfig1(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-3 Reg - status: 0x%02X\n", SNOR_MIO_ReadConfig2(uiDrvIdx));

        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
        SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr);

        iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
        SNOR_MIO_DEBUG("Default set for TCC\n");

        SNOR_MIO_DEBUG("Read Status-1 Reg - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-1 Reg - Config: 0x%02X\n", SNOR_MIO_ReadConfig(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-2 Reg - status: 0x%02X\n", SNOR_MIO_ReadConfig1(uiDrvIdx));
        SNOR_MIO_DEBUG("Read Config-3 Reg - status: 0x%02X\n", SNOR_MIO_ReadConfig2(uiDrvIdx));
    }

    return iRet;
}

static sint32 SNOR_MIO_XMC_DefaultConfig(uint32 uiDrvIdx)
{
    sint32  iRet = 0;
    SNOR_MIO_TRACE();
    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
        if(iRet == 0)
        {
            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr);
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

            if(iRet == 0)
            {
                SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].wrsr2);
                iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);

                SNOR_MIO_DEBUG("Default set for TCC\n");

                SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", SNOR_MIO_ReadStatus(uiDrvIdx));
                SNOR_MIO_DEBUG("Read Status2 Reg(09h) - status: 0x%02X\n", SNOR_MIO_ReadStatus1(uiDrvIdx));
                SNOR_MIO_DEBUG("Read Status3 Reg(95h) - status: 0x%02X\n", SNOR_MIO_ReadStatus2(uiDrvIdx));
            }
        }
    }

    return iRet;
}

static void SNOR_MIO_WriteDisable(uint32 uiDrvIdx)
{
    SNOR_MIO_TRACE();
    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write_disable);
}

static sint32 SNOR_MIO_ChipErase(uint32 uiDrvIdx)
{
    sint32 iRet = 0;

    SNOR_MIO_TRACE();
    SNOR_MIO_DEBUG("%s\n", __func__);

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else
    {
        iRet = SNOR_MIO_WriteEnable(uiDrvIdx);
        SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].chip_erase);

        if(iRet == 0)
        {
            iRet = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
        }
    }

    return iRet;
}

static sint32 SNOR_MIO_BlockErase(uint32 uiDrvIdx, uint32 address)
{
    sint32 res = 0;
    uint32 uiCodeTableAddr = 0u;
    uint32 uiReadedValue = 0u;

    SNOR_MIO_TRACE();
    SNOR_MIO_DEBUG("blk_erase: 0x%08X\n", address);
    //SNOR_MIO_WriteEAR(address);
    //address = (address & 0x00FFFFFF);
    res = SNOR_MIO_WriteEnable(uiDrvIdx);

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        //address |= ( CMD_ERASE_64K<< 24) & 0xFF000000;
        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((snor_mio_drv[uiDrvIdx].sfmc_addr.offset)<<(2u)));
        SNOR_WRITEREG(address, uiCodeTableAddr);

    }
    else
    {
        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].blk_erase.offset) + (1u))<<(2u)));
        uiReadedValue = SNOR_READREG(uiCodeTableAddr);
        uiReadedValue = ((uiReadedValue & 0xFF000000u) | (address & 0xFFFFFFu));
        SNOR_WRITEREG(uiReadedValue, uiCodeTableAddr);
    }

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].blk_erase);

    res = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
    return res;
}

static sint32 SNOR_MIO_SectorErase(uint32 uiDrvIdx, uint32 address)
{
    sint32 res = 0;
    uint32 uiCodeTableAddr = 0u;
    uint32 uiReadedValue = 0u;

    SNOR_MIO_TRACE();
    //SNOR_MIO_DEBUG("sec_erase: 0x%08X\n", address);
    //SNOR_MIO_WriteEAR(address);
    //address = (address & 0x00FFFFFF);
    res = SNOR_MIO_WriteEnable(uiDrvIdx);

    if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
    {
        //address |= (snor_mio_drv[uiDrvIdx].erase_cmd << 24) & 0xFF000000;

        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((snor_mio_drv[uiDrvIdx].sfmc_addr.offset)<<(2u)));
        SNOR_WRITEREG(address, uiCodeTableAddr);
    }
    else
    {
        uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].sec_erase.offset) + (1u))<<(2u)));
        uiReadedValue = SNOR_READREG(uiCodeTableAddr);
        uiReadedValue = ((uiReadedValue & 0xFF000000u) | (address & 0xFFFFFFu));
        SNOR_WRITEREG(uiReadedValue, uiCodeTableAddr);
    }

    SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].sec_erase);

    res = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
    return res;
}

void SNOR_MIO_Erase(uint32 uiDrvIdx, uint32 address, uint32 size)
{
    uint32 lba = 0;
    uint32 nPercent, nPercent_unit;

    uint32 count_4k = 0;

    const uint32 uiUnit64k = (64u * 1024u);
    const uint32 uiUnit4k = (4u * 1024u);

    SNOR_MIO_TRACE();

    //SNOR_MIO_DEBUG("[SNOR Erase] addr: 0x%08x, size: %8d kbyte     ", address, size>>10);
    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("invalid drv idx\n");
    }
    else if ((address & 0xFFFu) != 0u)
    {
        SNOR_MIO_DEBUG("Address alignment error.\n");
        SNOR_MIO_DEBUG("Set the alignment of the address in %d bytes.\n", SNOR_SECTOR_SIZE);
    }
    else
    {
        if (((address & 0xFFFFu) != 0u )&& (size > uiUnit64k))
        {
            count_4k = uiUnit64k - (address & 0xFFFFu);

            for(lba = 0u;lba < count_4k; lba += uiUnit4k)
            {
                //SNOR_MIO_DEBUG("Sector Erase : 0x%08x\n\r", address + lba);
                (void)SNOR_MIO_SectorErase(uiDrvIdx, (address + lba));
            }

            address += count_4k;
            size -= count_4k;
        }

        nPercent = 0u;
        nPercent_unit = ( size / uiUnit64k);
        for (lba = 0u; lba < nPercent_unit; lba++)
        {
            //if (lba && !(lba%41))
            //  SNOR_MIO_DEBUG("\r\n");
            if (nPercent < 99u)
            {
                //  nPercent += 100/nPercent_unit;
                nPercent = (100u*lba)/nPercent_unit;
                SNOR_MIO_DEBUG("\b\b\b\b%3d%%", nPercent);
            }

            //SNOR_MIO_DEBUG("Sector Erase: 0x%08x, nPercent_unit: %d, nPercent: %d\n", address, nP     ercent_unit, nPercent);
            (void)SNOR_MIO_BlockErase(uiDrvIdx, address);
            if((address < 0xF00000u) && (size >= uiUnit64k))
            {
                /* To prevent arithmetic results from being wrapped */
                address += uiUnit64k;
                size -= uiUnit64k;
            }
        }

        if (size > 0u)
        {
            nPercent_unit = size % uiUnit4k;
            if (nPercent_unit != 0u)
            {
                nPercent_unit = (size / uiUnit4k) + 1u;
            }
            else
            {
                nPercent_unit = size / uiUnit4k;
            }

            for (lba = 0u;lba < nPercent_unit; lba ++)
            {
                //SNOR_MIO_DEBUG("Sector Erase : 0x%08x\n", address);
                (void)SNOR_MIO_SectorErase(uiDrvIdx, address);
                if((address < 0xF00000u) && (size >= uiUnit4k))
                {
                    /* To prevent arithmetic results from being wrapped */
                    address += uiUnit4k;
                    size -= uiUnit4k;
                }
            }
        }

        SNOR_MIO_DEBUG("\b\b\b\b%3d%%", 100u);
        SNOR_MIO_DEBUG(" OK.\r\n");
    }
}

static sint32 SNOR_MIO_WriteByte(uint32 uiDrvIdx, uint32 address, const uint8 *pBuffer, uint32 lengthInByte)
{
    uint32 sfmc_buff_addr = 0u;
    uint32 sfmc_traget_addr = 0u;
    uint32 uiReadedValue = 0u;
    uint32 write_address = 0u;
    uint8  remain[SNOR_PAGE_SIZE];
    sint32 iRet = 0;

    sint32 res = 0;
    uint32 uiSrcIdx = 0u;
    uint32 uiRequestLength = lengthInByte;

    SNOR_MIO_TRACE();
    SNOR_MIO_DEBUG("[%s]address: 0x%08X, buffer: 0x%08X, length: %d\n", __func__, address, pBuffer, lengthInByte);

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("invalid drv idx\n");
    }
    else if ((address & 0xFFu) != 0u)
    {
        SNOR_MIO_DEBUG("Address alignment error.\n");
        SNOR_MIO_DEBUG("Set the alignment of the address in %d bytes.\n", SNOR_PAGE_SIZE);
    }
    else
    {

        while(lengthInByte > SNOR_PAGE_SIZE)
        {
            write_address = address;

            res = SNOR_MIO_WriteEnable(uiDrvIdx);
            if(res != 0)
            {
                SNOR_MIO_DEBUG("SNOR_MIO_WriteEnable error.\n");
                iRet = -1;
            }

            sfmc_buff_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].sfmc_buf.offset)<<(2u)) & 0x1FFu));

            /*
            not necessary
            SAL_MemSet((void*)sfmc_buff_addr, 0xFF, SNOR_PAGE_SIZE);
            */
            SAL_MemCopy((void*)sfmc_buff_addr, &pBuffer[uiSrcIdx], SNOR_PAGE_SIZE);

            if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
            {
                sfmc_traget_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].sfmc_addr.offset)<<(2u)) & 0x1FFu));
                SNOR_WRITEREG(write_address, sfmc_traget_addr);
            }
            else
            {
                sfmc_traget_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((((snor_mio_drv[uiDrvIdx].write.offset) + (1u))<<(2u)) &    0x1FFu));
                uiReadedValue = SNOR_READREG(sfmc_traget_addr);
                uiReadedValue = ((uiReadedValue & 0xFF000000u) | (write_address & 0xFFFFFFu));
                SNOR_WRITEREG(uiReadedValue, sfmc_traget_addr);
            }

            SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write);

            res = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
            if(res != 0)
            {
                SNOR_MIO_DEBUG("SNOR_MIO_WaitWriteComplete error.\n");
                iRet = -1;
            }

            if((address < 0xF00000u) && (uiSrcIdx < uiRequestLength))
            {
                /* To prevent arithmetic results from being wrapped */
                uiSrcIdx += SNOR_PAGE_SIZE;
                address += SNOR_PAGE_SIZE;
            }

            lengthInByte -= SNOR_PAGE_SIZE;
        }

        /* Remain written data */
        SAL_MemSet(remain, 0xFF, SNOR_PAGE_SIZE);
        SAL_MemCopy(remain, &pBuffer[uiSrcIdx], lengthInByte);

        write_address = address;

        res = SNOR_MIO_WriteEnable(uiDrvIdx);
        if(res != 0)
        {
            SNOR_MIO_DEBUG("SNOR_MIO_WriteEnable error.\n");
            iRet = -1;
        }

        sfmc_buff_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].sfmc_buf.offset)<<(2u)) & 0x1FFu) );

        SAL_MemCopy((void*)sfmc_buff_addr, remain, SNOR_PAGE_SIZE);

        if ((snor_mio_drv[uiDrvIdx].flags & (uint16)ADDR_4B) == (uint16)ADDR_4B)
        {
            sfmc_traget_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + (((snor_mio_drv[uiDrvIdx].sfmc_addr.offset)<<(2u))  & 0x1FFu) );
            SNOR_WRITEREG(write_address, sfmc_traget_addr);
        }
        else
        {
            sfmc_traget_addr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((((snor_mio_drv[uiDrvIdx].write.offset) + (1u))<<(2u)) & 0x1FFu) );
            uiReadedValue = SNOR_READREG(sfmc_traget_addr);
            uiReadedValue = ((uiReadedValue & 0xFF000000u) | (write_address & 0xFFFFFFu));
            SNOR_WRITEREG(uiReadedValue, sfmc_traget_addr);
        }

        SNOR_cmd_manual_run(uiDrvIdx, &snor_mio_drv[uiDrvIdx].write);

        res = SNOR_MIO_WaitWriteComplete(uiDrvIdx);
        if(res != 0)
        {
            SNOR_MIO_DEBUG("SNOR_MIO_WaitWriteComplete error.\n");
            iRet = -1;
        }
    }

    return iRet;
}

sint32 SNOR_MIO_Write(uint32 uiDrvIdx, uint32 address, const uint8 *pBuffer, uint32 length)
{
    SNOR_MIO_TRACE();
    SNOR_MIO_DEBUG("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", address, (uint32)pBuffer, length);

    (void)SNOR_MIO_WriteByte(uiDrvIdx, address, pBuffer, length);

    return 0;
}

static sint32 SNOR_MIO_ReadByte_fast(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 lengthInByte)
{
    sint32 res = -1;
    uint32 uiAddress = 0u;
    SNOR_MIO_TRACE();
//  SNOR_MIO_DEBUG("address: 0x%08X, buffer: 0x%08X, length: %d\n", address, (uint32)pBuffer, lengthInByte);
    if((uiDrvIdx <= (uint32)SFMC1) && (address < 0x10000000u))
    {
        SNOR_MIO_AutoRun(uiDrvIdx, 1u, 1u);
        uiAddress = (NOR_FLASH_BASE_ADDR(uiDrvIdx) + address);
        SAL_MemCopy(pBuffer, (void *)uiAddress, lengthInByte);
        SNOR_MIO_AutoRun(uiDrvIdx, 0u, 1u);
        res = 0;
    }

    return res;
}


static sint32 SNOR_MIO_ReadByte(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 lengthInByte)
{
    sint32 res = -1;
    uint32 uiAddress = 0u;
    SNOR_MIO_TRACE();
    //  SNOR_MIO_DEBUG("[%s]address: 0x%08X, buffer: 0x%08X, length: %d\n", __func__, address, (uint32)pBuffer, lengthInByte);
    if((uiDrvIdx <= (uint32)SFMC1) && (address < 0x10000000u))
    {
        SNOR_MIO_AutoRun(uiDrvIdx, 1u, 0u);
        uiAddress = (NOR_FLASH_BASE_ADDR(uiDrvIdx) + address);
        SAL_MemCopy(pBuffer, (void *)uiAddress, lengthInByte);
        SNOR_MIO_AutoRun(uiDrvIdx, 0u, 0u);
        res = 0;
    }

    return res;
}

sint32 SNOR_MIO_Read_Fast(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 length)
{
    sint32 res;

    SNOR_MIO_TRACE();

    res = SNOR_MIO_ReadByte_fast(uiDrvIdx, address, pBuffer, length);

    return res;
}

sint32 SNOR_MIO_Read(uint32 uiDrvIdx, uint32 address, uint8 *pBuffer, uint32 length)
{
    sint32 res;

    SNOR_MIO_TRACE();

    res = SNOR_MIO_ReadByte(uiDrvIdx, address, pBuffer, length);

    return res;
}

sint32 SNOR_MIO_FWDN_LowFormat(uint32 uiDrvIdx)
{
    sint32 res = 0;

    res = SNOR_MIO_ChipErase(uiDrvIdx);

    return res;
}

sint32 SNOR_MIO_FWDN_Read(uint32 uiDrvIdx, uint32 address, uint32 length, uint8 *buff)
{
    sint32 iRet = 0;

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else if((address > 0x10000000u ) || (length > 0x10000000u ) || (snor_mio_drv[uiDrvIdx].size < (address + length)))
    {
        SNOR_MIO_DEBUG("[%s]Exceed Size\n", __func__);
        iRet = -1;
    }
    else
    {
        iRet = SNOR_MIO_Read(uiDrvIdx, address, buff, length);
    }

    return iRet;
}

sint32 SNOR_MIO_FWDN_Write(uint32 uiDrvIdx, uint32 address, uint32 length, const uint8 *buf)
{
    const uint8 *data= (const uint8 *)0;
    uint32 i = 0u;
    uint32 size = 0u;
    sint32 res = 0;
    sint32 iRet = 0;
    uint32 total_sec_cnt = 0u;
    uint32 uiSrcIdx = 0u;
    uint32 uiRequestLength = length;

    SNOR_MIO_DEBUG("%s[%d]: address=0x%X, length=0x%X, buf=0x%p\n", __func__, __LINE__, address, length, buf);

    if(uiDrvIdx > (uint32)SFMC1)
    {
        SNOR_MIO_DEBUG("[%s]invalid drv index\n", __func__);
        iRet = -1;
    }
    else if ((address > (uint32)0x200000 ) || (length > (uint32)0x200000 ) || (snor_mio_drv[uiDrvIdx].size < (address + length)))
    {
        SNOR_MIO_DEBUG("[%s]Exceed Size\n", __func__);
        iRet = -1;
    }
    else
    {
        data = buf;
        SAL_MemSet(snor_mio_drv[uiDrvIdx].uiDataBuff, 0xFF, SNOR_SECTOR_SIZE);


        if ((address % SNOR_SECTOR_SIZE) != 0u)
        {
            /* Sector Read */
            (void)SNOR_MIO_Read(uiDrvIdx, (address & 0xFFFFF000u), snor_mio_drv[uiDrvIdx].uiDataBuff, SNOR_SECTOR_SIZE);
            i = (address & 0xFFFu);
            if (length > (SNOR_SECTOR_SIZE - i))
            {
                size = (length - (SNOR_SECTOR_SIZE - i));
            }
            else
            {
                size = length;
            }

            SNOR_MIO_DEBUG("Erase & Write less than 4KB of data, address: 0x%X\n", address);
            SAL_MemCmp( &snor_mio_drv[uiDrvIdx].uiDataBuff[i], &data[uiSrcIdx], size, &res);

            if (res != 0)
            {
                /* Data Merge */
                SAL_MemCopy(&snor_mio_drv[uiDrvIdx].uiDataBuff[i], &data[uiSrcIdx], size);

                (void)SNOR_MIO_SectorErase(uiDrvIdx, (address & ~(0xFFFu)));
                (void)SNOR_MIO_WriteByte(uiDrvIdx, (address & ~(0xFFFu)), snor_mio_drv[uiDrvIdx].uiDataBuff, SNOR_SECTOR_SIZE);
            }
            if(size <= SNOR_SECTOR_SIZE)
            {
                /* To prevent arithmetic results from being wrapped */
                address += size;
            }

            uiSrcIdx += size;
//            data += size;
            if(length >= size)
            {
                /* To prevent arithmetic results from being wrapped */
                length -= size;
            }
        }

        /* Write data in 4KBytes */
        total_sec_cnt = (length / SNOR_SECTOR_SIZE);
        SNOR_MIO_DEBUG("Erase & Write data in 4KBytes, address: 0x%X, total_sec_cnt=%d\n", address, total_sec_cnt);
        for (i = 0; i < total_sec_cnt; i++)
        {
            (void)SNOR_MIO_Read(uiDrvIdx, address, snor_mio_drv[uiDrvIdx].uiDataBuff, SNOR_SECTOR_SIZE);

            SAL_MemCmp(snor_mio_drv[uiDrvIdx].uiDataBuff, &data[uiSrcIdx], SNOR_SECTOR_SIZE, &res);

            if (res != 0)
            {
                (void)SNOR_MIO_SectorErase(uiDrvIdx, address);
                (void)SNOR_MIO_WriteByte(uiDrvIdx, address, &data[uiSrcIdx], SNOR_SECTOR_SIZE);
            }

            address += SNOR_SECTOR_SIZE;
            if(uiSrcIdx < uiRequestLength)
            {
                /* To prevent arithmetic results from being wrapped */
                uiSrcIdx += SNOR_SECTOR_SIZE;
            }
           // data += SNOR_SECTOR_SIZE;
            length -= SNOR_SECTOR_SIZE;
        }

        /* Remaining data */
        if (length > 0u)
        {
            SNOR_MIO_DEBUG("Erase & Write remaining data, address: 0x%X, length=%d\n", address, length);
            (void)SNOR_MIO_Read(uiDrvIdx, address, snor_mio_drv[uiDrvIdx].uiDataBuff, SNOR_SECTOR_SIZE);

            SAL_MemCmp(snor_mio_drv[uiDrvIdx].uiDataBuff, &data[uiSrcIdx], length, &res);

            if (res != 0)
            {
                SAL_MemCopy(snor_mio_drv[uiDrvIdx].uiDataBuff, &data[uiSrcIdx], length);

                (void)SNOR_MIO_SectorErase(uiDrvIdx, address);
                (void)SNOR_MIO_WriteByte(uiDrvIdx, address, snor_mio_drv[uiDrvIdx].uiDataBuff, SNOR_SECTOR_SIZE);
            }
        }
    }
    return iRet;
}

void SNOR_MIO_AutoRun(uint32 uiDrvIdx, uint32 uiFlag, uint32 uiFast)
{
    uint32 uiCodeTableAddr = 0u;

    if(uiDrvIdx <= (uint32)SFMC1 )
    {

        if(uiFlag == 1u)
        {
            if(uiFast == 1u)
            {
                uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((snor_mio_drv[uiDrvIdx].read_fast.offset << (2u)) & 0x1FFu)   );
            }
            else
            {
                uiCodeTableAddr = ((SFMC_REG_CODE_TABLE(uiDrvIdx)) + ((snor_mio_drv[uiDrvIdx].read.offset << (2u)) & 0x1FFu)   );
            }
            SNOR_WRITEREG( uiCodeTableAddr,      SFMC_REG_BADDR_AUTO(uiDrvIdx));
            SNOR_WRITEREG( SFMC_REG_RUN_AUTO_RUN, SFMC_REG_RUN(uiDrvIdx));
        }
        else
        {
            SNOR_WRITEREG( SFMC_REG_RUN_AUTO_STOP, SFMC_REG_RUN(uiDrvIdx));
        }
    }

}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SFMC == 1 )

