/****************************************************************************

One line to give the program's name and a brief idea of what it does.

Copyright (C) 2013 Telechips Inc.



This program is free software; you can redistribute it and/or modify it under the terms

of the GNU General Public License as published by the Free Software Foundation;

either version 2 of the License, or (at your option) any later version.



This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;

without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR

PURPOSE. See the GNU General Public License for more details.



You should have received a copy of the GNU General Public License along with

this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,

Suite 330, Boston, MA 02111-1307 USA

****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mkimage.h"

unsigned int gPrintFlag;

///////////////////////////////////////////temp
//FRWCON - offset 0
#define EF_FRWCON_BURSTMD (0x200UL)   // how to operate???

#define EF_FRWCON_RESET_CYCLE (0x100UL)

#define EF_FRWCON_ERASE_MODE_MASK (0x18UL)
#define EF_FRWCON_ERASE_MODE_M (2UL)
#define EF_FRWCON_ERASE_MODE_S (1UL)
#define EF_FRWCON_ERASE_MODE_P (0UL)

#define EF_FRWCON_PROG (0x2UL)

#define EF_FRWCON_ERASE (1UL)


//FSHSTAT - offset 4
#define EF_FSHSTAT_MAT_BUSY_MASK (0xFF00)

#define EF_FSHSTAT_MAT0_BUSY (0x0100)
#define EF_FSHSTAT_MAT1_BUSY (0x0200)
#define EF_FSHSTAT_MAT2_BUSY (0x0400)
#define EF_FSHSTAT_MAT3_BUSY (0x0800)
#define EF_FSHSTAT_MAT4_BUSY (0x1000)
#define EF_FSHSTAT_MAT5_BUSY (0x2000)
#define EF_FSHSTAT_MAT6_BUSY (0x4000)
#define EF_FSHSTAT_MAT7_BUSY (0x8000)

#define EF_FSHSTAT_DATA_LATCHED (0x80)
#define EF_FSHSTAT_ADDR_LATCHED (0x40)
#define EF_FSHSTAT_AnD_RDY (0x20)
#define EF_FSHSTAT_CMD_RDY (0x10)

//TM_FSHCON - offset 0x34
#define EF_TMFSHCON_ECC_ON    (0x10000000)
#define EF_TMFSHCON_BBLOCKEN  (0x01000000)
#define EF_TMFSHCON_TREG_ACCESS_EN   (0x4)
#define EF_TMFSHCON_USERMODE_TEST_EN (0x2)

//TM_ECCCON_1 - offset 0x38

#define EF_TMECCCON_WDATAECC_MASK  (0xFF00)
#define EF_TMECCCON_FAULT_CLR  (0x80)
#define EF_TMECCCON_FAULT_TYPE_MASK  (0x30)
#define EF_TMECCCON_FAULT_1  (0x8)

//TM_ECCFADDR_1 - offset 0x3c

//DCYCRDCON    - offset 0x100

#define EF_DCYCRDCON_FUSERD_WAIT_TIME_MASK (0xFf0000)

#define EF_DCYCRDCON_RD_WAIT_TIME_MASK (0x1f)

//DCYCWRCON    - offset 0x104

#define EF_DCYCWRCON_CMD_HOLD_TIME_MASK (0x1f00100)

#define EF_DCYCWRCON_CMD_SET_TIME_MASK (0xf00)

///////////////////////////////////////////temp

#define SNOR_PAGE_SIZE				(256)


static const unsigned long int CRC32_TABLE[256] = {
    0x00000000, 0x90910101, 0x91210201, 0x01B00300,
    0x92410401, 0x02D00500, 0x03600600, 0x93F10701,
    0x94810801, 0x04100900, 0x05A00A00, 0x95310B01,
    0x06C00C00, 0x96510D01, 0x97E10E01, 0x07700F00,
    0x99011001, 0x09901100, 0x08201200, 0x98B11301,
    0x0B401400, 0x9BD11501, 0x9A611601, 0x0AF01700,
    0x0D801800, 0x9D111901, 0x9CA11A01, 0x0C301B00,
    0x9FC11C01, 0x0F501D00, 0x0EE01E00, 0x9E711F01,
    0x82012001, 0x12902100, 0x13202200, 0x83B12301,
    0x10402400, 0x80D12501, 0x81612601, 0x11F02700,
    0x16802800, 0x86112901, 0x87A12A01, 0x17302B00,
    0x84C12C01, 0x14502D00, 0x15E02E00, 0x85712F01,
    0x1B003000, 0x8B913101, 0x8A213201, 0x1AB03300,
    0x89413401, 0x19D03500, 0x18603600, 0x88F13701,
    0x8F813801, 0x1F103900, 0x1EA03A00, 0x8E313B01,
    0x1DC03C00, 0x8D513D01, 0x8CE13E01, 0x1C703F00,
    0xB4014001, 0x24904100, 0x25204200, 0xB5B14301,
    0x26404400, 0xB6D14501, 0xB7614601, 0x27F04700,
    0x20804800, 0xB0114901, 0xB1A14A01, 0x21304B00,
    0xB2C14C01, 0x22504D00, 0x23E04E00, 0xB3714F01,
    0x2D005000, 0xBD915101, 0xBC215201, 0x2CB05300,
    0xBF415401, 0x2FD05500, 0x2E605600, 0xBEF15701,
    0xB9815801, 0x29105900, 0x28A05A00, 0xB8315B01,
    0x2BC05C00, 0xBB515D01, 0xBAE15E01, 0x2A705F00,
    0x36006000, 0xA6916101, 0xA7216201, 0x37B06300,
    0xA4416401, 0x34D06500, 0x35606600, 0xA5F16701,
    0xA2816801, 0x32106900, 0x33A06A00, 0xA3316B01,
    0x30C06C00, 0xA0516D01, 0xA1E16E01, 0x31706F00,
    0xAF017001, 0x3F907100, 0x3E207200, 0xAEB17301,
    0x3D407400, 0xADD17501, 0xAC617601, 0x3CF07700,
    0x3B807800, 0xAB117901, 0xAAA17A01, 0x3A307B00,
    0xA9C17C01, 0x39507D00, 0x38E07E00, 0xA8717F01,
    0xD8018001, 0x48908100, 0x49208200, 0xD9B18301,
    0x4A408400, 0xDAD18501, 0xDB618601, 0x4BF08700,
    0x4C808800, 0xDC118901, 0xDDA18A01, 0x4D308B00,
    0xDEC18C01, 0x4E508D00, 0x4FE08E00, 0xDF718F01,
    0x41009000, 0xD1919101, 0xD0219201, 0x40B09300,
    0xD3419401, 0x43D09500, 0x42609600, 0xD2F19701,
    0xD5819801, 0x45109900, 0x44A09A00, 0xD4319B01,
    0x47C09C00, 0xD7519D01, 0xD6E19E01, 0x46709F00,
    0x5A00A000, 0xCA91A101, 0xCB21A201, 0x5BB0A300,
    0xC841A401, 0x58D0A500, 0x5960A600, 0xC9F1A701,
    0xCE81A801, 0x5E10A900, 0x5FA0AA00, 0xCF31AB01,
    0x5CC0AC00, 0xCC51AD01, 0xCDE1AE01, 0x5D70AF00,
    0xC301B001, 0x5390B100, 0x5220B200, 0xC2B1B301,
    0x5140B400, 0xC1D1B501, 0xC061B601, 0x50F0B700,
    0x5780B800, 0xC711B901, 0xC6A1BA01, 0x5630BB00,
    0xC5C1BC01, 0x5550BD00, 0x54E0BE00, 0xC471BF01,
    0x6C00C000, 0xFC91C101, 0xFD21C201, 0x6DB0C300,
    0xFE41C401, 0x6ED0C500, 0x6F60C600, 0xFFF1C701,
    0xF881C801, 0x6810C900, 0x69A0CA00, 0xF931CB01,
    0x6AC0CC00, 0xFA51CD01, 0xFBE1CE01, 0x6B70CF00,
    0xF501D001, 0x6590D100, 0x6420D200, 0xF4B1D301,
    0x6740D400, 0xF7D1D501, 0xF661D601, 0x66F0D700,
    0x6180D800, 0xF111D901, 0xF0A1DA01, 0x6030DB00,
    0xF3C1DC01, 0x6350DD00, 0x62E0DE00, 0xF271DF01,
    0xEE01E001, 0x7E90E100, 0x7F20E200, 0xEFB1E301,
    0x7C40E400, 0xECD1E501, 0xED61E601, 0x7DF0E700,
    0x7A80E800, 0xEA11E901, 0xEBA1EA01, 0x7B30EB00,
    0xE8C1EC01, 0x7850ED00, 0x79E0EE00, 0xE971EF01,
    0x7700F000, 0xE791F101, 0xE621F201, 0x76B0F300,
    0xE541F401, 0x75D0F500, 0x7460F600, 0xE4F1F701,
    0xE381F801, 0x7310F900, 0x72A0FA00, 0xE231FB01,
    0x71C0FC00, 0xE151FD01, 0xE0E1FE01, 0x7070FF00
};

sEflash_InitHeader stCODE_def_EFLASH = {
	0x534C4665,      //"eFLS"
	0xFFFFFFFC,//0xFFFFFFEC,      //valid_n
	0x001E0002,      //DCYCRDCON
	0x00020100,      //DCYCWRCON

	0xFFFFFFFF,      //EXTCON0
	0xFFFFFFFF,	     //RSTCNT
	0xFFFFFFFF,//0x13000000,	     //EFLASH_CLKCHG
	{
	0xFFFFFFFF,	     //reserved[0]
	0xFFFFFFFF,	     //reserved[1]
	0xFFFFFFFF,	     //reserved[2]
	0xFFFFFFFF,	     //reserved[3]
	0xFFFFFFFF,	     //reserved[4]
	0xFFFFFFFF,	     //reserved[5]
	0xFFFFFFFF,	     //reserved[6]
	0xFFFFFFFF,	     //reserved[7]
	},
	0x04000000,		 //ulCRC
};


#if 1// VCP boot test


// Quad SPI 4READ3B & 4READ4B Commands (STR)
sSFQPI_InitHeader stCODE_4READ3B = {
	0x000000E0,//0x00000090,		// code;	//80Mhz, PLL0:1200, Div: 14+1
//	0x00000170,//0x00000090,		// code;	//50Mhz, PLL0:1200, Div: 23+1
	0x00040310,		// timing;
	0x00000500,		// delay_so;
	0x00001515,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,//0x00000820,		// dc_base_addr_manu_0;
	0x0000081C,//0x00000828,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	{
		0x00000000,		// reserved[0]
		0x00000000,		// reserved[1]
		0x00000000,		// reserved[2]
	},
					// read_3b
	{
		0x840000EB,//0x84000003,	// 0x800	// 4READ 3B
		0x4A000001,//0x48000001,	// 0x804
		0x86000000,//0x28000000,	// 0x808
		0x46002000,//0xF4000000,	// 0x80C
		0x2A000000,//0xF4000000,	// 0x810
		0xF4000000,	// 0x814	// STOP

		0xF4000000,//0xF4000000, // 0x818	// STOP
		0xF4000000, // 0x81C	// STOP

                0xA4000006,
                0xF4000000,
                0x84000001,
                0xA4000040,
                0xF4000000,
                0xF4000000,
                0xF4000000,

                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
                0xF4000000,
	},
	0x00000077		// crc
};


#else
// Quad SPI 4READ3B & 4READ4B Commands (STR)
sSFQPI_InitHeader stCODE_4READ3B = {
	0x00000090,		// code;	//80Mhz, PLL0:800, Div: 9+1
//	0x000000B0,		// code;	//66Mhz, PLL0:800, Div: 11+1
	0x00140300,		// timing;
//	0x000001FF,		// delay_so;
	0x00000500,		// delay_so;
	0x0000000C,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,		// dc_base_addr_manu_0;
	0x0000081C,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	{
		0x00000000,		// reserved[0]
		0x00000000,		// reserved[1]
		0x00000000,		// reserved[2]
	},
					// read_3b
	{
		0x840000EB,	// 0x800	// 4READ 3B
		0x4A000001,	// 0x804
		0x86000000,	// 0x808
		0x46002000,	// 0x80C
		0x2A000000,	// 0x810
		0xF4000000,	// 0x814	// STOP

		0xF4000000, // 0x818	// STOP
		0xF4000000, // 0x81C	// STOP
	},
	0x00000077		// crc
};
#endif /* NEVER */

