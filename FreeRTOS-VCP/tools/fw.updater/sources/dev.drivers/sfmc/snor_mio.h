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

#ifndef _SNOR_MIO_H
#define _SNOR_MIO_H

#define NOR_FLASH_BASE_ADDR			(0x40000000)
#define SFMC_BASE_ADDR          	(0xA0F00000)

#define SFMC_REG_MAGIC          	(SFMC_BASE_ADDR+0x0000)
#define SFMC_REG_VERSION        	(SFMC_BASE_ADDR+0x0004)
#define SFMC_REG_PARAM          	(SFMC_BASE_ADDR+0x0008)
#define SFMC_REG_STATUS         	(SFMC_BASE_ADDR+0x000C)
#define SFMC_REG_RUN            	(SFMC_BASE_ADDR+0x0010)
#define SFMC_REG_INT_PEND       	(SFMC_BASE_ADDR+0x0014)
#define SFMC_REG_INT_ENB        	(SFMC_BASE_ADDR+0x0018)
#define SFMC_REG_BADDR_MANU     	(SFMC_BASE_ADDR+0x001C)
#define SFMC_REG_BADDR_AUTO     	(SFMC_BASE_ADDR+0x0020)
#define SFMC_REG_MODE           	(SFMC_BASE_ADDR+0x0024)
#define SFMC_REG_TIMING         	(SFMC_BASE_ADDR+0x0028)
#define SFMC_REG_DELAY_SO       	(SFMC_BASE_ADDR+0x002C)
#define SFMC_REG_DELAY_CLK      	(SFMC_BASE_ADDR+0x0030)
#define SFMC_REG_DELAY_WBD0     	(SFMC_BASE_ADDR+0x0034)
#define SFMC_REG_DELAY_WBD1     	(SFMC_BASE_ADDR+0x0038)
#define SFMC_REG_DELAY_RBD0     	(SFMC_BASE_ADDR+0x003C)
#define SFMC_REG_DELAY_RBD1     	(SFMC_BASE_ADDR+0x0040)
#define SFMC_REG_DELAY_WOEBD0   	(SFMC_BASE_ADDR+0x0044)
#define SFMC_REG_DELAY_WOEBD1   	(SFMC_BASE_ADDR+0x0048)
#define SFMC_REG_DELAY_TIMEOUT  	(SFMC_BASE_ADDR+0x004C)
#define SFMC_REG_CODE_TABLE     	(SFMC_BASE_ADDR+0x0800)

#define SFMC_REG_CODE_VERINFO   	(SFMC_BASE_ADDR+0x09E0)
#define SFMC_REG_CODE_VERINFO_SIZE	(0x1F)


#define SFMC_REG_RUN_MAN_IDLE				( 0x0 << 0)
#define SFMC_REG_RUN_MAN_RUN				( 0x1 << 0)
#define SFMC_REG_RUN_MAN_STOP				( 0x3 << 0)

#define SFMC_REG_RUN_AUTO_IDLE				( 0x0 << 4)
#define SFMC_REG_RUN_AUTO_RUN				( 0x1 << 4)
#define SFMC_REG_RUN_AUTO_STOP				( 0x3 << 4)

#define SFMC_REG_RUN_SOFT_RESET				( 0x1 << 8)

#define SFMC_REG_INT_ENB_USER_0 			(	1 << 0)
#define SFMC_REG_INT_ENB_USER_1 			(	1 << 1)
#define SFMC_REG_INT_ENB_USER_2 			(	1 << 2)
#define SFMC_REG_INT_ENB_USER_3 			(	1 << 3)

#define SFMC_REG_INT_ENB_CORE_ERR_EN 		(	1 << 4)
#define SFMC_REG_INT_ENB_TIMEOUT_EN			(	1 << 5)

