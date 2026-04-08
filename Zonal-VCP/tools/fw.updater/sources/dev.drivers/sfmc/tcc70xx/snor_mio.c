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

#include <sal_impl.h>
#include <debug.h>
#include <gpio.h>
#include "snor_mio.h"
#include "snor_mio_gdma.h"
#if 0
#define SNOR_MIO_DEBUG(fmt, args...)   	{LOGD(DBG_TAG_SFMC, fmt, ## args)}//mcu_printf(fmt, ## args)
#define SNOR_MIO_CT_DEBUG(fmt, args...)	{LOGD(DBG_TAG_SFMC, fmt, ## args)}//mcu_printf(fmt, ## args)
#define SNOR_MIO_TRACE(fmt, args...)	{LOGD(DBG_TAG_SFMC, fmt, ## args)}//{LOGD(DBG_TAG_SFMC, fmt, ## args)}
#else
#define SNOR_MIO_DEBUG(fmt, args...)
#define SNOR_MIO_CT_DEBUG(fmt, args...)
#define SNOR_MIO_TRACE(fmt, args...)
#endif
//#define SNOR_MIO_GDMA			1
#ifdef SNOR_MIO_GDMA
#define HwPL080_BASE        0x1B600000
#define HwPL080         ((volatile DMA_sPort *)HwPL080_BASE)
#endif


//extern void arch_invalidate_cache_range(addr_t start, size_t len);

static sint32 SNOR_MIO_ISSI_DefaultConfig(void);
static sint32 SNOR_MIO_MICRON_DefaultConfig(void);
static sint32 SNOR_MIO_MXIC_DefaultConfig(void);
static sint32 SNOR_MIO_WINBOND_DefaultConfig(void);
static sint32 SNOR_MIO_CYPRESS_DefaultConfig(void);
static sint32 SNOR_MIO_XMC_DefaultConfig(void);

snor_mio_drv_t snor_mio_drv;