sSFQPI_InitHeader stCODE_4READ4B = {
	0x00000090,		// code;	//80Mhz, PLL0:800, Div: 9+1
//	0x000000B0,		// code;	//66Mhz, PLL0:800, Div: 11+1
	0x00140300,		// timing;
//	0x000001FF,		// delay_so;
	0x00000500,		// delay_so;
	0x0000000C,		// dc_clk;
	0x00000000,		// dc_wbd0;
	0x00000000,		// dc_wbd1;
	0x00000000,		// dc_rbd0;
	0x00000000,		// dc_rbd1;
	0x00000000,		// dc_woebd0;
	0x00000000,		// dc_woebd1;
	0x00000818,		// dc_base_addr_manu_0;
	0x0000081C,		// dc_base_addr_manu_1;
	0x00000800,		// dc_base_addr_auto;
	0x00000001,		// run_mode;
	{
		0x00000000,		// reserved[0]
		0x00000000,		// reserved[1]
		0x00000000,		// reserved[2]
	},
					// read_4b
	{
		0x840000EC,	// 0x800	// 4READ 4B
		0x4A000000,	// 0x804
		0x86000000,	// 0x808
		0x46002000,	// 0x80C
		0x2A000000,	// 0x810
		0xF4000000,	// 0x814	// STOP

		0xA40000B7, // 0x818	// EN4B
		0xF4000000, // 0x81C	// STOP
		0xF4000000, // 0x820	// STOP
	},
	0x00000077		// crc
};