#define SFMC_REG_MODE_FLASH_RESET			(	1 << 0)
#define SFMC_REG_MODE_MODE_DQUAD			(	1 << 1)	// Dual Quad mode enable (Active High)
#define SFMC_REG_MODE_FIFO_CTRL_EN			(	1 << 2) // Enable DQS Clock
#define SFMC_REG_MODE_FIFO_CTRL_DIS 		(	0)
#define SFMC_REG_MODE_SERIAL_0				( 0x1 << 4) // Serial Mode 0
#define SFMC_REG_MODE_SERIAL_3				( 0x2 << 4) // Serial Mode 3
#define SFMC_REG_MODE_CROSS_WR				(	1 << 6) // Write Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_CROSS_RD				(	1 << 7) // Read Data Cross Enable (Octa DTR Mode)
#define SFMC_REG_MODE_SIO_OFF_QUAD			( 0xF0<< 8) // Quad Memory
#define SFMC_REG_MODE_SIO_OFF_OCTA			( 0x00<< 8) // Octa or Dual Quad Memory

#define SFMC_REG_TIMING_SEL_P_180			(	1 << 0)	// Select rd_p_180 data
#define SFMC_REG_TIMING_SEL_N_180			(	1 << 1) // Select rd_n_180 data
#define SFMC_REG_TIMING_SEL_PN				(	1 << 2) // Secect p data (must 1 at DTR mode)
#define SFMC_REG_TIMING_SEL_PN_DT(x)		( (x) << 2) // Secect p data (must 1 at DTR mode)

#define SFMC_REG_TIMING_SEL_DQS_FCLK 		(0x0 << 4) // fclk
#define SFMC_REG_TIMING_SEL_DQS_FCLK_PAD	(0x1 << 4) // fclk pad input
#define SFMC_REG_TIMING_SEL_DQS_I_DQS_OCTA	(0x2 << 4) // i_dqs octa only
#define SFMC_REG_TIMING_SEL_DQS_N_FCLK 		(0x3 << 4) // ~fclk
#define SFMC_REG_TIMING_SEL_DQS(x) 			((x) << 4) // ~fclk

#define SFMC_REG_TIMING_READ_LATENCY(x)		(((x) << 8) & 0x00000F00)
#define SFMC_REG_TIMING_CS_TO_CS(x)			(((x) << 16) & 0x000F0000)
#define SFMC_REG_TIMING_SC_EXTND(x)			(((x) << 20) & 0x00300000)

#define SFMC_REG_DELAY_SO_SLDH(x)			(((x) << 0) & 0x000000FF)	// CS Low to SO Latency (0 ~7)
#define SFMC_REG_DELAY_SO_SLCH(x)			(((x) << 8) & 0x00000300)	// CS Low to SCLK Latency (0 ~2)
#define SFMC_REG_DELAY_SO_INV_SCLK(x)		((x) << 10)	// Invert SCLK


#define SFMC_REG_DELAY_CLK_WD(x)			(((x) <<  0) & 0x000000FF)	//SCLK Clock Delay (0xE7) [7:6] Delay ctrl, [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_BUF(x)		(((x) <<  0) & 0x0000003F)	//SCLK Clock Delay [5:0] clk_buf sel
#define SFMC_REG_DELAY_CLK_WD_CTRL(x)		(((x) <<  6) & 0x000000C0)	//SCLK Clock Delay [7:6] Delay ctrl


#define SFMC_REG_DELAY_CLK_RD(x)			(((x) <<  8) & 0x0000FF00)	//Read Clock Delay 	[15:14] Delay ctrl, [13:8] clk buf sel

#define SFMC_REG_DELAY_CLK_RD_BUF(x)		(((x) <<  8) & 0x00003F00)	//Read Clock Delay [15:14] clk_buf sel
#define SFMC_REG_DELAY_CLK_RD_CTRL(x)		(((x) << 14) & 0x0000C000)	//Read Clock Delay [13:8] Delay ctrl

#define SFMC_REG_DELAY_CLK_CSN_BD(x)		(((x) << 16) & 0x001F0000)	//CSN Signal Delay
#define SFMC_REG_DELAY_CLK_WR_TAB			(	1 << 21) // CLK_WD tab_delay chg
#define SFMC_REG_DELAY_CLK_RD_TAB			(	1 << 22) // CLK_RD tab_delay chg


#define SFMC_REG_DELAY_WBD0_SO_0(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_1(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_2(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Write bit delay
#define SFMC_REG_DELAY_WBD0_SO_3(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Write bit delay

#define SFMC_REG_DELAY_WBD1_SO_4(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_5(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_6(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Write bit delay
#define SFMC_REG_DELAY_WBD1_SO_7(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Write bit delay