/************************************************************************************************************

                                        Serial Nor Flash Product Table

************************************************************************************************************/
snor_product_info_t gSNOR_MIO_ProductTable[] =
{
	/*
	|Seral Flash				|MID	|DID	|Sector |Read CMD			|F_Read 		|Write CMD	|Flag			|
	|NAME						|		|		|Count	|					|				|			|				|
	*/
	{"MXIC-MX25L1633E",			0xC2,	0x2415, 32, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MXIC, 2MB
	{"MXIC-MX25L3233F",			0xC2,	0x2016, 64, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MXIC, 4MB
	{"MXIC-MX25L6435E",			0xC2,	0x2017, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MXIC, 8MB
	{"MXIC-MX25L12835F",		0xC2,	0x2018, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MXIC, 16MB
	{"MXIC-MX25L25645G",		0xC2,	0x2019, 512,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B,	ADDR_4B|SECT_4K },	// MXIC, 32MB
	{"MXIC-MX25L51245G",		0xC2,	0x201A, 1024,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B,	ADDR_4B|SECT_4K },	// MXIC, 64MB
	{"MXIC-MX25LM51245G",		0xC2,	0x853A, 1024,	CMD_FAST_READ4B,	CMD_8READ,		CMD_PP4B,	ADDR_4B|SECT_4K },	// MXIC, 64MB

	{"WINBOND-W25X20CL",		0xEF,	0x3012, 4, 		CMD_FAST_READ,		CMD_DREAD,		CMD_PP, 	SECT_4K 		},	// WINBOND, 256Kbyte
	{"WINBOND-W25X40CL",		0xEF,	0x3013, 8,	 	CMD_FAST_READ,		CMD_DREAD,		CMD_PP, 	SECT_4K 		},	// WINBOND, 512Kbyte
	{"WINBOND-W25Q80JV",		0xEF,	0x4014, 16, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// WINBOND, 1MB
	{"WINBOND-W25Q16JV",		0xEF,	0x4015, 32,		CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// WINBOND, 2MB
	{"WINBOND-W25Q32JV",		0xEF,	0x4016, 64,		CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// WINBOND, 4MB
	{"WINBOND-W25Q64JV",		0xEF,	0x4017, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// WINBOND, 8MB
	{"WINBOND-W25Q128JV",		0xEF,	0x4018, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// WINBOND, 16MB
	{"WINBOND-W25Q256JV",		0xEF,	0x4019, 512,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B, 	ADDR_4B|SECT_4K },	// WINBOND, 32MB
	{"WINBOND-W25Q256JV-IM",	0xEF,	0x7019, 512,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B, 	ADDR_4B|SECT_4K },	// WINBOND, 32MB

	{"MICRON-MT25QL64ABA",		0x20,	0xBA17, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MICRON, 4MB
	{"MICRON-MT25QL128ABA",		0x20,	0xBA18, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MICRON, 16MB
	{"MICRON-MT25QL256ABA",		0x20,	0xBA19, 512,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B, 	ADDR_4B|SECT_4K },	// MICRON, 32MB
	{"MICRON-MT25QL512ABB",		0x20,	0xBA20, 1024,	CMD_FAST_READ4B,	CMD_4DTRD4B,	CMD_PP4B,	ADDR_4B|SECT_4K },	// MICRON, 64MB

	{"MICRON-MT25QU64ABA",		0x20,	0xBB17, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MICRON, 4MB
	{"MICRON-MT25QU128ABA",		0x20,	0xBB18, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// MICRON, 16MB
	{"MICRON-MT25QU256ABA",		0x20,	0xBB19, 512,	CMD_FAST_READ4B,	CMD_4READ4B,	CMD_PP4B, 	ADDR_4B|SECT_4K },	// MICRON, 32MB
	{"MICRON-MT25QU512ABB",		0x20,	0xBB20, 1024,	CMD_FAST_READ4B,	CMD_4DTRD4B,	CMD_PP4B,	ADDR_4B|SECT_4K },	// MICRON, 64MB

	{"CYPRESS-S25FL116K",		0x01,	0x4015, 32,		CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 2MB
	{"CYPRESS-S25FL132K",		0x01,	0x4016, 64,		CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 4MB
	{"CYPRESS-S25FL164K",		0x01,	0x4017, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 8MB
	{"CYPRESS-S25FL064L",		0x01,	0x6017, 128,	CMD_READ,			CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 8MB
#if 0
	{"CYPRESS-S25FL064P",		0x01,	0x0216, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 8MB
	{"CYPRESS-S25FL128S",		0x01,	0x2018, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// CYPRESS, 16MB
	{"CYPRESS-S25FL256S",		0x01,	0x0219, 512,	CMD_FAST_READ4B,	CMD_4DTRD4B,	CMD_PP4B,	ADDR_4B|SECT_4K },	// CYPRESS, 32MB
#endif
	{"XMC-XM25QH64A",			0x20,	0x7017, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP,		SECT_4K			},	// XMC, 8MB
#if 0
	{"ISSI-IS25LP032",			0x9D,	0x6016, 64, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K 		},	// ISSI, 4MB
#endif
	{"ISSI-IS25LP016D",			0x9D,	0x6015, 32, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K			},	// ISSI, 2MB
	{"ISSI-IS25LP032D",			0x9D,	0x6016, 64, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K			},	// ISSI, 4MB
	{"ISSI-IS25LP256",			0x9D,	0x6019, 512, 	CMD_FAST_READ,		CMD_4READ4B,	CMD_PP4B, 	ADDR_4B|SECT_4K	},	// ISSI, 32MB
	{"ISSI-IS25WP032D",			0x9D,	0x7016, 64, 	CMD_FAST_READ,		CMD_4READ,		CMD_PP, 	SECT_4K			},	// ISSI, 4MB

	{"GIGADEVICE-GD25Q16C",		0xC8,	0x4015, 32,		CMD_FAST_READ,		CMD_4READ,		CMD_PP,		SECT_4K			},	// GD, 2MB
	{"GIGADEVICE-GD25Q32C",		0xC8,	0x4016, 64,		CMD_FAST_READ,		CMD_4READ,		CMD_PP,		SECT_4K			},	// GD, 4MB
	{"GIGADEVICE-GD25Q64C",		0xC8,	0x4017, 128,	CMD_FAST_READ,		CMD_4READ,		CMD_PP,		SECT_4K			},	// GD, 8MB
	{"GIGADEVICE-GD25Q127C",	0xC8,	0x4018, 256,	CMD_FAST_READ,		CMD_4READ,		CMD_PP,		SECT_4K			},	// GD, 16MB
};


static void SNOR_MIO_PortConfig(void)
{
	GPIO_Config(GPIO_GPB(12), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(13), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(14), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(15), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(16), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_NOPULL | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(17), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);
	GPIO_Config(GPIO_GPB(18), GPIO_FUNC(1U) | GPIO_INPUT | GPIO_INPUTBUF_EN);
}

static void SNOR_MIO_ReadID(uint8 *pMID, unsigned short *pDevID)
{
	uint32	data;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdid.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);

	*pMID 	= ((data & 0xFF000000) >> 24);
	*pDevID = ((data & 0x00FFFF00) >> 8);

	SNOR_MIO_DEBUG("Read ID: 0x%X, MID:0x%02X, DevID:0x%04X\n", data, *pMID, *pDevID);
}

static void SNOR_MIO_4B_Enable(void)
{
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.en4b.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
}

static sint32 SNOR_MIO_CheckMemoryType(void)
{
	uint8 MID;
	unsigned short DID;
	sint32 i;

	SNOR_MIO_TRACE();

	SNOR_MIO_ReadID(&MID,&DID);
	SNOR_MIO_DEBUG("SNOR MID: 0x%x, DID: 0x%x\n", MID, DID);

	for ( i=0 ; i<(int)(sizeof(gSNOR_MIO_ProductTable)/sizeof(gSNOR_MIO_ProductTable[0])); i++)
	{
		if (gSNOR_MIO_ProductTable[i].ManufID == MID && gSNOR_MIO_ProductTable[i].DevID == DID)
		{
			snor_mio_drv.name = gSNOR_MIO_ProductTable[i].name;
			snor_mio_drv.ManufID = MID;
			snor_mio_drv.DevID = DID;
			snor_mio_drv.page_size = 256;
			snor_mio_drv.sector_size = (64*1024);
			snor_mio_drv.sector_count = gSNOR_MIO_ProductTable[i].TotalSector;
			snor_mio_drv.size = gSNOR_MIO_ProductTable[i].TotalSector * snor_mio_drv.sector_size;
			SNOR_MIO_DEBUG("SNOR Part name: %s\n", snor_mio_drv.name);
			SNOR_MIO_DEBUG("SNOR sector count: %d\n", snor_mio_drv.sector_count);
			SNOR_MIO_DEBUG("SNOR total size: %d MB\n", (snor_mio_drv.size >> 20));

			snor_mio_drv.flags = gSNOR_MIO_ProductTable[i].flags;

			/* Set Erase sector Size and Command */
			if ( snor_mio_drv.flags& SECT_4K)
			{
				snor_mio_drv.erase_cmd = CMD_ERASE_4K;
				snor_mio_drv.erase_size = 4 * 1024;
			}
			else if (snor_mio_drv.flags & SECT_32K)
			{
				snor_mio_drv.erase_cmd = CMD_ERASE_32K;
				snor_mio_drv.erase_size = 32 * 1024;
			}
			else
			{
				if ( snor_mio_drv.flags & ADDR_4B)
					snor_mio_drv.erase_cmd = CMD_ERASE_64K_4B;
				else
				    snor_mio_drv.erase_cmd = CMD_ERASE_64K;
				snor_mio_drv.erase_size = snor_mio_drv.sector_size;
			}

			/* Set Read Command*/
			snor_mio_drv.cmd_read		= gSNOR_MIO_ProductTable[i].cmd_read;
			snor_mio_drv.cmd_read_fast 	= gSNOR_MIO_ProductTable[i].cmd_read_fast;

			SNOR_MIO_DEBUG("SNOR Read CMD: 0x%04X, Raad Fast CMD: 0x%04X\n", snor_mio_drv.cmd_read, snor_mio_drv.cmd_read_fast);

			/* default Write Command */
			snor_mio_drv.cmd_write = gSNOR_MIO_ProductTable[i].cmd_write;

			SNOR_MIO_DEBUG("SNOR Write CMD: 0x%02X\n", snor_mio_drv.cmd_write);

			snor_mio_drv.current_io_mode = IO_NUM_SINGLE;

			return TRUE;
		}
	}

	return FALSE;
}

static uint32 SNOR_MIO_Get_Code_Offset(code_table_info_t *code)
{
	SNOR_MIO_TRACE();
	return (code->offset + code->size);
}

static uint32 SNOR_MIO_Make_cmd_readid(void)
{
	uint32	i;
	uint32 	code[5];
	uint32	code_offset;

	SNOR_MIO_TRACE();

	//-------------------------------------------
	// Buffer & Address variable definess
	//-------------------------------------------
	snor_mio_drv.sfmc_buf.offset 	= 0;
	snor_mio_drv.sfmc_buf.size 	= (SFMC_BUF_SIZE >> 2);

	snor_mio_drv.sfmc_addr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.sfmc_buf); //offset: 0x40
	snor_mio_drv.sfmc_addr.size 	= 1;

	//-------------------------------------------
	// READ ID
	//-------------------------------------------
	snor_mio_drv.rdid.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.sfmc_addr);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_ID, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdid.size = 3;

	SNOR_MIO_CT_DEBUG("[READ ID]\n");
	for ( i = 0; i < snor_mio_drv.rdid.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdid.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdid.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdid.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdid);

	return code_offset;
}

static uint32 SNOR_MIO_Make_XMC_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[5];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status register (RDSR)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ Status 2 register (RDSR1)
	//-------------------------------------------
	snor_mio_drv.rdsr1.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x09, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr1.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr1.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr1.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr1.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr1.offset + i)));
	}

	//-------------------------------------------
	// READ Status 3 register (RDSR2)
	//-------------------------------------------
	snor_mio_drv.rdsr2.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr1);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x95, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr2.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS2 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr2.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr2.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr2.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr2.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr2);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	/* Read Status 2 register is RO type */

	//-------------------------------------------
	// Write Status register (WRSR2)
	//-------------------------------------------
	snor_mio_drv.wrsr2.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xC0, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr2.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS2 REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr2.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr2.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr2.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr2.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr2);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_CYPRESS_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[5];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status-1 register (RDSR1)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG-1 register (RDCR1)
	//-------------------------------------------
	snor_mio_drv.rdcr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x35, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ CONFIG1 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG-2 register (RDCR2)
	//-------------------------------------------
	snor_mio_drv.rdcr1.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr1.size = 3;

	SNOR_MIO_CT_DEBUG("[READ CONFIG2 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr1.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr1.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr1.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr1.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG-3 register (RDCR3)
	//-------------------------------------------
	snor_mio_drv.rdcr2.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr1);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x33, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr2.size = 3;

	SNOR_MIO_CT_DEBUG("[READ CONFIG3 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr2.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr2.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr2.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr2.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr2);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x0002, 2, LA_KEEP_CS, IO_NUM_SINGLE);	// Status-1:0x00, Config-1:0x02(QE)
	MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x6078, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Config-2:0x60, Config-3:0x78(4 Dummy)
	MK_STOP_CMD(code[3]);
	snor_mio_drv.wrsr.size = 4;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_WINBOND_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[5];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status register (RDSR)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG register (RDSR1)
	//-------------------------------------------
	snor_mio_drv.rdsr1.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x35, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr1.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS1 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr1.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr1.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr1.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr1.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG register (RDSR2)
	//-------------------------------------------
	snor_mio_drv.rdsr2.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr1);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr2.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS2 REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr2.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr2.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr2.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr2.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr2);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR1)
	//-------------------------------------------
	snor_mio_drv.wrsr1.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x31, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x02, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg Default value: 0x02(Enable QE bit)
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr1.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS1 REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr1.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr1.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr1.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr1.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR2)
	//-------------------------------------------
	snor_mio_drv.wrsr2.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr1);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x11, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x60, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr2.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS2 REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr2.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr2.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr2.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr2.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr2);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_ISSI_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[6];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status register (RDSR)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ AUTOBOOT register (RDAB)
	//-------------------------------------------
	snor_mio_drv.rdcr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x14, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ AUTOBOOT REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
//	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40E0, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    if ( snor_mio_drv.flags & ADDR_4B)
	{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x4000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    }
	else
	{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    }
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	//-------------------------------------------
	// Write AUTOBOOT register (WRAB)
	//-------------------------------------------
	snor_mio_drv.wrab.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x15, 1, LA_KEEP_CS, IO_NUM_SINGLE);
    if ( snor_mio_drv.flags & ADDR_4B)
	{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x0000, 2, LA_KEEP_CS, IO_NUM_SINGLE);
		MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x0000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);
		MK_STOP_CMD(code[3]);
		snor_mio_drv.wrab.size = 4;
    }
	else
	{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	    MK_WRITE_CMD(code[2], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	    MK_WRITE_CMD(code[3], D_DTR_DISABLE, 0x00, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	    MK_WRITE_CMD(code[4], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
		MK_STOP_CMD(code[5]);
		snor_mio_drv.wrab.size = 6;
    }

	SNOR_MIO_CT_DEBUG("[WRITE AUTOBOOT REG]\n");
	for ( i = 0; i < snor_mio_drv.wrab.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrab.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrab.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrab.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrab);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x29, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_MICRON_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[5];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status register (RDSR)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG register (RDCR)
	//-------------------------------------------
	snor_mio_drv.rdcr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB5, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ CONFIG REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x00, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	//-------------------------------------------
	// Write Configuration register (WRCR)
	//-------------------------------------------
	snor_mio_drv.wrcr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB1, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0xEF8F, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrcr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE CONFIG REG]\n");
	for ( i = 0; i < snor_mio_drv.wrcr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrcr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrcr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrcr.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrcr);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_MXIC_CMD(uint32 code_offset)
{
	uint32	i;
	uint32 	code[5];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ Status register (RDSR)
	//-------------------------------------------
	snor_mio_drv.rdsr.offset 	= code_offset;
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdsr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.rdsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdsr.offset + i)));
	}

	//-------------------------------------------
	// READ CONFIG register (RDCR)
	//-------------------------------------------
	snor_mio_drv.rdcr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdsr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_READ_CONFIG, 1, LA_KEEP_CS, IO_NUM_SINGLE);

	MK_READ_DATA(code[1], 4, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[2]);
	snor_mio_drv.rdcr.size = 3;

	SNOR_MIO_CT_DEBUG("[READ CONFIG REG]\n");
	for ( i = 0; i < snor_mio_drv.rdcr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.rdcr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.rdcr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.rdcr.offset + i)));
	}

	//-------------------------------------------
	// Write Status register (WRSR)
	//-------------------------------------------
	snor_mio_drv.wrsr.offset 	= SNOR_MIO_Get_Code_Offset(&snor_mio_drv.rdcr);
	MK_WRITE_CMD(code[0], D_DTR_DISABLE, CMD_WRITE_STATUS, 1, LA_KEEP_CS, IO_NUM_SINGLE);
