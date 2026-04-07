
#ifndef __MKIMAGE_H__
#define __MKIMAGE_H__

#include "jatom.h"
#include "ecc.h"

#define ECC_TABLE_ENABLE   //mkimage upgrade

#define ALIGN_SIZE 4

/***************************** SNOR Flash Memory Map *****************************/


#define EF_INIT_HEAD_SIZE               (2 * 1024)
#define EF_INIT_HEAD0_OFFSET            (0x00000000)
#define EF_INIT_HEAD1_OFFSET            (0x00001000)

#define SFMC_INIT_HEAD_SIZE             (2 * 1024)
#define SFMC_INIT_HEAD0_OFFSET          (0x00000800)
#define SFMC_INIT_HEAD1_OFFSET          (0x00001800)

#define HSM_AREA_SIZE                   (128 * 1024)
#define HSM_AREA0_OFFSET                (0x00002000)
#define HSM_AREA1_OFFSET                (0x00022000)

#define MICOM_ROM_AREA_SIZE             (0x00100000)
#define MICOM_ROM_HEADER_SIZE           (0x00001000)
#define MICOM_HEADER_OFFSET             (0x00042000)

#define CERT_AREA_SIZE                  (4 * 1024)
#define CERT_ALIGN_SIZE                 (0x10)

#define MICOM_UPDATER_AREA_SIZE         (192 * 1024)
#define MICOM_UPDATER_HEADER_SIZE       (0)
#define MICOM_UPDATER_HEADER_OFFSET     (0x00143000)
#define MICOM_UPDATER_IMAGE_OFFSET      (MICOM_UPDATER_HEADER_OFFSET + MICOM_UPDATER_HEADER_SIZE)
#define MICOM_UPDATER_ALIGN_SIZE        (0x10)


/*********************************************************************************/
#ifdef ECC_TABLE_ENABLE

#define ECC_DATA_BLOCK                  (64)
#define IMG_OUTPUT_FILE_NUM             (5)
#define IMG_FILENAME_DEFAULT            ".rom"
#define IMG_FILENAME_1M_ECC             "_1M_ECC.rom"
#define IMG_FILENAME_2M_ECC             "_2M_ECC.rom"
#define IMG_FILENAME_3M_ECC             "_3M_ECC.rom"
#define IMG_FILENAME_4M_ECC             "_4M_ECC.rom"
#define IMG_TOTAL_SIZE_1M               (1 * 1024 * 1024)
#define IMG_TOTAL_SIZE_2M               (2 * 1024 * 1024)
#define IMG_TOTAL_SIZE_3M               (3 * 1024 * 1024)
#define IMG_TOTAL_SIZE_4M               (4 * 1024 * 1024)

enum _IMAGE_TYPE_
{
    IMG_DEFAULT = 0,                    //It is for default image 'tcc70xx_pflash_boot', not including ECC
    IMG_1M_ECC,
    IMG_2M_ECC,
    IMG_3M_ECC,
    IMG_4M_ECC
};

typedef struct _tcc_output_info_{
    char            dest_name[64];
    char            done_flag;
    unsigned int    total_img_size;             // total rom size (1~4 Mb)
    unsigned int    target_data_start_addr;     // Data offset for calculating ECC table (depend on image size)
    unsigned int    target_data_size;           // Data size for calculating ECC table : header + hsm + vcp_fw + vcp_sub_fw + padding
    unsigned int    ecc_start_addr;             // ECC offset (start point)
    unsigned int    ecc_size;                   // Result of ECC calculation : data_size / 8
}tcc_output_info;


extern BOOL write_padding(FILE *dest_fd, tcc_output_info *output_info);

extern BOOL write_ecc(FILE *dest_fd, tcc_output_info *output_info);

#endif

/*********************************************************************************/

/* eflash Init Header */