#define SFMC_REG_DELAY_RBD0_SO_0(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_1(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_2(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Read bit delay
#define SFMC_REG_DELAY_RBD0_SO_3(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Read bit delay

#define SFMC_REG_DELAY_RBD1_SO_4(x)			(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_5(x)			(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_6(x)			(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Read bit delay
#define SFMC_REG_DELAY_RBD1_SO_7(x)			(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Read bit delay


#define SFMC_REG_DELAY_WOEBD0_SO_0(x)		(((x) <<  0) & 0x000000FF)	// Flash Memory SO[0] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_1(x)		(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[1] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_2(x)		(((x) << 16) & 0x00FF0000)	// Flash Memory SO[2] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD0_SO_3(x)		(((x) << 24) & 0xFF000000)	// Flash Memory SO[3] Write Output Enable bit delay

#define SFMC_REG_DELAY_WOEBD1_SO_4(x)		(((x) <<  0) & 0x000000FF)	// Flash Memory SO[4] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_5(x)		(((x) <<  8) & 0x0000FF00)	// Flash Memory SO[5] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_6(x)		(((x) << 16) & 0x00FF0000)	// Flash Memory SO[6] Write Output Enable bit delay
#define SFMC_REG_DELAY_WOEBD1_SO_7(x)		(((x) << 24) & 0xFF000000)	// Flash Memory SO[7] Write Output Enable bit delay


#define SFMC_REG_DELAY_TIMEOUT_CNT(x)		(((x) << 0) & 0x0000FFFF)	// Timeout Counter [15:0]
#define SFMC_REG_DELAY_TIMEOUT_ENB			(	1 << 16)				// Timeout Enable


#define CMD0_READ				0x0
#define CMD0_WRITE				0x1
#define CMD0_WRITE_BYTE			0x2
#define CMD0_OPERATION			0x3

#define CMD1_OPERATION			0x0
#define CMD1_COMPARE			0x1
#define CMD1_BR_INT				0x2
#define CMD1_WAIT_STOP			0x3

#define CMD2_BRANCH				0x0
#define CMD2_INT				0x1
#define CMD2_WAIT				0x0
#define CMD2_STOP				0x1

#define LA_KEEP_CS				0x0
#define LA_DEASSERT_CS			0x1

#define D_DTR_DISABLE			0x0	// Double Transfer Rate(DTR) mode disable
#define D_DTR_ENABLE			0x1

#define AR_TABLE_WRITE			0x0
#define AR_AHB_WRITE			0x1

#define AA_TABLE_ADDR			0x0
#define AA_AHB_ADDR				0x1

#define DU_REAL_DATA			0x0
#define DU_DUMMY_DATA			0x1

#define IO_NUM_NC				0xF
#define IO_NUM_SINGLE			0x0
#define IO_NUM_DUAL				0x1
#define IO_NUM_QUAD				0x2
#define IO_NUM_OCTA				0x3

#define MK_WRITE_CMD(CODE, DTR, DATA, SIZE, LA, IO_NUM)	{\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA 				<< 29)	\
						| (DTR			 	<< 28) 	\
						| (SIZE				<< 26)	\
						| (IO_NUM			<< 24)	\
						| (DATA				<<  0));\
}

#define MK_WRITE_ADDR_AHB(CODE, DTR, IO_NUM) {		\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (DTR 				<< 28) 	\
						| (AA_AHB_ADDR		<< 27)	\
						| (DU_REAL_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_WRITE_ADDR_AHB_3B(CODE, DTR, IO_NUM) {		\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (DTR 				<< 28) 	\
						| (AA_AHB_ADDR		<< 27)	\
						| (DU_REAL_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (1				<<  0));\
}

#define MK_WRITE_ADDR_TABLE(CODE, ADDR, SIZE, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (D_DTR_DISABLE 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA	<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_ADDR(CODE, ADDR, SIZE, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE_BYTE 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA		<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_DUMMY_CYCLE(CODE, DTR, SIZE, IO_NUM) {	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (DTR			 	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (DU_DUMMY_DATA 	<< 26)	\
						| (IO_NUM			<< 24)	\
						| (SIZE				<< 12));\
}