//	MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40E0, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    if ( snor_mio_drv.flags & ADDR_4B)
	{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x4000, 2, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    }
	else{
	    MK_WRITE_CMD(code[1], D_DTR_DISABLE, 0x40, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);	// Status Reg: 0x40, Configure Reg: 0x00
    }
	MK_STOP_CMD(code[2]);
	snor_mio_drv.wrsr.size = 3;

	SNOR_MIO_CT_DEBUG("[WRITE STATUS REG]\n");
	for ( i = 0; i < snor_mio_drv.wrsr.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.wrsr.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.wrsr.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.wrsr.offset + i)));
	}

	//-------------------------------------------
	// Write Enable
	//-------------------------------------------
	snor_mio_drv.write_enable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.wrsr);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x06, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_enable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE ENABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_enable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_enable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_enable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_enable.offset + i)));
	}

	//-------------------------------------------
	// Write disable
	//-------------------------------------------
	snor_mio_drv.write_disable.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_enable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0x04, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.write_disable.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE DISABLE]\n");
	for ( i = 0; i < snor_mio_drv.write_disable.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.write_disable.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write_disable.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write_disable.offset + i)));
	}

	//-------------------------------------------
	// enter 4byte mode
	//-------------------------------------------
	snor_mio_drv.en4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.write_disable);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xB7, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.en4b.size = 2;

	SNOR_MIO_CT_DEBUG("[ENTER 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.en4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.en4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.en4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.en4b.offset + i)));
	}

	//-------------------------------------------
	// exit 4byte mode
	//-------------------------------------------
	snor_mio_drv.ex4b.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.en4b);

	MK_WRITE_CMD(code[0], D_DTR_DISABLE, 0xE9, 1, LA_DEASSERT_CS, IO_NUM_SINGLE);
	MK_STOP_CMD(code[1]);
	snor_mio_drv.ex4b.size = 2;

	SNOR_MIO_CT_DEBUG("[EXIT 4byte mode]\n");
	for ( i = 0; i < snor_mio_drv.ex4b.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ex4b.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
								 	(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ex4b.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ex4b.offset + i)));
	}

	//-------------------------------------------
	// EAR Write
	//-------------------------------------------
	snor_mio_drv.ear_mode.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ex4b);
	code[0] = 0x40002040;