typedef	struct {
	unsigned int	signature; // "eFLS"
	unsigned int	valid_n;   // []: 0=valid, 1=invalid
	unsigned int	DCYCRDCON; // 0x0100: (0x1E << 16) | (0x02 << 0); // [23:16]FUSEDRD_WAIT_TIME=0x1E, [4:0]RD_WAIT_TIME=0x02
	unsigned int	DCYCWRCON; // 0x0104: (0x02 << 16) | (0x01 << 8); // [20:16]CMD_HOLD_TIME=0x02, [11:8]CMD_SET_TIME=0x01
	unsigned int	EXTCON0;   // 0x8030: (0x00 <<  8) | (0x00 << 2); // [12:8]CLK_DIV_VAL=0x0, [2]CLK_DIV_EN=0x0
	unsigned int	RSTCNT;    // 0x8040: (19 << 16) | (0xC8 << 0); // [26:16]DCT_INIT_TIME, [11:0]RSTN_TIME
        unsigned int    EFLASH_CLKCHG; // b4: 0xA0F24020: [30:28]CLK_SEL(1=PLL0_FOUT,2=PLL1_FOUT), [27:24]DIV(0=D2,1=D4,2=D6,3=D8): (0x13<<24)
        unsigned int	reserved[8];
	unsigned int	ulCRC;
} sEflash_InitHeader;



/* SFMC Init Header */
#define CODE_VLU_SIZE	46

typedef	struct {
	unsigned int	code;
				//[1:0]   -> sflash_mode_sel        => 0x0 : SPI(fixed) 0x1: QPI, 0x2: QPI-DUAL 0x3: OPI (0x0 to 0x2 are the same)
			    //[2]     -> str/dtr                => 0x0: STR(fixed), 0x1: DTR
			    //[3]     -> auto/manu              => 0x0: AUTO(fixed), 0x1: MANU
			    //[11:4]  -> fclk_div(fin=800MHz)   => 0x1f: 25MHz, 0x1d: 26MHz, 0x1b: 28MHz, 0x19: 30MHz, 0x17:33MHz,  0x15:36MHz,  0x13: 40MHz,  0x11:44MHz,
			    //					  				   0x0f: 50MHz, 0x0d: 57MHz, 0x0b: 66MHz, 0x09: 80MHz, 0x07:100MHz, 0x05:133MHz, 0x03: 200MHz, 0x01: 400MHz.
			    //[13:12] -> fclk_sel               => 0x0: PLL0(800Mhz, fixed)
			    //[31]    -> reserved
	unsigned int	timing;
	unsigned int	delay_so;
	unsigned int	dc_clk;
	unsigned int	dc_wbd0;
	unsigned int	dc_wbd1;
	unsigned int	dc_rbd0;
	unsigned int	dc_rbd1;
	unsigned int	dc_woebd0;
	unsigned int	dc_woebd1;
	unsigned int	dc_base_addr_manu_0;
	unsigned int	dc_base_addr_manu_1;
	unsigned int	dc_base_addr_auto;
	unsigned int	run_mode;
	unsigned int	ulReserved[3];
	unsigned int	code_vlu[CODE_VLU_SIZE];
	unsigned int	ulCRC;
} sSFQPI_InitHeader;

/* Input files information structure */
typedef struct _tcc_input_info_x {
    char *dest_name;
	char *hsm_bin_name;
    char *micom_bin_name;
    char *cert_bin_name;
    char *update_bin_name;
    unsigned int cert_enable;
	unsigned int snor_size;
	unsigned int micom_rom_bass_addr;
} tcc_input_info_x;

extern char *jmalloc_string(char *sz);
extern BOOL write_pflash_init_header(FILE *dest_fd);

extern BOOL write_hsm_image(FILE *dest_fd, FILE *hsm_bin_fd, unsigned int *hsm_size);

extern BOOL write_micom_rom(FILE *dest_fd, FILE *micom_rom_fd, unsigned int *micom_size, unsigned int secured);

extern BOOL write_cert_image(FILE *dest_fd, FILE *cert_bin_fd, unsigned int *cert_bin_size);

extern BOOL write_micom_updater_image(FILE *dest_fd, FILE *update_bin_fd, unsigned int *updater_size, unsigned int cert_enable);

extern BOOL create_final_rom(FILE *out_rom_fd, unsigned int snor_size);

extern void clear_input_info(tcc_input_info_x *p_input_info);

#endif /* __MKIMAGE_H__ */