#define MK_READ_DATA_AHB(CODE, DTR, IO_NUM)	{			\
				(CODE) = ((CMD0_READ	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (DTR				<< 28) 	\
						| (AR_AHB_WRITE		<< 27)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_READ_DATA(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_READ	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AR_TABLE_WRITE	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_DATA_AHB(CODE, IO_NUM)	{			\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_AHB_ADDR		<< 27)	\
						| (IO_NUM			<< 24)	\
						| (0				<<  0));\
}

#define MK_WRITE_DATA(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_DEASSERT_CS		<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_DATA_3B(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_DEASSERT_CS	<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  1)	\
						| (1				<<  0));\
}


#define MK_WRITE_PP_ADDR(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR				<<  0));\
}

#define MK_WRITE_PP_ADDR_3B(CODE, SIZE, ADDR, IO_NUM)	{	\
				(CODE) = ((CMD0_WRITE	 	<< 30)	\
						| (LA_KEEP_CS		<< 29)	\
						| (D_DTR_DISABLE	<< 28) 	\
						| (AA_TABLE_ADDR	<< 27)	\
						| (IO_NUM			<< 24)	\
						| (SIZE 			<< 12)	\
						| (ADDR 			<<	1)	\
						| (1				<<	0));\
}



#define MK_WAIT_CMD(CODE,DELAY) { 					\
				(CODE) = ((CMD0_OPERATION	<< 30)	\
						| (CMD1_WAIT_STOP	<< 28)	\
						| (CMD2_WAIT		<< 26)	\
						| (DELAY			<<  0));\
}

#define MK_STOP_CMD(CODE) { 						\
				(CODE) = ((CMD0_OPERATION	<< 30)	\
						| (CMD1_WAIT_STOP	<< 28)	\
						| (CMD2_STOP		<< 26)	\
						| (0				<<  0));\
}

#define SET_CODE_TABLE(OFFSET, VAL) 		\
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + (OFFSET<<2)) = VAL)

#define GET_CODE_TABLE(OFFSET) 		\
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + (OFFSET<<2)))

#define GET_CODE_TABLE_ADDR(OFFSET) 		\
	(SFMC_REG_CODE_TABLE + (OFFSET<<2));	\


#define SET_CMD_AUTO_ADDR(OFFSET) 		\
	(*(volatile uint32 *)(SFMC_REG_BADDR_AUTO) = ((0x800)+(OFFSET<<2)));	\


#define SET_CMD_MANU_ADDR(OFFSET) 		\
	(*(volatile uint32 *)(SFMC_REG_BADDR_MANU) = ((0x800)+(OFFSET<<2)));	\

#define SET_CMD_RUN(VAL) 		\
	(*(volatile uint32 *)(SFMC_REG_RUN) = VAL);	\

#define WAIT_CMD_COMPLETE() {		\
	while(1) {	\
		if((*(volatile uint32 *)(SFMC_REG_RUN) & 0xF)== 0)	\
			break;	\
	};	\
}

#define SET_CODE_TABLE_ERASE_ADDR(OFFSET, ADDR) 		\
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + ((OFFSET+1)<<2)) &= ~(0x00FFFFFF)); \
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + ((OFFSET+1)<<2)) |= (ADDR&0x00FFFFFF))

#define SET_CODE_TABLE_WRITE_ADDR(OFFSET, ADDR) 		\
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + ((OFFSET+1)<<2)) &= ~(0x00FFFFFF)); \
	(*(volatile uint32 *)(SFMC_REG_CODE_TABLE + ((OFFSET+1)<<2)) |= (ADDR&0x00FFFFFF))


//==============================================
// Training Parameter Define
//==============================================
#define SFMC_SEL_DQS_NUM			4	// 0 ~ 3
#define SFMC_SEL_PN_NUM				2	// 0 ~ 1
#define SFMC_CS_EXTEND_NUM			4	// 0 ~ 3
#define SFMC_READ_LATENCY_NUM		8	// 0 ~ 7
#define SFMC_INV_SCLK_NUM			2	// 0, 1
#define SFMC_SLCH_NUM				3	// 0 ~ 2
#define SFMC_SLDH_NUM				2	// 0 ~ 1
#define SFMC_CLK_WD_CTRL_NUM		4	// 0 ~ 3
#define SFMC_CLK_WD_BUF_NUM			40	// 0 ~ 39
#define SFMC_CLK_RD_CTRL_NUM		4	// 0 ~ 3
#define SFMC_CLK_RD_BUF_NUM			40	// 0 ~ 39