//	MK_WRITE_DATA(code[1], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset << 2), IO_NUM_SINGLE);

	MK_STOP_CMD(code[1]);
	snor_mio_drv.ear_mode.size = 2;

	SNOR_MIO_CT_DEBUG("[WRITE EAR]\n");
	for ( i = 0; i < snor_mio_drv.ear_mode.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.ear_mode.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.ear_mode.offset + i)));
	}

	code_offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.ear_mode);

	return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_erase(uint32 code_offset)
{
	uint32	i;
	uint32	code_count;
	uint32	addr_io, cmd_io;
	uint32 	code[10];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// Chip Erase (CE) cmd: 0xC7
	//-------------------------------------------
	snor_mio_drv.chip_erase.offset = code_offset;
	code_count = 0;

	if (snor_mio_drv.flags & WR_QPP)
	{
		cmd_io 	= IO_NUM_QUAD;
		addr_io	= IO_NUM_QUAD;
	}
	else
	{
		cmd_io 	= IO_NUM_SINGLE;
		addr_io = IO_NUM_SINGLE;
	}

	MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_CHIP, 1, LA_DEASSERT_CS, cmd_io);
	++code_count;
	MK_STOP_CMD(code[code_count]);
	++code_count;
	snor_mio_drv.chip_erase.size = code_count;

	SNOR_MIO_CT_DEBUG("[CHIP ERASE]\n");
	for ( i = 0; i < snor_mio_drv.chip_erase.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.chip_erase.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.chip_erase.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.chip_erase.offset + i)));
	}

	//-------------------------------------------
	// Sector Erase (SE) cmd: 0x20 / 4k-byte
	//-------------------------------------------
	snor_mio_drv.sec_erase.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.chip_erase);
	code_count = 0;

	if (snor_mio_drv.flags & WR_QPP)
	{
		cmd_io 	= IO_NUM_QUAD;
		addr_io	= IO_NUM_QUAD;
	}
	else
	{
		cmd_io 	= IO_NUM_SINGLE;
		addr_io = IO_NUM_SINGLE;
	}

	if ( snor_mio_drv.flags & ADDR_4B)
	{
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_4K_4B, 1, LA_KEEP_CS, cmd_io);
		++code_count;
		MK_WRITE_DATA(code[code_count], 4, (snor_mio_drv.sfmc_addr.offset), addr_io);
	}
	else
	{
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_4K, 1, LA_KEEP_CS, cmd_io);
		++code_count;
		//MK_WRITE_DATA_3B(code[code_count], 3, (snor_mio_drv.sfmc_addr.offset), addr_io);
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_DEASSERT_CS, addr_io);
	}
	++code_count;
	MK_STOP_CMD(code[code_count]);
	++code_count;
	snor_mio_drv.sec_erase.size = code_count;

	SNOR_MIO_CT_DEBUG("[SECTOR ERASE]\n");
	for ( i = 0; i < snor_mio_drv.sec_erase.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.sec_erase.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.sec_erase.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.sec_erase.offset + i)));
	}

	//-------------------------------------------
	// Block Erase (BE) cmd: 0xd8 / 64k-byte
	//-------------------------------------------
	snor_mio_drv.blk_erase.offset = SNOR_MIO_Get_Code_Offset(&snor_mio_drv.sec_erase);
	code_count = 0;

	if (snor_mio_drv.flags & WR_QPP)
	{
		cmd_io = IO_NUM_QUAD;
		addr_io= IO_NUM_QUAD;
	}
	else
	{
		cmd_io = IO_NUM_SINGLE;
		addr_io = IO_NUM_SINGLE;
	}

	if ( snor_mio_drv.flags & ADDR_4B)
	{
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_64K_4B, 1, LA_KEEP_CS, cmd_io);
		++code_count;
		MK_WRITE_DATA(code[code_count], 4, (snor_mio_drv.sfmc_addr.offset), addr_io);
	}
	else
	{
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, CMD_ERASE_64K, 1, LA_KEEP_CS, cmd_io);
	    ++code_count;
		//MK_WRITE_DATA_3B(code[code_count], 3, (snor_mio_drv.sfmc_addr.offset), addr_io);
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_DEASSERT_CS, addr_io);
	}
	++code_count;

	MK_STOP_CMD(code[code_count]);
	++code_count;
	snor_mio_drv.blk_erase.size = code_count;

	SNOR_MIO_CT_DEBUG("[BLOCK ERASE]\n");
	for ( i = 0; i < snor_mio_drv.blk_erase.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.blk_erase.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.blk_erase.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.blk_erase.offset + i)));
	}

	code_offset = snor_mio_drv.blk_erase.offset + snor_mio_drv.blk_erase.size;

	return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_write(uint32 code_offset)
{
	uint32	i;
	uint32	code_count;
	uint32	addr_io, cmd_io;
	uint32	write_io;
	uint32 	code[10];

	SNOR_MIO_TRACE();
	//-------------------------------------------
	// WRITE
	//-------------------------------------------
	snor_mio_drv.write.offset = code_offset;
	code_count = 0;

	cmd_io		= IO_NUM_SINGLE;
	addr_io 	= IO_NUM_SINGLE;
	write_io	= IO_NUM_SINGLE;

	if (snor_mio_drv.cmd_write == CMD_4PP4B)
	{
		cmd_io		= IO_NUM_SINGLE;
		addr_io		= IO_NUM_QUAD;
		write_io 	= IO_NUM_QUAD;
	}
	else if (snor_mio_drv.cmd_write == CMD_PP4B)
	{
		cmd_io 		= IO_NUM_SINGLE;
		addr_io 	= IO_NUM_SINGLE;
		write_io 	= IO_NUM_SINGLE;
	}

	MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv.cmd_write, 1, LA_KEEP_CS, cmd_io);
	++code_count;
	if (snor_mio_drv.flags & ADDR_4B)
	{
		MK_WRITE_PP_ADDR(code[code_count], 4, (snor_mio_drv.sfmc_addr.offset), addr_io);
	}
	else
	{
		//MK_WRITE_PP_ADDR_3B(code[code_count], 3, (snor_mio_drv.sfmc_addr.offset), addr_io);
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, 0x00, 3, LA_KEEP_CS, addr_io);
	}
	++code_count;
	MK_WRITE_DATA(code[code_count], SFMC_BUF_SIZE, (snor_mio_drv.sfmc_buf.offset), write_io);
	++code_count;

	MK_STOP_CMD(code[code_count]);
	++code_count;
	snor_mio_drv.write.size = code_count;

	SNOR_MIO_CT_DEBUG("[WRITE]\n");
	for ( i = 0; i < snor_mio_drv.write.size; ++i ){
		SET_CODE_TABLE((snor_mio_drv.write.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.write.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.write.offset + i)));
	}

	code_offset = snor_mio_drv.write.offset + snor_mio_drv.write.size;

	return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_read(uint32 code_offset)
{
	uint32	i;
	uint32	code_count;
	uint32	dummy_io;
	uint32 	code[10];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ
	//-------------------------------------------
	snor_mio_drv.read.offset = code_offset;

	code_count = 0;

	MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv.cmd_read, 1, LA_KEEP_CS, IO_NUM_SINGLE);
	++code_count;

	switch (snor_mio_drv.cmd_read)
	{
		case CMD_READ:
		case CMD_READ4B:
			dummy_io 	= IO_NUM_NC;
			break;

		case CMD_FAST_READ:
		case CMD_FAST_READ4B:
			dummy_io	= IO_NUM_SINGLE;
			break;

		default:
			dummy_io 	= IO_NUM_NC;
			break;
	}

	if (snor_mio_drv.flags & ADDR_4B)
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
	snor_mio_drv.read.size = code_count;

	SNOR_MIO_CT_DEBUG("[READ]\n");
	for ( i = 0; i < snor_mio_drv.read.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.read.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.read.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.read.offset + i)));
	}

	code_offset = snor_mio_drv.read.offset + snor_mio_drv.read.size;

	return code_offset;
}

static uint32 SNOR_MIO_Make_cmd_read_fast(uint32 code_offset)
{
	uint32	i;
	uint32	code_count;
	uint32	addr_io, read_io, dummy_io, cmd_io;
	uint32	cmd_count;
	uint32 	dummy_count;
	uint32	performance_enhance;
	uint32 	code[10];
	SNOR_MIO_TRACE();

	//-------------------------------------------
	// READ
	//-------------------------------------------
	snor_mio_drv.read_fast.offset = code_offset;

	// Set default
	snor_mio_drv.dt_mode = D_DTR_DISABLE;
	cmd_io = IO_NUM_SINGLE;
	dummy_count = 1;
	cmd_count = 1;
	performance_enhance = FALSE;

	switch (snor_mio_drv.cmd_read_fast)
	{
		case CMD_READ:
		case CMD_READ4B:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_SINGLE;
			dummy_io 	= IO_NUM_NC;
			break;

		case CMD_FAST_READ:
		case CMD_FAST_READ4B:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_SINGLE;
			dummy_io	= IO_NUM_SINGLE;
			break;

		case CMD_FASTDTRD_4B:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_SINGLE;
			dummy_io	= IO_NUM_SINGLE;
			snor_mio_drv.dt_mode = D_DTR_ENABLE;
			break;

		case CMD_DREAD:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_DUAL;
			dummy_io	= IO_NUM_SINGLE;
			break;

		case CMD_QREAD:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_QUAD;
			dummy_io	= IO_NUM_SINGLE;
			cmd_io 		= IO_NUM_SINGLE;
			break;

		case CMD_4DTRD:
		case CMD_4DTRD4B:
			addr_io 	= IO_NUM_QUAD;
			read_io 	= IO_NUM_QUAD;
			dummy_io	= IO_NUM_QUAD;
			cmd_io 		= IO_NUM_SINGLE;
			snor_mio_drv.dt_mode = D_DTR_ENABLE;
			dummy_count = 8;
			performance_enhance = TRUE;
			break;

		case CMD_4READ:
		case CMD_4READ4B:
			addr_io 	= IO_NUM_QUAD;
			read_io 	= IO_NUM_QUAD;
			dummy_io	= IO_NUM_QUAD;
			cmd_io		= IO_NUM_SINGLE;
			snor_mio_drv.dt_mode = D_DTR_DISABLE;
			dummy_count = 2;
			performance_enhance = TRUE;
			break;

		case CMD_QREAD4B:
			addr_io 	= IO_NUM_SINGLE;
			dummy_io	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_QUAD;
			cmd_io 		= IO_NUM_SINGLE;
			break;

		case CMD_8READ:
			addr_io 	= IO_NUM_OCTA;
			read_io 	= IO_NUM_OCTA;
			dummy_io	= IO_NUM_OCTA;
			cmd_io		= IO_NUM_OCTA;
			dummy_count = 0x14;
			cmd_count 	= 2;
			break;

		case CMD_8DTRD:
			addr_io 	= IO_NUM_OCTA;
			read_io 	= IO_NUM_OCTA;
			dummy_io	= IO_NUM_OCTA;
			cmd_io 		= IO_NUM_OCTA;
			dummy_count = 0x15;
			cmd_count 	= 2;
			snor_mio_drv.dt_mode = D_DTR_ENABLE;
			break;

		default:
			addr_io 	= IO_NUM_SINGLE;
			read_io 	= IO_NUM_SINGLE;
			dummy_io 	= IO_NUM_NC;
			break;
	}

	code_count = 0;

	if (cmd_io == IO_NUM_OCTA)
	{
		MK_WRITE_CMD(code[code_count], snor_mio_drv.dt_mode, snor_mio_drv.cmd_read_fast, cmd_count, LA_KEEP_CS, cmd_io);
	}
	else
	{
		MK_WRITE_CMD(code[code_count], D_DTR_DISABLE, snor_mio_drv.cmd_read_fast, cmd_count, LA_KEEP_CS, cmd_io);
	}
	++code_count;

	if (snor_mio_drv.flags & ADDR_4B)
	{
		MK_WRITE_ADDR_AHB(code[code_count], snor_mio_drv.dt_mode, addr_io);
	}
	else
	{
		MK_WRITE_ADDR_AHB_3B(code[code_count], snor_mio_drv.dt_mode, addr_io);
	}

	++code_count;

	if (performance_enhance == TRUE)
	{
		// performance enhance mode
		MK_WRITE_CMD(code[code_count], snor_mio_drv.dt_mode, 0x00, 1, LA_KEEP_CS, dummy_io);
		++code_count;
	}

	if (dummy_io != IO_NUM_NC)
	{
		MK_WRITE_DUMMY_CYCLE(code[code_count], snor_mio_drv.dt_mode, dummy_count, dummy_io);
		++code_count;
	}

	MK_READ_DATA_AHB(code[code_count], snor_mio_drv.dt_mode, read_io);
	++code_count;
	MK_STOP_CMD(code[code_count]);
	++code_count;
	snor_mio_drv.read_fast.size = code_count;

	SNOR_MIO_CT_DEBUG("[READ_FAST]\n");
	for ( i = 0; i < snor_mio_drv.read_fast.size; ++i )
	{
		SET_CODE_TABLE((snor_mio_drv.read_fast.offset + i), code[i]);

		SNOR_MIO_CT_DEBUG( "[0x%08X][0x%08X]\n",
									(SFMC_REG_CODE_TABLE + ((snor_mio_drv.read_fast.offset + i) <<2)),
									GET_CODE_TABLE((snor_mio_drv.read_fast.offset + i)));
	}

	code_offset = snor_mio_drv.read_fast.offset + snor_mio_drv.read_fast.size;

	return code_offset;
}