static unsigned int gMicom_rom_size;

void clear_input_info(tcc_input_info_x *p_input_info)
{
    if (p_input_info) {
        CLOSE_HANDLE(p_input_info->dest_name, NULL, free);
    }
}

char *jmalloc_string(char *sz)
{
    char *ret_str = NULL;
    unsigned int len = 0;

    if (sz) {
        len = strlen(sz);
        ret_str = (char *)malloc((len * sizeof(char)) + sizeof(char));
        if (ret_str) {
            strcpy(ret_str, sz);
        }
    }
    return ret_str;
}

unsigned int FWUG_CalcCrc8(unsigned char *base, unsigned int length)
{
	unsigned int crcout = 0;
	unsigned int cnt;
	unsigned char	code, tmp;

	for(cnt=0; cnt<length; cnt++)
	{
		code = base[cnt];
		tmp = code^crcout;
		crcout = (crcout>>8)^CRC32_TABLE[tmp&0xFF];
	}
	return crcout;
}

unsigned int PUSH_CRC(unsigned int crc, unsigned char *src, unsigned size)
{
    for ( ; size; size--) {
        crc = CRC32_TABLE[(crc ^ (*(src++))) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}


int EFLASH_MIO_BOOT_Write_Header(unsigned char *pucRomFile_Buffer)
{

    sEflash_InitHeader sEfInitHeader;
    unsigned int 	uiHeaderSize;
    unsigned char	*pucRomFile = pucRomFile_Buffer;
    unsigned int	uiRomFileIndex;

    //=====================================================
    // Write Boot Header
    //=====================================================
    uiHeaderSize = SFMC_INIT_HEAD_SIZE * 2;

    //=====================================================
    // Make SFMC InitHeader Info
    //=====================================================
    memset(&sEfInitHeader, 0x00, sizeof(sEflash_InitHeader));

    memcpy(&sEfInitHeader, &stCODE_def_EFLASH, sizeof(sEflash_InitHeader));

    sEfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sEfInitHeader, sizeof(sEflash_InitHeader) - 4);

    if(gPrintFlag)
    {
        printf("<SNOR_MAP: 0x%08x ++0x%08x>\n", SFMC_INIT_HEAD0_OFFSET, SFMC_INIT_HEAD_SIZE*2);
        printf("[Make EFLASH Read CMD Header...]\n");

        printf("Header Size: %d byte\n", uiHeaderSize);
        printf("EFLASH Register Set for Chipboot )\n");
        printf("\t R0  :       0x%08X\n", sEfInitHeader.signature);
        printf("\t R1  :       0x%08X\n", sEfInitHeader.valid_n);
        printf("\t R2  :       0x%08X\n", sEfInitHeader.DCYCRDCON);
        printf("\t R3  :       0x%08X\n", sEfInitHeader.DCYCWRCON);
        printf("\t R4  :       0x%08X\n", sEfInitHeader.EXTCON0);
        printf("\t R5  :       0x%08X\n", sEfInitHeader.RSTCNT);
        printf("\t CRC :       0x%08X\n", sEfInitHeader.ulCRC);
    }

    uiRomFileIndex = 0;
    memcpy(&pucRomFile[uiRomFileIndex], &sEfInitHeader, sizeof(sEflash_InitHeader));

    return 0;
}


int SNOR_MIO_BOOT_Write_Header(unsigned char *pucRomFile_Buffer)
{
	sSFQPI_InitHeader sfInitHeader;
	unsigned int 	uiHeaderSize;
	unsigned char	*pucRomFile = pucRomFile_Buffer;
	unsigned int	uiRomFileIndex;

	//=====================================================
	// Write Boot Header
	//=====================================================
	uiHeaderSize = SFMC_INIT_HEAD_SIZE * 2;

	//=====================================================
	// Make SFMC InitHeader Info
	//=====================================================
	memset(&sfInitHeader, 0x00, sizeof(sSFQPI_InitHeader));

	memcpy(&sfInitHeader, &stCODE_4READ3B, sizeof(sSFQPI_InitHeader));

    sfInitHeader.ulCRC = FWUG_CalcCrc8((unsigned char*)&sfInitHeader, sizeof(sSFQPI_InitHeader) - 4);

    if(gPrintFlag)
    {
        printf("\n");
        printf("<SNOR_MAP: 0x%08x ++0x%08x>\n", SFMC_INIT_HEAD0_OFFSET, SFMC_INIT_HEAD_SIZE*2);
        printf("[Make SFMC Read CMD Header...]\n");
        printf("Header Size: %d byte\n", uiHeaderSize);
        printf("(0) FAST READ CMD Set for Chipboot (SPI)\n");
        {
            printf("\tCode:        0x%08X\n", sfInitHeader.code);
            printf("\tTiming:      0x%08X\n", sfInitHeader.timing);
            printf("\tDelay_s:     0x%08X\n", sfInitHeader.delay_so);
            printf("\tDc_clk:      0x%08X\n", sfInitHeader.dc_clk);
            printf("\tRun_mode:    0x%08X\n", sfInitHeader.run_mode);
            printf("\tRead_cmd:    0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
                                        sfInitHeader.code_vlu[0],
                                        sfInitHeader.code_vlu[1],
                                        sfInitHeader.code_vlu[2],
                                        sfInitHeader.code_vlu[3],
                                        sfInitHeader.code_vlu[4]);
        }

        printf("\tCMD CRC:     0x%08X\n", sfInitHeader.ulCRC);
    }

	uiRomFileIndex = SFMC_INIT_HEAD_SIZE;
	memcpy(&pucRomFile[uiRomFileIndex], &sfInitHeader, sizeof(sSFQPI_InitHeader));

	return 0;
}

BOOL write_pflash_init_header(FILE *dest_fd)
{
	unsigned char *headers_buf;

    if (dest_fd) {
		//=========================================
		// Prepare: Header buffer
		//=========================================
		headers_buf = malloc(SFMC_INIT_HEAD_SIZE * 2);
		if (!headers_buf) {
			printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
			return FALSE;
		}

        memset(headers_buf, 0xFF, SFMC_INIT_HEAD_SIZE * 2);


		//=========================================
		// Make
		//=========================================

		if (EFLASH_MIO_BOOT_Write_Header((unsigned char *)headers_buf) != 0)
			return FALSE;

		if (SNOR_MIO_BOOT_Write_Header((unsigned char *)headers_buf) != 0)
			return FALSE;

		fseek(dest_fd, EF_INIT_HEAD0_OFFSET, SEEK_SET);
		if (fwrite(headers_buf, 1, (EF_INIT_HEAD_SIZE + SFMC_INIT_HEAD_SIZE) , dest_fd) != (EF_INIT_HEAD_SIZE + SFMC_INIT_HEAD_SIZE))
		{//lun 0
			free(headers_buf);
			printf("%s - file write fail\n", __func__);
			return FALSE;
		}
		else
		{
			if (fwrite(headers_buf, 1, (EF_INIT_HEAD_SIZE + SFMC_INIT_HEAD_SIZE) , dest_fd) != (EF_INIT_HEAD_SIZE + SFMC_INIT_HEAD_SIZE))
			{//lun 1
				free(headers_buf);
				printf("%s - file write fail\n", __func__);
				return FALSE;
			}
			free(headers_buf);
			//printf("%s - success\n", __func__);
			return TRUE;
		}
    }
    return FALSE;
}


BOOL write_hsm_image(FILE *dest_fd, FILE *hsm_bin_fd, unsigned int *hsm_size)
{
    BOOL            ret = FALSE;
    unsigned int    len = 0;
    unsigned char * hsm_area_buf;

    if(gPrintFlag)
    {
        printf("\n");
        printf("<SNOR_MAP: 0x%08x ++0x%08x>\n", HSM_AREA0_OFFSET, HSM_AREA_SIZE*2);
        printf("[Write HSM F/W Image ...]\n");
    }

    hsm_area_buf = malloc(HSM_AREA_SIZE);
    if (!hsm_area_buf)
    {
        printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
        return FALSE;
    }
    memset(hsm_area_buf, 0xff, HSM_AREA_SIZE);

    fseek(hsm_bin_fd, 0, SEEK_END);
    len = ftell(hsm_bin_fd);
    if (len > HSM_AREA_SIZE)
    {
        printf("The file size is larger than area size - write fail\n");
        free(hsm_area_buf);
        return FALSE;
    }
    *hsm_size = len;

    fseek(hsm_bin_fd, 0, SEEK_SET);
    fread(hsm_area_buf, 1, len , hsm_bin_fd);

    fseek(dest_fd, HSM_AREA0_OFFSET, SEEK_SET);
    if (fwrite(hsm_area_buf, 1, HSM_AREA_SIZE, dest_fd) != HSM_AREA_SIZE)
    {
        free(hsm_area_buf);
        printf("%s - 0 write fail\n", __func__);
        return FALSE;
    }

    fseek(dest_fd, HSM_AREA1_OFFSET, SEEK_SET);
    if (fwrite(hsm_area_buf, 1, HSM_AREA_SIZE, dest_fd) != HSM_AREA_SIZE)
    {
        printf("%s - 1 write fail\n", __func__);
    }
    else
    {
        //printf("%s - success\n", __func__);
        ret = TRUE;
    }

    free(hsm_area_buf);

    return ret;
}


BOOL write_micom_rom(FILE *dest_fd, FILE *micom_bin_fd, unsigned int *micom_size, unsigned int secured)
{
    unsigned int len = 0;
    unsigned char *micom_rom_header_buf;

    if (dest_fd && micom_bin_fd)
    {
        fseek(micom_bin_fd, 0, SEEK_END);
        len = ftell(micom_bin_fd);

        if(gPrintFlag)
        {
            printf("\n");
            printf("<SNOR_MAP: 0x%08x ++0x%08x>\n", MICOM_HEADER_OFFSET, MICOM_ROM_AREA_SIZE);
            printf("[Write %s Micom FW Image ...]\n", secured? "Secure": "Non-Secure");
            printf("MICOM ROM size: 0x%x\n", len);
        }

        gMicom_rom_size = len;

        micom_rom_header_buf = malloc(len);
        if (!micom_rom_header_buf)
        {
            printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
            return FALSE;
        }

        fseek(micom_bin_fd, 0, SEEK_SET);
        fread(micom_rom_header_buf, 1, len, micom_bin_fd);

        /* Non-Secure image has no header */
        if(secured)
            fseek(dest_fd, MICOM_HEADER_OFFSET, SEEK_SET);
        else
            fseek(dest_fd, MICOM_HEADER_OFFSET + MICOM_ROM_HEADER_SIZE, SEEK_SET);

        if (fwrite(micom_rom_header_buf, 1, len, dest_fd) != len)
        {
            free(micom_rom_header_buf);
            printf("%s - file write fail\n", __func__);
            return FALSE;
        }
        else
        {
            free(micom_rom_header_buf);
            *micom_size = len;
            //printf("%s - success\n", __func__);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL write_cert_image(FILE *dest_fd, FILE *cert_bin_fd, unsigned int *cert_bin_size)
{
    unsigned int    image_size = 0;
    unsigned char * cert_img_buf;
    unsigned char * cert_area_buf;

    unsigned int    align_size;
    unsigned int    align_addr;

    align_size = gMicom_rom_size % CERT_ALIGN_SIZE;

    if(align_size != 0)
    {
        align_size = CERT_ALIGN_SIZE - align_size;
    }

    align_addr = MICOM_HEADER_OFFSET + gMicom_rom_size + align_size;

    if (dest_fd && cert_bin_fd)
    {
        fseek(cert_bin_fd, 0, SEEK_END);
        image_size = ftell(cert_bin_fd);
        cert_img_buf = malloc(image_size);

        if(gPrintFlag)
        {
            printf("\n");
            printf("align_size : 0x%08X, align_addr : 0x%08X\n", align_size, align_addr);

            printf("\n");
            printf("<SNOR_MAP: 0x%08X ++0x%08X>\n", align_addr, CERT_AREA_SIZE);
            printf("[Write Cert Image ...]\n");
            printf("\tCert f/w size : 0x%X\n", image_size);
        }

        if (!cert_img_buf)
        {
            printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
            return FALSE;
        }
        else
        {
            memset(cert_img_buf, 0xff, image_size);
        }

        fseek(cert_bin_fd, 0, SEEK_SET);
        fread(cert_img_buf, 1, image_size, cert_bin_fd);

        cert_area_buf = malloc(CERT_AREA_SIZE);
        if (!cert_area_buf)
        {
            printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
            return FALSE;
        }
        else
        {
            memset(cert_area_buf, 0xFF, CERT_AREA_SIZE);
        }

        memcpy(&cert_area_buf[0], (unsigned char *)cert_img_buf, image_size);

        fseek(dest_fd, align_addr, SEEK_SET);
        if (fwrite(cert_area_buf, 1, CERT_AREA_SIZE, dest_fd) != CERT_AREA_SIZE)
        {
            free(cert_img_buf);
            free(cert_area_buf);
            printf("%s - file write fail\n", __func__);
            return FALSE;
        }
        else
        {
            free(cert_img_buf);
            free(cert_area_buf);
            *cert_bin_size = image_size;
            //printf("%s - success\n", __func__);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL write_micom_updater_image(FILE *dest_fd, FILE *update_bin_fd, unsigned int *updater_size, unsigned int cert_enable)
{
    unsigned int    image_size = 0;
    unsigned char * updater_img_buf;
    unsigned char * updater_area_buf;

    unsigned int    align_size;
    unsigned int    align_addr;

    align_size = gMicom_rom_size % MICOM_UPDATER_ALIGN_SIZE;

    if(align_size != 0)
    {
        align_size = MICOM_UPDATER_ALIGN_SIZE - align_size;
    }

    if(cert_enable == 1)
    {
        align_addr = MICOM_HEADER_OFFSET + gMicom_rom_size + CERT_AREA_SIZE + align_size;
    }
    else
    {
        align_addr = MICOM_HEADER_OFFSET + gMicom_rom_size + align_size;
    }

    if (dest_fd && update_bin_fd)
    {
        fseek(update_bin_fd, 0, SEEK_END);
        image_size = ftell(update_bin_fd);
        updater_img_buf = malloc(image_size);

        if(gPrintFlag)
        {
            printf("\n");
            printf("align_size : 0x%08X, align_addr : 0x%08X\n", align_size, align_addr);

            printf("\n");
            printf("<SNOR_MAP: 0x%08X ++0x%08X>\n", align_addr, MICOM_UPDATER_AREA_SIZE);
            printf("[Write Micom Updater Image ...]\n");
            printf("\tMICOM Sub f/w size : 0x%X\n", image_size);
        }

        if (!updater_img_buf)
        {
            printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
            return FALSE;
        }
        else
        {
            memset(updater_img_buf, 0xff, image_size);
        }

        fseek(update_bin_fd, 0, SEEK_SET);
        fread(updater_img_buf, 1, image_size, update_bin_fd);

        updater_area_buf = malloc(MICOM_UPDATER_AREA_SIZE);
        if (!updater_area_buf)
        {
            printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
            return FALSE;
        }
        else
        {
            memset(updater_area_buf, 0xFF, MICOM_UPDATER_AREA_SIZE);
        }

        memcpy(&updater_area_buf[0], (unsigned char *)updater_img_buf, image_size);

        fseek(dest_fd, align_addr, SEEK_SET);
        if (fwrite(updater_area_buf, 1, MICOM_UPDATER_AREA_SIZE, dest_fd) != MICOM_UPDATER_AREA_SIZE)
        {
            free(updater_img_buf);
            free(updater_area_buf);
            printf("%s - file write fail\n", __func__);
            return FALSE;
        }
        else
        {
            free(updater_img_buf);
            free(updater_area_buf);
            *updater_size = image_size + MICOM_UPDATER_HEADER_SIZE;
            //printf("%s - success\n", __func__);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL create_final_rom(FILE *out_rom_fd, unsigned int snor_size)
{
    unsigned char *rom_buf;
    unsigned int   len;
    unsigned int   out_rom_size;
    size_t         read_size   = 0;
    BOOL           ret         = FALSE;

    if (out_rom_fd)
    {
        out_rom_size = (snor_size * 1024);

	rom_buf = malloc(out_rom_size);
	if (!rom_buf)
        {
	    printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
	    return FALSE;
	}
	memset(rom_buf, 0xff, out_rom_size);

	fseek(out_rom_fd, 0, SEEK_END);
	len = ftell(out_rom_fd);
	fseek(out_rom_fd, 0, SEEK_SET);
	read_size = fread(rom_buf, 1, len, out_rom_fd);
	if( read_size != len)
	{
	    printf("[%s: %d] File read error\n", __func__, __LINE__);
	    free(rom_buf);
	    ret = FALSE;
	}
	else
	{
	    printf("Total Image Size: %d byte\n", (unsigned int)(out_rom_size));
            fseek(out_rom_fd, 0, SEEK_SET);

	    if (fwrite(rom_buf, 1, out_rom_size, out_rom_fd) != out_rom_size)
	    {
		free(rom_buf);
		printf("%s - file write fail\n", __func__);
		ret =  FALSE;
	    }
	    else
	    {
		free(rom_buf);
		printf("%s - success\n", __func__);
		ret =  TRUE;
	    }
	}
    }

    return ret;
}

#ifdef ECC_TABLE_ENABLE
BOOL write_padding(FILE *dest_fd, tcc_output_info *output_info)
{
    BOOL ret                            = TRUE;
    unsigned char *padding_buf          = NULL;
    unsigned int uiCurSize              = 0;
    unsigned int uiPaddingSize          = 0;
    unsigned int uiTotalSize            = 0;

    if (dest_fd)
    {
        fseek(dest_fd, 0, SEEK_END);
        uiCurSize = ftell(dest_fd);

        uiTotalSize     = output_info->total_img_size;
        uiPaddingSize   = uiTotalSize - uiCurSize;

        if(gPrintFlag)
        {
            printf("\n");
            printf("<SNOR_MAP: 0x%08X ~ 0x%08X>\n", uiCurSize, uiTotalSize);
            printf("[Write padding ...]\n");
            printf("\tCurrent File Size : %d\n", uiCurSize);
            printf("\tPadding data size : %d\n", uiPaddingSize);
            printf("\tTotal data size   : %d\n", uiTotalSize);
        }

        padding_buf = (unsigned char*)malloc(sizeof(unsigned char) * uiPaddingSize);
        if (!padding_buf)
        {
            printf("[%s: %d] Low memory (cannot allocate memory for verify)\n", __func__, __LINE__);
            ret = FALSE;
        }
        else
        {
            memset(padding_buf, 0xff, uiPaddingSize);
            if (fwrite(padding_buf, 1, uiPaddingSize, dest_fd) != uiPaddingSize)
            {
                printf("%s - padding data write fail\n", __func__);
                ret = FALSE;
            }
            else
            {
                //printf("%s - success\n", __func__);
                ret = TRUE;
            }
            free(padding_buf);
        }
    }

    return ret;
}

BOOL get_ecc_data(    FILE *dest_fd, unsigned char *ecc_buf, tcc_output_info *output_info)
{
    BOOL            ret                 = TRUE;
    unsigned char*  pcuDataBuf          = NULL;
    unsigned char   ucEccBuf            = 0;
    unsigned int    uiLoopNum           = 0;
    unsigned int    uiLoopCnt           = 0;
    unsigned int    uiEccIdx            = 0;
    unsigned int    uiUpperData         = 0;
    unsigned int    uiLowerData         = 0;
    unsigned int    idx;

    pcuDataBuf = (unsigned char*)malloc(ECC_DATA_BLOCK);
    if(!pcuDataBuf)
    {
        printf("[%s: %d] Low memory(cannot allocate memory for verify)\n", __func__, __LINE__);
        ret = FALSE;
    }
    else
    {
        uiLoopNum = output_info->target_data_size / ECC_DATA_BLOCK;

        fseek(dest_fd, output_info->target_data_start_addr, SEEK_SET);

        uiEccIdx = uiLoopCnt = 0;

        while(uiLoopCnt < uiLoopNum)
        {
            memset(pcuDataBuf, 0x00, ECC_DATA_BLOCK);

            if(fread(pcuDataBuf, 1, ECC_DATA_BLOCK, dest_fd) != ECC_DATA_BLOCK)
            {
                printf("[%s: %d] File read error\n", __func__, __LINE__);
                ret = FALSE;
                break;
            }

            ucEccBuf = 0;
            for(idx = 0; (idx * 4) < ECC_DATA_BLOCK; idx += 2)
            {
                uiLowerData = *(unsigned int *)(pcuDataBuf + (4 * idx));
                uiUpperData = *(unsigned int *)(pcuDataBuf + (4 * (idx + 1)));
                gen_parity(uiUpperData, uiLowerData, &ucEccBuf);

                ecc_buf[uiEccIdx++] = ucEccBuf;
            }

            uiLoopCnt++;
        }

        free(pcuDataBuf);
    }

    return ret;
}

BOOL write_ecc(FILE *dest_fd, tcc_output_info *output_info)
{
    BOOL            ret                 = FALSE;
    unsigned char*  pucEccBuf           = NULL;
    unsigned int    uiEccSize           = output_info->ecc_size;

    if(gPrintFlag)
    {
        printf("\n");
        printf("<SNOR_MAP: 0x%08X ++0x%08X>\n", output_info->ecc_start_addr, output_info->ecc_size);
        printf("[Write ECC data ...]\n");
        printf("\tECC start address : 0x%08X\n" , output_info->ecc_start_addr);
        printf("\tECC data size     : %d\n"     , uiEccSize);
    }

    if (dest_fd)
    {
        pucEccBuf = (unsigned char*)malloc(uiEccSize);
        if(!pucEccBuf)
        {
            printf("[%s: %d] Low memory (cannot allocate memory for verify)\n", __func__, __LINE__);
            ret = FALSE;
        }
        else
        {
            memset(pucEccBuf, 0x00, uiEccSize);
            if(get_ecc_data(dest_fd, pucEccBuf, output_info) == FALSE)
            {
                printf("[%s: %d] Get ecc data Fail\n", __func__, __LINE__);
                ret = FALSE;
            }
            else
            {
                fseek(dest_fd, output_info->ecc_start_addr, SEEK_SET);
                if (fwrite(pucEccBuf, 1, uiEccSize, dest_fd) != uiEccSize)
                {
                    printf("[%s: %d] ECC write FAIL!!!\n", __func__, __LINE__);
                    ret = FALSE;
                }
                else
                {
                    //printf("write_ecc_table - success\n\n");
                    ret = TRUE;
                }
            }
        }

        free(pucEccBuf);
    }

    return ret;
}
#endif