typedef struct  {
	uint32 	offset;
	uint32	size;
} code_table_info_t;

typedef struct SNOR_MIO_PRODUCT_INFO {
	int8	*name;
	uint8 	ManufID;
	uint16 	DevID;
	uint32 	TotalSector;
	uint16 	cmd_read;
	uint16 	cmd_read_fast;
	uint16 	cmd_write;
	uint16 	flags;
} snor_product_info_t;

#define SFMC_BUF_SIZE		256

typedef struct SNOR_MIO_DRV {
    const int8	*name;
	uint8		ManufID;
	uint16		DevID;
    uint8		shift;
    uint16		flags;
    uint16		current_io_mode;
    uint16		max_read_io;

    uint32 		size;
    uint32 		page_size;
    uint32 		sector_size;
	uint32 		sector_count;
    uint32 		erase_size;

    uint16 		cmd_read;
    uint16 		cmd_read_fast;
    uint16 		cmd_write;
    uint16 		erase_cmd;

	uint16 		dt_mode;

	code_table_info_t	sfmc_buf;
	code_table_info_t	sfmc_addr;
	code_table_info_t	rdid;
	code_table_info_t	rdsr;
	code_table_info_t	wrsr;
	code_table_info_t	rdcr;
	code_table_info_t	wrcr;
	code_table_info_t	en4b;
	code_table_info_t	ex4b;
	code_table_info_t	ear_mode;

	//code_table_info_t	en_m_io;
	//code_table_info_t	ex_m_io;
	//code_table_info_t	config_dummy;
	code_table_info_t	write_enable;
	code_table_info_t 	write_disable;
	code_table_info_t	read;
	code_table_info_t	read_fast;
	code_table_info_t 	write;
	code_table_info_t	blk_erase;
	code_table_info_t	sec_erase;
	code_table_info_t	chip_erase;

	code_table_info_t	rdcr1;
	code_table_info_t	rdcr2;
	code_table_info_t	rdsr1;
	code_table_info_t	rdsr2;
	code_table_info_t	wrsr1;
	code_table_info_t	wrsr2;
	code_table_info_t	individual_lock;
	code_table_info_t	individual_unlock;

	code_table_info_t	rdab;
	code_table_info_t	wrab;

	uint32		iSFMC_REG_TIMING;
	uint32		iSFMC_REG_DELAY_SO;
	uint32		iSFMC_REG_DELAY_CLK;
	uint8 		uiDataBuffer[0x1000];
} snor_mio_drv_t;

//---------------------------------------------
// Erase commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_ERASE_4K				0x20
#define CMD_ERASE_4K_4B				0x21
#define CMD_ERASE_32K				0x52
#define CMD_ERASE_CHIP				0xc7
#define CMD_ERASE_64K				0xd8
#define CMD_ERASE_64K_4B			0xDC

//---------------------------------------------
// Write commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_PP						0x02	// page program
#define CMD_4PP						0x38	// Quad page program

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_PP4B					0x12	// page program 4b
#define CMD_4PP4B					0x3E	// Quad page program 4b
#define CMD_8PP4B					0x12ED	// Octa Page program 4b

//---------------------------------------------

#define CMD_WRITE_STATUS			0x01

#define CMD_WRITE_DISABLE			0x04
#define CMD_READ_STATUS				0x05
#define CMD_READ_CONFIG				0x15

#define CMD_QUAD_PAGE_PROGRAM		0x32
#define CMD_READ_STATUS1			0x35
#define CMD_WRITE_ENABLE			0x06
//#define CMD_READ_CONFIG				0x35
#define CMD_FLAG_STATUS				0x70

#define CMD_WRITE_CONFIG2_OPI		0x728D
#define CMD_WRITE_CONFIG2			0x72

#define CONFIG2_SPI_MODE			0x00
#define CONFIG2_STR_OPI_MODE		0x01
#define CONFIG2_DTR_OPI_MODE		0x02