static void SNOR_MIO_Make_CMD(uint32 code_offset)
{
	SNOR_MIO_TRACE();
	code_offset = SNOR_MIO_Make_cmd_read(code_offset);
	code_offset = SNOR_MIO_Make_cmd_write(code_offset);
	code_offset = SNOR_MIO_Make_cmd_erase(code_offset);
	SNOR_MIO_Make_cmd_read_fast(code_offset);
}

sint32 SNOR_MIO_Init(void)
{
	uint32 code_offset;
//	uint32 ip_num, version;
	sint32 res = FALSE;
	uint8 memory_type;

	SNOR_MIO_TRACE();

	SNOR_MIO_PortConfig();

	/*
	 * PLL0_FOUT:     1200 Mhz
	 * PLL1_FOUT:     1500 Mhz
	 * PLL0_FOUT_DIV:  600 Mhz
	 * PLL1_FOUT_DIV:  750 Mhz
	 * XIN:             12 Mhz
	 * XIN_DIV:          6 Mhz
	 *
	 * SFMC Clock: 24Mhz (PLL0_FOUT_DIV / (24+1))
	 */
    //*(volatile uint32 *)(0xA0F24028) = 0xEA000018; //24Mhz
    //*(volatile uint32 *)(0xA0F24028) = 0xE000000E; //80Mhz

	//-------------------------------------------
	// Controller Setting
	//-------------------------------------------
	SNOR_MIO_DEBUG("SNOR Mode Selection\n");

	//SAL_TaskSleep(10);

	SET_CMD_RUN(SFMC_REG_RUN_SOFT_RESET|SFMC_REG_RUN_AUTO_STOP);

    *(volatile uint32 *)(SFMC_REG_MODE) = SFMC_REG_MODE_FLASH_RESET;

	SAL_TaskSleep(10);

	SET_CMD_RUN(~SFMC_REG_RUN_SOFT_RESET);

	*(volatile uint32 *)(SFMC_REG_TIMING)		= SFMC_REG_TIMING_SC_EXTND(0) |
													  SFMC_REG_TIMING_CS_TO_CS(4) |
													  SFMC_REG_TIMING_READ_LATENCY(3) |
													  SFMC_REG_TIMING_SEL_DQS_FCLK;
    *(volatile uint32 *)(SFMC_REG_DELAY_SO)	= SFMC_REG_DELAY_SO_INV_SCLK(1) |
													  SFMC_REG_DELAY_SO_SLCH(1);

    *(volatile uint32 *)(SFMC_REG_DELAY_CLK)	= 0;

	SNOR_MIO_DEBUG("SPI-STR mode Set Done\n");

	code_offset = SNOR_MIO_Make_cmd_readid();

	res = SNOR_MIO_CheckMemoryType();
	if (res == FALSE)
	{
		SNOR_MIO_DEBUG("Failed to read SNOR ID\n");
		return -1;
	}

	switch (snor_mio_drv.ManufID)
	{
		case 0xEF:
		case 0xC8: /* GigaDevice is the same as Winbond */
			code_offset = SNOR_MIO_Make_WINBOND_CMD(code_offset);
			break;
		case 0x01:
			code_offset = SNOR_MIO_Make_CYPRESS_CMD(code_offset);
			break;
		case 0x20:
			memory_type = (snor_mio_drv.DevID >> 8) & 0xFF;
			if (memory_type == 0x70) /* XMC */
				code_offset = SNOR_MIO_Make_XMC_CMD(code_offset);
			else /* MICRON */
				code_offset = SNOR_MIO_Make_MICRON_CMD(code_offset);
			break;
		case 0x9D:
			code_offset = SNOR_MIO_Make_ISSI_CMD(code_offset);
			break;
		default:
			code_offset = SNOR_MIO_Make_MXIC_CMD(code_offset);
			break;
	}
#if 0
	if ( snor_mio_drv.flags & ADDR_4B)
		SNOR_MIO_4B_Enable();
#endif
	SNOR_MIO_Make_CMD(code_offset);

	switch (snor_mio_drv.ManufID)
	{
		case 0xEF:
		case 0xC8:
			SNOR_MIO_WINBOND_DefaultConfig();
			break;
		case 0x01:
			SNOR_MIO_CYPRESS_DefaultConfig();
			break;
		case 0x20:
			memory_type = (snor_mio_drv.DevID >> 8) & 0xFF;
			if (memory_type == 0x70) /* XMC */
				SNOR_MIO_XMC_DefaultConfig();
			else /* MICRON */
				SNOR_MIO_MICRON_DefaultConfig();
			break;
		case 0x9D:
			SNOR_MIO_ISSI_DefaultConfig();
			break;
		default:
			SNOR_MIO_MXIC_DefaultConfig();
			break;
	}

	SAL_MemSet(snor_mio_drv.uiDataBuffer, 0xFF, SNOR_SECTOR_SIZE);
	return 0;
}