//---------------------------------------------
// Read commands
//---------------------------------------------
// 3 byte Address Command Set
//---------------------------------------------
#define CMD_READ				0x03	// Normal read
#define CMD_FAST_READ			0x0B	// Fast read
#define CMD_FASTDTRD			0x0D	// Fast DT read

#define CMD_DREAD				0x3B	// Dual read
#define CMD_QREAD				0x6B	// Quad read

#define CMD_4READ				0xEB	// Quad STR read SPI/QPI

#define CMD_4DTRD				0xED	// Quad DT read

//---------------------------------------------
// 4 byte Address Command Set
//---------------------------------------------
#define CMD_READ4B				0x13
#define CMD_FAST_READ4B			0x0C
#define CMD_FASTDTRD_4B			0x0E

#define CMD_DREAD4B				0x3C
#define CMD_QREAD4B 			0x6C

#define CMD_4READ4B				0xEC	// Quad STR read SPI/QPI

#define CMD_4DTRD4B				0xEE	// Quad DT read


#define CMD_8READ				0xEC13
#define CMD_8DTRD				0xEE11

//---------------------------------------------

#define CMD_READ_ID					0x9f

#define CMD_EAR						0xC5

#define SNOR_SERIAL_MODE_0			(1 << 0)
#define SNOR_SERIAL_MODE_3			(1 << 1)

#define SNOR_PROTOCOL_SINGLE		(1 << 0)
#define SNOR_PROTOCOL_DUAL			(1 << 1)
#define SNOR_PROTOCOL_QUAD			(1 << 2)
#define SNOR_PROTOCOL_OCTA			(1 << 3)

#define SNOR_SECTOR_SIZE			0x1000
#define SNOR_PAGE_SIZE				0x100

#ifndef Hw0
#define Hw31	0x80000000
#define Hw30	0x40000000
#define Hw29	0x20000000
#define Hw28	0x10000000
#define Hw27	0x08000000
#define Hw26	0x04000000
#define Hw25	0x02000000
#define Hw24	0x01000000
#define Hw23	0x00800000
#define Hw22	0x00400000
#define Hw21	0x00200000
#define Hw20	0x00100000
#define Hw19	0x00080000
#define Hw18	0x00040000
#define Hw17	0x00020000
#define Hw16	0x00010000
#define Hw15	0x00008000
#define Hw14	0x00004000
#define Hw13	0x00002000
#define Hw12	0x00001000
#define Hw11	0x00000800
#define Hw10	0x00000400
#define Hw9		0x00000200
#define Hw8		0x00000100
#define Hw7		0x00000080
#define Hw6		0x00000040
#define Hw5		0x00000020
#define Hw4		0x00000010
#define Hw3		0x00000008
#define Hw2		0x00000004
#define Hw1		0x00000002
#define Hw0		0x00000001
#endif

#ifndef ISZERO
#define ISZERO(X, MASK)		( !(((uint32)(X)) & ((uint32)(MASK))) )
#endif
/* sf param flags */
enum {
	SECT_4K		= 1 << 0,
	SECT_32K	= 1 << 1,
	E_FSR		= 1 << 2,
	SST_WR		= 1 << 3,
	WR_QPP		= 1 << 4,
	ADDR_4B		= 1 << 5,
};




sint32 SNOR_MIO_Init(void);
void SNOR_MIO_Erase(uint32 address, uint32 size);
sint32 SNOR_MIO_Write(uint32 address, const void *pBuffer, uint32 length);
sint32 SNOR_MIO_Read(uint32 address, void *pBuffer, uint32 length);
sint32 SNOR_MIO_Read_Fast(uint32 address, void *pBuffer, uint32 length);
#if 0
sint32 SNOR_MIO_WriteSector(uint32 lba_addr, const void *pBuffer, uint32 nSector);
sint32 SNOR_MIO_ReadSector(unsigned long ulLBA_addr, unsigned long ulSector, void *buff);
#endif
sint32 SNOR_MIO_FWDN_LowFormat(void);
sint32 SNOR_MIO_FWDN_Read(uint32 address, uint32 length, void *buff);
sint32 SNOR_MIO_FWDN_Write(uint32 address, uint32 length, void *buf);
sint32 SNOR_MIO_AutoRead(uint8 mode);

uint32 SNOR_MIO_GetTotalSize(void);

#endif