#if 0
void sfmc_test_SWRST_RC_TIME (uint32 warm_boot, uint32 reset_enable, uint32 reset_start, uint32 cmd_iteration, uint32 run_iteration)
{
	uint32	addr, data_temp, manu_run;
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
	uint32	nReg, data_temp, addr, manu_run;
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

static unsigned short SNOR_MIO_ReadConfig_2B(void)
{
	uint32	data;
	unsigned short 	config;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdcr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	config = ((data & 0xFFFF0000) >> 16);

	return config;
}

static uint8 SNOR_MIO_ReadConfig(void)
{
	uint32	data;
	uint8 	config;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdcr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	config = (data & 0xFF);
	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return config;
}

static uint8 SNOR_MIO_ReadConfig1(void)
{
	uint32	data;
	uint8 	config;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdcr1.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	config = (data & 0xFF);
	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return config;
}

static uint8 SNOR_MIO_ReadConfig2(void)
{
	uint32	data;
	uint8 	config;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdcr2.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	config = (data & 0xFF);
	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return config;
}

static uint8 SNOR_MIO_ReadStatus(void)
{
	uint32	data;
	uint8 	status;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdsr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	status = (data & 0xFF);

	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return status;
}

static uint8 SNOR_MIO_ReadStatus1(void)
{
	uint32	data;
	uint8 	status;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdsr1.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	status = (data & 0xFF);
	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return status;
}

static uint8 SNOR_MIO_ReadStatus2(void)
{
	uint32	data;
	uint8 	status;
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.rdsr2.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	data = (uint32)GET_CODE_TABLE(snor_mio_drv.sfmc_buf.offset);
	status = (data & 0xFF);
	//SNOR_MIO_DEBUG("status: 0x%08X\n", status);

	return status;
}

static sint32 SNOR_MIO_WaitWriteComplete(void)
{
	uint8 nStatus;
	uint32 retry_count;

	retry_count = 10000;
    while(1)
    {
        nStatus = (uint8)SNOR_MIO_ReadStatus();
        if (ISZERO(nStatus,Hw0)) //check WIP
            break;

		--retry_count;
		SAL_TaskSleep(1);
		if (retry_count == 0)
		{
			SNOR_MIO_DEBUG("\x1b[1;33m[%s:%d]\x1b[0m\n", __func__, __LINE__);
			return -1;
    }
}

	return 0;
}

static void SNOR_MIO_4B_Disable(void)
{
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.ex4b.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
}


static sint32 SNOR_MIO_WriteEnable(void)
{
    uint8 nStatus;
	uint32 retry_count;

    SNOR_MIO_TRACE();
	retry_count = 10000;
    while(1)
    {
        SET_CMD_MANU_ADDR(snor_mio_drv.write_enable.offset);
        SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
        WAIT_CMD_COMPLETE();

        nStatus = (uint8)SNOR_MIO_ReadStatus();
        if (!ISZERO(nStatus,Hw1)) //check WEL
            break;

		--retry_count;
		SAL_TaskSleep(1);
		if (retry_count == 0)
		{
			return -1;
		}
    }
	return 0;
}

static sint32 SNOR_MIO_ISSI_DefaultConfig(void)
{
	uint8 	status;
	uint8 	config;
	int		res;
	SNOR_MIO_TRACE();

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);

	if (ISZERO(status, Hw6))
	{
		SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");
		res = SNOR_MIO_WriteEnable();
		SET_CMD_MANU_ADDR(snor_mio_drv.wrsr.offset);
		SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
		WAIT_CMD_COMPLETE();
		res = SNOR_MIO_WaitWriteComplete();

		status = SNOR_MIO_ReadStatus();
		SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);
	}

	config = SNOR_MIO_ReadConfig();
	SNOR_MIO_DEBUG("Read AUTOBOOT Reg - Config: 0x%02X\n", config);
	if (!ISZERO(config, Hw0))
	{
		SNOR_MIO_DEBUG("Disable AUTOBOOT\n");
		SET_CMD_MANU_ADDR(snor_mio_drv.wrab.offset);
		SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
		WAIT_CMD_COMPLETE();
		res = SNOR_MIO_WaitWriteComplete();

		config = SNOR_MIO_ReadConfig();
		SNOR_MIO_DEBUG("Read AUTOBOOT Reg - Config: 0x%02X\n", config);
	}

	return res;
}


static sint32 SNOR_MIO_MICRON_DefaultConfig(void)
{
	uint8 status;
	unsigned short config;
	int		res;
	SNOR_MIO_TRACE();

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);

//	if (ISZERO(status,BIT(6)))
	{
		SNOR_MIO_DEBUG("Set to default status for FWDN\n");
		res = SNOR_MIO_WriteEnable();
		SET_CMD_MANU_ADDR(snor_mio_drv.wrsr.offset);
		SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
		WAIT_CMD_COMPLETE();
		res = SNOR_MIO_WaitWriteComplete();

		status = SNOR_MIO_ReadStatus();
		SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);
	}

	SNOR_MIO_DEBUG("Set to default configuration for FWDN\n");
	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.wrcr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	config = SNOR_MIO_ReadConfig_2B();
	SNOR_MIO_DEBUG("Read Config Reg - Config: 0x%04X\n", config);

    ( void ) config;
    ( void ) status;

	return res;
}

static sint32 SNOR_MIO_MXIC_DefaultConfig(void)
{
	uint8 	status;
	uint8 	config;
	int		res;
	SNOR_MIO_TRACE();

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);

	if(ISZERO(status,Hw6))
	{
		SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");
		res = SNOR_MIO_WriteEnable();
		SET_CMD_MANU_ADDR(snor_mio_drv.wrsr.offset);
		SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
		WAIT_CMD_COMPLETE();
		res = SNOR_MIO_WaitWriteComplete();

		status = SNOR_MIO_ReadStatus();
		SNOR_MIO_DEBUG("Read Status Reg - status: 0x%02X\n", status);
	}

	config = SNOR_MIO_ReadConfig();
	SNOR_MIO_DEBUG("Read Config Reg - Config: 0x%02X\n", config);

    ( void ) config;

	return res;
}

static sint32 SNOR_MIO_WINBOND_WRSR1_Default(void)
{
	int		res;
	SNOR_MIO_TRACE();

	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.wrsr1.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	return res;
}

static sint32 SNOR_MIO_WINBOND_WRSR2_Default(void)
{
	int		res;
	SNOR_MIO_TRACE();

	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.wrsr2.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	return res;
}

static sint32 SNOR_MIO_WINBOND_DefaultConfig(void)
{
	uint8 	status;
	int		res;
	SNOR_MIO_TRACE();

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus1();
	SNOR_MIO_DEBUG("Read Status1 Reg(35h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus2();
	SNOR_MIO_DEBUG("Read Status2 Reg(15h) - status: 0x%02X\n", status);

	res = SNOR_MIO_WINBOND_WRSR1_Default();
	SNOR_MIO_DEBUG("Set QE (Quad Enable) bit\n");
	res |= SNOR_MIO_WINBOND_WRSR2_Default();
	SNOR_MIO_DEBUG("Clear WPS (Write Protect Selection) bit\n");

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus1();
	SNOR_MIO_DEBUG("Read Status1 Reg(35h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus2();
	SNOR_MIO_DEBUG("Read Status2 Reg(15h) - status: 0x%02X\n", status);

    ( void ) status;

	return res;
}

static sint32 SNOR_MIO_CYPRESS_DefaultConfig(void)
{
	uint8 	status;
	uint8 	config;
	int		res;
	SNOR_MIO_TRACE();

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status-1 Reg - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadConfig();
	SNOR_MIO_DEBUG("Read Config-1 Reg - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadConfig1();
	SNOR_MIO_DEBUG("Read Config-2 Reg - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadConfig2();
	SNOR_MIO_DEBUG("Read Config-3 Reg - status: 0x%02X\n", status);

	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.wrsr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();
	SNOR_MIO_DEBUG("Default set for TCC\n");

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status-1 Reg - status: 0x%02X\n", status);
	config = SNOR_MIO_ReadConfig();
	SNOR_MIO_DEBUG("Read Config-1 Reg - Config: 0x%02X\n", config);
	status = SNOR_MIO_ReadConfig1();
	SNOR_MIO_DEBUG("Read Config-2 Reg - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadConfig2();
	SNOR_MIO_DEBUG("Read Config-3 Reg - status: 0x%02X\n", status);

    ( void ) config;
    ( void ) status;

	return res;
}

static sint32 SNOR_MIO_XMC_DefaultConfig(void)
{
	uint8 	status;
	int		res;
	SNOR_MIO_TRACE();

	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.wrsr.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	SET_CMD_MANU_ADDR(snor_mio_drv.wrsr2.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	SNOR_MIO_DEBUG("Default set for TCC\n");

	status = SNOR_MIO_ReadStatus();
	SNOR_MIO_DEBUG("Read Status Reg(05h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus1();
	SNOR_MIO_DEBUG("Read Status2 Reg(09h) - status: 0x%02X\n", status);
	status = SNOR_MIO_ReadStatus2();
	SNOR_MIO_DEBUG("Read Status3 Reg(95h) - status: 0x%02X\n", status);

    ( void ) status;

	return res;
}

static void SNOR_MIO_WriteDisable(void)
{
	SNOR_MIO_TRACE();
	SET_CMD_MANU_ADDR(snor_mio_drv.write_disable.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
}

static sint32 SNOR_MIO_ChipErase(void)
{
	sint32 res;

	SNOR_MIO_TRACE();
	SNOR_MIO_DEBUG("%s\n", __func__);

	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.chip_erase.offset);
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	while (1)
	{
		res = SNOR_MIO_WaitWriteComplete();
		if (!res)
			break;
	}

	return res;
}

static sint32 SNOR_MIO_BlockErase(uint32 address)
{
	sint32 res;

	SNOR_MIO_TRACE();
	SNOR_MIO_DEBUG("blk_erase: 0x%08X\n", address);
	//SNOR_MIO_WriteEAR(address);
	//address = (address & 0x00FFFFFF);
	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.blk_erase.offset);
	if (snor_mio_drv.flags & ADDR_4B)
	{
		//address |= ( CMD_ERASE_64K<< 24) & 0xFF000000;
		SET_CODE_TABLE(snor_mio_drv.sfmc_addr.offset, address);
	}
	else
	{
		SET_CODE_TABLE_ERASE_ADDR(snor_mio_drv.blk_erase.offset, address);
	}
	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();
	return res;
}

static sint32 SNOR_MIO_SectorErase(uint32 address)
{
	sint32 res;

	SNOR_MIO_TRACE();
	//SNOR_MIO_DEBUG("sec_erase: 0x%08X\n", address);
	//SNOR_MIO_WriteEAR(address);
	//address = (address & 0x00FFFFFF);
	res = SNOR_MIO_WriteEnable();
	SET_CMD_MANU_ADDR(snor_mio_drv.sec_erase.offset);

	if (snor_mio_drv.flags & ADDR_4B)
	{
		//address |= (snor_mio_drv.erase_cmd << 24) & 0xFF000000;
		SET_CODE_TABLE(snor_mio_drv.sfmc_addr.offset, address);
	}
	else
	{
		SET_CODE_TABLE_ERASE_ADDR(snor_mio_drv.sec_erase.offset, address);
	}

	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();
	return res;
}

void SNOR_MIO_Erase(uint32 address, uint32 size)
{
	uint32 lba = 0;
	uint32 nPercent, nPercent_unit;

	uint32 count_4k = 0;
	SNOR_MIO_TRACE();

	//SNOR_MIO_DEBUG("[SNOR Erase] addr: 0x%08x, size: %8d kbyte     ", address, size>>10);


	if (address & 0xFFF)
	{
		//Address Error
		return;
	}

	if ((address & 0xFFFF) && (size > (64 * 1024)))
	{
		count_4k = (64 * 1024) - (address & 0xFFFF);

		for(lba = 0;lba < count_4k; lba += 4*1024)
		{
			//SNOR_MIO_DEBUG("Sector Erase : 0x%08x\n\r", address + lba);
			SNOR_MIO_SectorErase(address + lba);
		}

		address += count_4k;
		size -= count_4k;
	}

	nPercent = 0;
	nPercent_unit = ( size / (64*1024));
	for (lba = 0;lba < nPercent_unit; lba++)
	{
		//if (lba && !(lba%41))
		//  SNOR_MIO_DEBUG("\r\n");
		if (nPercent<99)
		{
			//  nPercent += 100/nPercent_unit;
			nPercent = (100*lba)/nPercent_unit;
			SNOR_MIO_DEBUG("\b\b\b\b%3d%%", nPercent);
		}

		//SNOR_MIO_DEBUG("Sector Erase: 0x%08x, nPercent_unit: %d, nPercent: %d\n", address, nP     ercent_unit, nPercent);
		SNOR_MIO_BlockErase(address);
		address += 64*1024;
		size -= 64*1024;
	}

	if (size > 0)
	{
		nPercent_unit = size % (4*1024);
		if (nPercent_unit) nPercent_unit = size / (4*1024) + 1;
		else nPercent_unit = size / (4*1024);

		for (lba = 0;lba < nPercent_unit; lba ++)
		{
			//SNOR_MIO_DEBUG("Sector Erase : 0x%08x\n", address);
			SNOR_MIO_SectorErase(address);
			address += 4*1024;
			size -= 4*1024;
		}
	}

	SNOR_MIO_DEBUG("\b\b\b\b%3d%%", 100);
	SNOR_MIO_DEBUG(" OK.\r\n");
}

static sint32 SNOR_MIO_WriteByte(uint32 address, uint8 *pBuffer, uint32 lengthInByte)
{
	uint32 sfmc_buff_addr;
	uint32 write_address;
	uint32 remain[SNOR_PAGE_SIZE];
	int	res = FALSE;
	SNOR_MIO_TRACE();
	SNOR_MIO_DEBUG("[%s]address: 0x%08X, buffer: 0x%08X, length: %d\n", __func__, address, (uint32)pBuffer, lengthInByte);

	if (address&0xFF)
	{
		SNOR_MIO_DEBUG("Address alignment error.\n");
		SNOR_MIO_DEBUG("Set the alignment of the address in %d bytes.\n", SNOR_PAGE_SIZE);
		return res;
	}

	while(lengthInByte > SNOR_PAGE_SIZE)
	{
		write_address = address;

		res = SNOR_MIO_WriteEnable();

		SET_CMD_MANU_ADDR(snor_mio_drv.write.offset);

		sfmc_buff_addr = GET_CODE_TABLE_ADDR(snor_mio_drv.sfmc_buf.offset);
		SAL_MemSet((void*)sfmc_buff_addr, 0xFF, SNOR_PAGE_SIZE);
		SAL_MemCopy((void*)sfmc_buff_addr, (const void*)pBuffer, SNOR_PAGE_SIZE);

		if (snor_mio_drv.flags & ADDR_4B)
		{
			SET_CODE_TABLE(snor_mio_drv.sfmc_addr.offset, write_address);
		}
		else
		{
			SET_CODE_TABLE_WRITE_ADDR(snor_mio_drv.write.offset, write_address);
		}

		SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
		WAIT_CMD_COMPLETE();
		res = SNOR_MIO_WaitWriteComplete();

		pBuffer	+= SNOR_PAGE_SIZE;
		address += SNOR_PAGE_SIZE;
		lengthInByte -= SNOR_PAGE_SIZE;
	}

	/* Remain written data */
	SAL_MemSet((void*)remain, 0xFF, SNOR_PAGE_SIZE);
	SAL_MemCopy((void*)remain, (uint32 *)pBuffer, lengthInByte);
	write_address = address;

	res = SNOR_MIO_WriteEnable();

	SET_CMD_MANU_ADDR(snor_mio_drv.write.offset);

	sfmc_buff_addr = GET_CODE_TABLE_ADDR(snor_mio_drv.sfmc_buf.offset);
	SAL_MemCopy((void*)sfmc_buff_addr, (void*)remain, SNOR_PAGE_SIZE);

	if (snor_mio_drv.flags & ADDR_4B)
	{
		SET_CODE_TABLE(snor_mio_drv.sfmc_addr.offset, write_address);
	}
	else
	{
		SET_CODE_TABLE_WRITE_ADDR(snor_mio_drv.write.offset, write_address);
	}

	SET_CMD_RUN(SFMC_REG_RUN_MAN_RUN);
	WAIT_CMD_COMPLETE();
	res = SNOR_MIO_WaitWriteComplete();

	return res;
}

sint32 SNOR_MIO_Write(uint32 address, const void *pBuffer, uint32 length)
{
	SNOR_MIO_TRACE();
	SNOR_MIO_DEBUG("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", address, (uint32)pBuffer, length);

	SNOR_MIO_WriteByte(address,(uint8*)pBuffer,length);

	return 0;
}

static void SNOR_MIO_Read_AHB(uint32 address, uint8 *pBuffer, uint32 length)
{
#if 0 //Not supported yet
	uint32 i;
	SNOR_MIO_TRACE();
	//SNOR_MIO_DEBUG("Read address: 0x%08X, buffer: 0x%08X, length: %d\n", address, (uint32)pBuffer, length);

#ifdef SNOR_MIO_GDMA
	HwGDMA0->uSADDR0 	= (uint32)(NOR_FLASH_BASE_ADDR + address) ;
	HwGDMA0->uDADDR0	= (uint32)pBuffer;
	HwGDMA0->uSPARAM0.nSPARAM	= 4;
	HwGDMA0->uDPARAM0.nDPARAM 	= 4;
	HwGDMA0->uHCOUNT0   = (length/32) ;
	HwGDMA0->uCHCTRL0.nCHCTRL =
		((0x1)<<13)	| 	// SYNC
		((0x1)<<12) | 	// HRD
		((0x2)<<8) 	|  	// TYPE	- SW Transfer
		((0x3)<<6) 	|  	// BSIZE - 1/2/4/8 Burst
		((0x2)<<4) 	|  	// WSIZE - byte/half/WORD
		((0x1)<<3) 	|  	// FLAG
		((0x1)<<0) 	|  	// EN
		(0);

	while (!HwGDMA0->uCHCTRL0.bCHCTRL.FLG);
#else
	SAL_MemCopy((void*)pBuffer, (void *)(NOR_FLASH_BASE_ADDR + address), length);
#endif /* 0 */
#endif
}

static void SNOR_MIO_ReadByte_fast(uint32 address, uint8 *pBuffer, uint32 lengthInByte)
{
	SNOR_MIO_TRACE();
//	SNOR_MIO_DEBUG("address: 0x%08X, buffer: 0x%08X, length: %d\n", address, (uint32)pBuffer, lengthInByte);

	SET_CMD_AUTO_ADDR(snor_mio_drv.read_fast.offset);

	SET_CMD_RUN(SFMC_REG_RUN_AUTO_RUN);

//#ifdef SNOR_MIO_GDMA
//	arch_invalidate_cache_range((addr_t)pBuffer, lengthInByte);
//#endif

#ifdef SNOR_MIO_GDMA
	while(lengthInByte)
	{
		uint32 nCountInPage;
		nCountInPage = 256 - (address&0xFF);
		if (nCountInPage>lengthInByte)
			nCountInPage = lengthInByte;

		SNOR_MIO_Read_AHB(address, pBuffer, nCountInPage);

		pBuffer += nCountInPage;
		address += nCountInPage;
		lengthInByte -= nCountInPage;
	}

#else
	SAL_MemCopy((void*)pBuffer, (void *)(NOR_FLASH_BASE_ADDR + address), lengthInByte);
#endif
	SET_CMD_RUN(SFMC_REG_RUN_AUTO_STOP);
}


static void SNOR_MIO_ReadByte(uint32 address, uint8 *pBuffer, uint32 lengthInByte)
{
	SNOR_MIO_TRACE();
	//	SNOR_MIO_DEBUG("[%s]address: 0x%08X, buffer: 0x%08X, length: %d\n", __func__, address, (uint32)pBuffer, lengthInByte);

	SET_CMD_AUTO_ADDR(snor_mio_drv.read.offset);

	SET_CMD_RUN(SFMC_REG_RUN_AUTO_RUN);

//#ifdef SNOR_MIO_GDMA
//	arch_invalidate_cache_range((addr_t)pBuffer, lengthInByte);
//#endif

#ifdef SNOR_MIO_GDMA
	while(lengthInByte)
	{
		uint32 nCountInPage;
		nCountInPage = 256 - (address&0xFF);
		if (nCountInPage>lengthInByte)
			nCountInPage = lengthInByte;

		SNOR_MIO_Read_AHB(address, pBuffer, nCountInPage);

		pBuffer += nCountInPage;
		address += nCountInPage;
		lengthInByte -= nCountInPage;
	}

#else
	SAL_MemCopy((void*)pBuffer, (void *)(NOR_FLASH_BASE_ADDR + address), lengthInByte);
#endif
	SET_CMD_RUN(SFMC_REG_RUN_AUTO_STOP);
}

sint32 SNOR_MIO_Read_Fast(uint32 address, void *pBuffer, uint32 length)
{
	SNOR_MIO_TRACE();

	SNOR_MIO_ReadByte_fast(address, (uint8*)pBuffer, length);

	return 0;
}

sint32 SNOR_MIO_Read(uint32 address, void *pBuffer, uint32 length)
{
	SNOR_MIO_TRACE();

	SNOR_MIO_ReadByte(address, (uint8*)pBuffer, length);

	return 0;
}

sint32 SNOR_MIO_FWDN_LowFormat(void)
{
	sint32 res;

    res = SNOR_MIO_ChipErase();

	return res;
}

sint32 SNOR_MIO_FWDN_Read(uint32 address, uint32 length, void *buff)
{
	if (snor_mio_drv.size < (address + length))
	{
		SNOR_MIO_DEBUG("[%s]Exceed Size\n", __func__);
	}

	SNOR_MIO_Read(address, buff, length);

	return 0;
}

sint32 SNOR_MIO_FWDN_Write(uint32 address, uint32 length, void *buf)
{
	uint8 *data;
	uint32 i, size;
	sint32 res;
	uint32 total_sec_cnt;

	SNOR_MIO_DEBUG("%s[%d]: address=0x%X, length=0x%X, buf=0x%p\n", __func__, __LINE__, address, length, buf);
	if (snor_mio_drv.size < (address + length))
	{
		SNOR_MIO_DEBUG("[%s]Exceed Size\n", __func__);
		return -1;
	}

	data = (uint8  *)buf;
	SAL_MemSet(snor_mio_drv.uiDataBuffer, 0xFF, SNOR_SECTOR_SIZE);


	if (address % SNOR_SECTOR_SIZE)
	{
		/* Sector Read */
		SNOR_MIO_Read((address & ~(0xFFF)), snor_mio_drv.uiDataBuffer, SNOR_SECTOR_SIZE);
		i = (address & 0xFFF);
		if (length > (SNOR_SECTOR_SIZE - i))
		{
			size = (length - (SNOR_SECTOR_SIZE - i));
		}
		else
		{
			size = length;
		}

		SNOR_MIO_DEBUG("Erase & Write less than 4KB of data, address: 0x%X\n", address);
		SAL_MemCmp((const void *)(snor_mio_drv.uiDataBuffer + i), (const void *)data, size, &res);
		if (res != 0)
		{
			/* Data Merge */
			SAL_MemCopy(&snor_mio_drv.uiDataBuffer[i], data, size);
			SNOR_MIO_SectorErase((address & ~(0xFFF)));
			SNOR_MIO_WriteByte((address & ~(0xFFF)), snor_mio_drv.uiDataBuffer, SNOR_SECTOR_SIZE);
		}

		address += size;
		data += size;
		length -= size;
	}

	/* Write data in 4KBytes */
	total_sec_cnt = (length / SNOR_SECTOR_SIZE);
	SNOR_MIO_DEBUG("Erase & Write data in 4KBytes, address: 0x%X, total_sec_cnt=%d\n", address, total_sec_cnt);
	for (i = 0; i < total_sec_cnt; i++)
	{
		SNOR_MIO_Read(address, snor_mio_drv.uiDataBuffer, SNOR_SECTOR_SIZE);

		SAL_MemCmp((const void *)snor_mio_drv.uiDataBuffer, (const void *)data, SNOR_SECTOR_SIZE, &res);
		if (res != 0)
		{
			SNOR_MIO_SectorErase(address);
			SNOR_MIO_WriteByte(address, data, SNOR_SECTOR_SIZE);
		}

		address += SNOR_SECTOR_SIZE;
		data += SNOR_SECTOR_SIZE;
		length -= SNOR_SECTOR_SIZE;
	}

	/* Remaining data */
	if (length > 0)
	{
		SNOR_MIO_DEBUG("Erase & Write remaining data, address: 0x%X, length=%d\n", address, length);
		SNOR_MIO_Read(address, snor_mio_drv.uiDataBuffer, SNOR_SECTOR_SIZE);

		SAL_MemCmp((const void *)snor_mio_drv.uiDataBuffer, (const void *)data, length, &res);
		if (res != 0)
		{
			SAL_MemCopy(snor_mio_drv.uiDataBuffer, data, length);
			SNOR_MIO_SectorErase(address);
			SNOR_MIO_WriteByte(address, snor_mio_drv.uiDataBuffer, SNOR_SECTOR_SIZE);
		}
	}

	return 0;
}

sint32 SNOR_MIO_AutoRead(uint8 mode)
{
    if(mode == 1)
    {
	    SET_CMD_AUTO_ADDR(snor_mio_drv.read.offset);
	    SET_CMD_RUN(SFMC_REG_RUN_AUTO_RUN);
    }
    else
    {
        SET_CMD_RUN(SFMC_REG_RUN_AUTO_STOP);
    }

	return 0;
}

