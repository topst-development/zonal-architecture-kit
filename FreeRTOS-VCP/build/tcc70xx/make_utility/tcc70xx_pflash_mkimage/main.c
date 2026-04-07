#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "jatom.h"
#include "mkimage.h"

extern unsigned int gPrintFlag;

static void help_msg(char *prog_name)
{
	printf(	"------------------------------------------------------------\n"
			"|  TCC_MK_SNOR_BOOT: Make up SNOR Master Image\n"
			"|  [USAGE]\n"
			"|  -i : input cfg file name\n"
			"|  -o : output file name\n"
			"|\n"
			"| * example \n"
			"| $ %s -i vcp.cfg -o vcp_flash.rom\n"
			"|\n"
			"| * Date: 2020.12.10\n"
			"------------------------------------------------------------\n", prog_name);
}

static BOOL parse_cfg(int argc, char *argv[], tcc_input_info_x *p_input_info)
{
    BOOL ret = FALSE;
	FILE *fd = NULL;
	char buf[1024], key[64], value[512];
	char *cfg_file = NULL, *sp, *ep;
    static struct option long_options[] = {
        {"cfg_name", 1, 0, 'c'},
        {"dest_name", 1, 0, 'o'},
        {0, 0, 0, 0}
    };

    if (!p_input_info) {
        return FALSE;
    }

    while (1) {
        int c = 0;
        int option_index = 0;

        c = getopt_long(argc, argv, "i:o:", long_options, &option_index);
        if (c == -1) { break; }

        switch (c) {
        case 0:
            break;
		case 'i':
			cfg_file = jmalloc_string(optarg);
			break;
        case 'o':
            p_input_info->dest_name = jmalloc_string(optarg);
            break;
        default:
            printf("invalid argument: optarg[%s]\n", optarg);
            break;
        }
    }

	fd = fopen(cfg_file, "r");
	if (!fd)
		return FALSE;

	while (!feof(fd))
	{
		memset(key, 0x00, 64);
		memset(value, 0x00, 512);

		fgets(buf, 1024, fd);

		if (buf[0] == '#')
		{
			continue;
		}
		else if (strstr(buf, "="))
		{
			sp = strstr(buf, "=");
			ep = strstr(buf, "\n");
			memcpy(key, buf, sp - buf);
			memcpy(value, sp + 1, ep - sp - 1);

            if (!strcmp(key, "CERT_ENABLE"))
			{
				p_input_info->cert_enable = (unsigned int)atol(value)? 1: 0;
			}
			else if (!strcmp(key, "HSM_BIN"))
			{
				p_input_info->hsm_bin_name = jmalloc_string(value);
			}
			else if (!strcmp(key, "MICOM_BIN"))
			{
				p_input_info->micom_bin_name = jmalloc_string(value);
			}
            else if (!strcmp(key, "CERT_BIN"))
			{
				p_input_info->cert_bin_name = jmalloc_string(value);
			}
            else if (!strcmp(key, "UPDATE_BIN"))
			{
				p_input_info->update_bin_name = jmalloc_string(value);
			}
		}
	}

	if( (p_input_info->dest_name != 0)
		&& (p_input_info->hsm_bin_name != 0)
		&& (p_input_info->micom_bin_name != 0)
		&& (p_input_info->cert_bin_name != 0)
		&& (p_input_info->update_bin_name != 0))
	{
		ret = TRUE;
	}
	else
	{
		printf("parse_cfg error .. \n");
	}

	return ret;
}


#ifdef ECC_TABLE_ENABLE
static void config_output_info(tcc_output_info *output_info, char *dest_name)
{
    char ucTempName[128]      = {0,};
    unsigned int idx        = 0;

    printf("=================== Each Image File Info ===================\n");

    //remove string ".rom"
    for(idx = 0; idx < (strlen(dest_name) - 4); idx++)
    {
        ucTempName[idx] = dest_name[idx];
    }

    for(idx = 0; idx < IMG_OUTPUT_FILE_NUM; idx++)
    {
        sprintf(output_info[idx].dest_name, ucTempName);
        switch(idx)
        {
            case IMG_DEFAULT:
            {
                printf("Default rom\n");
                strcat(output_info[idx].dest_name, IMG_FILENAME_DEFAULT);
                output_info[idx].total_img_size         = IMG_TOTAL_SIZE_1M;
                output_info[idx].target_data_start_addr = 0x00;
                output_info[idx].target_data_size       = 0;
                output_info[idx].ecc_start_addr         = 0;
                output_info[idx].ecc_size               = 0;
                output_info[idx].done_flag              = 1;
                break;
            }

            case IMG_1M_ECC:
            {
                printf("1 MB rom\n");
                strcat(output_info[idx].dest_name, IMG_FILENAME_1M_ECC);
                output_info[idx].total_img_size         = IMG_TOTAL_SIZE_1M;
                output_info[idx].target_data_start_addr = 0x00;
                output_info[idx].target_data_size       = output_info[idx].total_img_size / (unsigned int) 9;
                /* Multiplying only by '8' does not make data size divisible by '64' */
                output_info[idx].target_data_size       = ((output_info[idx].target_data_size >> 3) << 6);
                output_info[idx].ecc_size               = (output_info[idx].target_data_size >> 3);
                output_info[idx].ecc_start_addr         = output_info[idx].target_data_size;
                break;
            }

            case IMG_2M_ECC:
            {
                printf("2 MB rom\n");
                strcat(output_info[idx].dest_name, IMG_FILENAME_2M_ECC);
                output_info[idx].total_img_size         = IMG_TOTAL_SIZE_2M;
                output_info[idx].target_data_start_addr = 0x00;
                output_info[idx].target_data_size       = output_info[idx].total_img_size / (unsigned int) 9;
                output_info[idx].target_data_size       = ((output_info[idx].target_data_size >> 3) << 6);
                output_info[idx].ecc_size               = (output_info[idx].target_data_size >> 3);
                output_info[idx].ecc_start_addr         = output_info[idx].target_data_size;
                break;
            }

            case IMG_3M_ECC:
            {
                printf("3 MB rom\n");
                strcat(output_info[idx].dest_name, IMG_FILENAME_3M_ECC);
                output_info[idx].total_img_size         = IMG_TOTAL_SIZE_3M;
                output_info[idx].target_data_start_addr = IMG_TOTAL_SIZE_2M;
                output_info[idx].target_data_size       = IMG_TOTAL_SIZE_1M / (unsigned int) 9;
                output_info[idx].target_data_size       = ((output_info[idx].target_data_size >> 3) << 6);
                output_info[idx].ecc_size               = (output_info[idx].target_data_size >> 3);
                output_info[idx].ecc_start_addr         = (IMG_TOTAL_SIZE_2M + output_info[idx].target_data_size);
                break;
            }

            case IMG_4M_ECC:
            {
                printf("4 MB rom\n");
                strcat(output_info[idx].dest_name, IMG_FILENAME_4M_ECC);
                output_info[idx].total_img_size         = IMG_TOTAL_SIZE_4M;
                output_info[idx].target_data_start_addr = IMG_TOTAL_SIZE_2M;
                output_info[idx].target_data_size       = IMG_TOTAL_SIZE_2M / (unsigned int) 9;
                output_info[idx].target_data_size       = ((output_info[idx].target_data_size >> 3) << 6);
                output_info[idx].ecc_size               = (output_info[idx].target_data_size >> 3);
                output_info[idx].ecc_start_addr         = (IMG_TOTAL_SIZE_2M + output_info[idx].target_data_size);
                break;
            }

            default:
            {
                break;
            }
        }

        printf("    target_data_start_addr : \t 0x%08X\n",   output_info[idx].target_data_start_addr);
        printf("    target_data_size       : \t 0x%08X\n",   output_info[idx].target_data_size);
        printf("    ecc_start_addr         : \t 0x%08X\n",   output_info[idx].ecc_start_addr);
        printf("    ecc_size               : \t 0x%08X\n\n", output_info[idx].ecc_size);
    }
}

static unsigned int calculate_micom_size(char *micom_bin_name)
{
    unsigned int uiSize                 = 0;
    FILE *fpMcuFd                       = NULL;

    fpMcuFd = fopen(micom_bin_name, "r");
    if(!fpMcuFd)
    {
        printf("cannot open file (r5_fw.rom) !!\n");
    }
    else
    {
        fseek(fpMcuFd, 0, SEEK_END);
        uiSize = ftell(fpMcuFd);

        uiSize += (2 * EF_INIT_HEAD_SIZE) + (2 * SFMC_INIT_HEAD_SIZE)
               + (2 * HSM_AREA_SIZE) + MICOM_UPDATER_AREA_SIZE
               + MICOM_UPDATER_ALIGN_SIZE;

        fclose(fpMcuFd);
    }

    return uiSize;
}

static BOOL tcc_sfmc_mk_bootrom(tcc_input_info_x *p_input_info)
{
    BOOL ret                            = TRUE;
    char *dest_name                     = p_input_info->dest_name;
    char *hsm_bin_name                  = p_input_info->hsm_bin_name;
    char *micom_bin_name                = p_input_info->micom_bin_name;
    char *cert_bin_name                 = p_input_info->cert_bin_name;
    char *update_bin_name               = p_input_info->update_bin_name;
    char ucRemoveName[128]              = {0,};
    unsigned int cert_enable            = p_input_info->cert_enable;
    unsigned int secured                = 0;
    unsigned int uiMcuSize              = 0;
    unsigned int uiPreMcuSize           = 0;    // EF + SFMC + HSM + R5(MCU) + ECC
    unsigned int uiHsmSize              = 0;
    unsigned int cert_size              = 0;
    unsigned int updater_size           = 0;
    unsigned int uiFileIdx              = 0;
    tcc_output_info sOutputInfo[8]      = {0,};

    FILE *dest_fd                       = NULL;
    FILE *hsm_bin_fd                    = NULL;
    FILE *micom_bin_fd                  = NULL;
    FILE *cert_bin_fd                   = NULL;
    FILE *update_bin_fd	                = NULL;

    hsm_bin_fd      = fopen(hsm_bin_name, "rb");        //hsm.bin
    micom_bin_fd    = fopen(micom_bin_name, "rb");      //r5_fw.bin
    cert_bin_fd     = fopen(cert_bin_name, "rb");       //vcpcert.bin
    update_bin_fd   = fopen(update_bin_name, "rb");     //updater.bin

    if ((hsm_bin_fd == NULL) || (micom_bin_fd == NULL) || (cert_bin_fd == NULL) || (update_bin_fd == NULL))
    {
        if(hsm_bin_fd == NULL)
            printf("[%s : %d] ERROR! File does not existed, Fd is NULL (%s)\n", __func__, __LINE__, hsm_bin_name);

        if(micom_bin_fd == NULL)
            printf("[%s : %d] ERROR! File does not existed, Fd is NULL (%s) \n", __func__, __LINE__, micom_bin_name);

        if(cert_bin_fd == NULL)
            printf("[%s : %d] ERROR! File does not existed, Fd is NULL (%s) \n", __func__, __LINE__, cert_bin_name);

        if(update_bin_fd == NULL)
            printf("[%s : %d] ERROR! File does not existed, Fd is NULL (%s) \n", __func__, __LINE__, update_bin_name);

        ret = FALSE;
    }
    else
    {
        config_output_info(sOutputInfo, dest_name);

        uiPreMcuSize = calculate_micom_size(micom_bin_name);
        if(!uiPreMcuSize)
        {
            printf("[%s : %d] ERROR! MCU F/W size calculation error", __func__, __LINE__);
            ret = FALSE;
        }
        else
        {
            gPrintFlag = TRUE;

            for(uiFileIdx = IMG_DEFAULT ; uiFileIdx < IMG_OUTPUT_FILE_NUM ; uiFileIdx++)
            {
                if (sOutputInfo[uiFileIdx].dest_name)
                {
                    if((uiFileIdx != IMG_DEFAULT) && ((uiPreMcuSize + sOutputInfo[uiFileIdx].ecc_size) > (uiFileIdx * IMG_TOTAL_SIZE_1M)))
                    {
                        printf("\n");
                        printf("\x1b[1;32m<OUTPUT FILE : %s>\x1b[0m\n", sOutputInfo[uiFileIdx].dest_name);

                        sOutputInfo[uiFileIdx].done_flag = 0;

                        /* file size (EF + SFMC + HSM + MICOM) is bigger than target IMG size */
                        printf("\x1b[1;33m(Warning) Cannot make (%d MB) rom file because file size exceeded. \x1b[0m\n", uiFileIdx);
                        printf("\t  Total size   :    %07d byte\n", uiPreMcuSize + sOutputInfo[uiFileIdx].ecc_size);
                        printf("\t  r5_fw size   :    %07d byte\n", uiPreMcuSize);
                        printf("\t  ECC size     :    %07d byte\n", sOutputInfo[uiFileIdx].ecc_size);

                        /* remove rom file : rm -rf tcc70xx_pflash_boot_XM_ECC.rom */
                        sprintf(ucRemoveName, "rm -rf ");
                        strcat(ucRemoveName, sOutputInfo[uiFileIdx].dest_name);
                        system(ucRemoveName);

                        continue;
                    }

                    /* tcc70xx_pflash_boot ~ .rom */
                    dest_fd = fopen(sOutputInfo[uiFileIdx].dest_name, "w+b");
                    if (dest_fd == NULL)
                    {
                        if(dest_fd == NULL)
                            printf("[%s : %d] ERROR! File Open Fail (%s) \n", __func__, __LINE__, sOutputInfo[uiFileIdx].dest_name);

                        ret = FALSE;
                    }
                    else
                    {
                        fseek(hsm_bin_fd, 0, SEEK_SET);
                        fseek(micom_bin_fd, 0, SEEK_SET);
                        fseek(cert_bin_fd, 0, SEEK_SET);
                        fseek(update_bin_fd, 0, SEEK_SET);

                        if(write_pflash_init_header(dest_fd) == FALSE)
                        {
                            printf("[%s : %d] write_pflash_init_header FAIL\n", __func__, __LINE__);
                            ret = FALSE;
                        }
                        else
                        {
                            if(write_hsm_image(dest_fd, hsm_bin_fd, &uiHsmSize) == FALSE)
                            {
                                printf("[%s : %d] write_hsm_image FAIL\n", __func__, __LINE__);
                                ret = FALSE;
                            }
                            else
                            {
                                if (strstr(micom_bin_name, ".bin"))
                                    secured = 0;
                                else if (strstr(micom_bin_name, ".rom"))
                                    secured = 1;

                                if(write_micom_rom(dest_fd, micom_bin_fd, &uiMcuSize, secured) == FALSE)
                                {
                                    printf("[%s : %d] write_micom_rom FAIL\n", __func__, __LINE__);
                                    ret = FALSE;
                                }
                                else
                                {
                                    if(cert_enable == 1)
                                    {
                                        if(write_cert_image(dest_fd, cert_bin_fd, &cert_size) == FALSE)
                                        {
                                            printf("[%s : %d] write_cert_image FAIL\n", __func__, __LINE__);
                                            ret = FALSE;
                                        }
                                    }

                                    if(ret == TRUE)
                                    {
                                        if(write_micom_updater_image(dest_fd, update_bin_fd, &updater_size, cert_enable) == FALSE)
                                        {
                                            printf("[%s : %d] write_micom_updater_image FAIL\n", __func__, __LINE__);
                                            ret = FALSE;
                                        }
                                        else
                                        {
                                            if(uiFileIdx != IMG_DEFAULT)
                                            {
                                                if(write_padding(dest_fd, &sOutputInfo[uiFileIdx]) == FALSE)
                                                {
                                                    printf("[%s : %d] write_padding FAIL\n", __func__, __LINE__);
                                                    ret = FALSE;
                                                }
                                                else
                                                {
                                                    if(write_ecc(dest_fd, &sOutputInfo[uiFileIdx]) == FALSE)
                                                    {
                                                        printf("[%s : %d] write_ecc FAIL\n", __func__, __LINE__);
                                                    }
                                                    else
                                                    {
                                                        sOutputInfo[uiFileIdx].done_flag = 1;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    fflush(dest_fd);

                    CLOSE_HANDLE(dest_fd, NULL, fclose);
                }

                gPrintFlag = FALSE;
            }

            printf("\n\x1b[1;32m<OUTPUT FILE LIST>\x1b[0m\n");
            for(uiFileIdx = 0; uiFileIdx < IMG_OUTPUT_FILE_NUM; uiFileIdx++)
            {
                if(sOutputInfo[uiFileIdx].done_flag)
                {
                    printf("%s\n",sOutputInfo[uiFileIdx].dest_name);
                }
            }
            printf("\n\n");

        }
    }

    CLOSE_HANDLE(hsm_bin_fd, NULL, fclose);
    CLOSE_HANDLE(micom_bin_fd, NULL, fclose);
    CLOSE_HANDLE(cert_bin_fd, NULL, fclose);
    CLOSE_HANDLE(update_bin_fd, NULL, fclose);

    return ret;
}

#else
/*          Original image make function             */
static BOOL tcc_sfmc_mk_bootrom(tcc_input_info_x *p_input_info)
{

	unsigned int hsm_size = 0;
	unsigned int micom_size = 0;
    unsigned int cert_size = 0;
    unsigned int updater_size = 0;

	BOOL ret = FALSE;
	char *dest_name			= p_input_info->dest_name;
	char *hsm_bin_name		= p_input_info->hsm_bin_name;
	char *micom_bin_name	= p_input_info->micom_bin_name;
    char *cert_bin_name     = p_input_info->cert_bin_name;
    char *update_bin_name	= p_input_info->update_bin_name;
	FILE *dest_fd		= NULL;
	FILE *mcert_bin_fd	= NULL;
	FILE *hsm_bin_fd	= NULL;
	FILE *r5bl1_bin_fd	= NULL;
    FILE *cert_bin_fd	= NULL;
	FILE *update_bin_fd	= NULL;
	FILE *micom_bin_fd	= NULL;
	unsigned int secured = 0;
    unsigned int cert_enable= p_input_info->cert_enable;

	if (dest_name)
	{
		if (!exist_file(dest_name))
		{
			dest_fd			= fopen(dest_name, "w+b");
			hsm_bin_fd		= fopen(hsm_bin_name, "rb");
			micom_bin_fd	= fopen(micom_bin_name, "rb");
            cert_bin_fd	    = fopen(cert_bin_name, "rb");
            update_bin_fd	= fopen(update_bin_name, "rb");

			if (dest_fd && hsm_bin_fd && micom_bin_fd && cert_bin_fd && update_bin_fd) \
			{
				;
			}
			else
			{
				printf("cannot make file !! (%s) \n",  dest_name);
				printf("(%s) file exist!!!\n", dest_name);
				goto close;
			}

			//===================================
			// Write EFLASH & SFMC Init Header 0 & 1 ((2 Kbyte + 2 Kbyte) * 2) : 0x00000000 ~ 0x00002000
			//===================================
			ret = write_pflash_init_header(dest_fd);
			if (ret == FALSE)
				goto close;


			//===================================
			// HSM rom 0 & 1 Write (256 Kbyte * 2) : 0x00002000 ~ 0x00022000
			//===================================
			ret = write_hsm_image(dest_fd, hsm_bin_fd, &hsm_size);
			if (ret == FALSE)
				goto close;

			//===================================
			//R5 Main rom 0 & 1 write : 0x0200000 ~
			//===================================
			if (strstr(micom_bin_name, ".bin"))
				secured = 0;
			else if (strstr(micom_bin_name, ".rom"))
				secured = 1;

			ret = write_micom_rom(dest_fd, micom_bin_fd, &micom_size, secured);
			if (ret == FALSE)
				goto close;

            //===================================
			// Cert bin Write (4 Kbyte)
			//===================================
			if(cert_enable == 1)
            {
    			ret = write_cert_image(dest_fd, cert_bin_fd, &cert_size);
    			if (ret == FALSE)
    				goto close;
            }

            //===================================
			// R5 updater rom 0 Write (64 Kbyte)
			//===================================
			ret = write_micom_updater_image(dest_fd, update_bin_fd, &updater_size, cert_enable);
			if (ret == FALSE)
				goto close;

			/* Create size-aligned final ROM file */
/*
			ret = create_final_rom(dest_fd, p_input_info->snor_size);
			if (ret == FALSE)
				goto close;
*/

			fflush(dest_fd);
		}
	}
close:
	CLOSE_HANDLE(mcert_bin_fd, NULL, fclose);
	CLOSE_HANDLE(hsm_bin_fd, NULL, fclose);
	CLOSE_HANDLE(r5bl1_bin_fd, NULL, fclose);
    CLOSE_HANDLE(cert_bin_fd, NULL, fclose);
	CLOSE_HANDLE(update_bin_fd, NULL, fclose);
	CLOSE_HANDLE(micom_bin_fd, NULL, fclose);
	CLOSE_HANDLE(dest_fd, NULL, fclose);

	if (ret == FALSE)
		remove(dest_name);

	return ret;
}
#endif

int main(int argc, char *argv[])
{
    int ret = -1;
    tcc_input_info_x param;

    memset(&param, 0x0, sizeof(tcc_input_info_x));

	if (parse_cfg(argc, argv, &param) == TRUE) {
		printf("\n");
		printf("HSM binary file   : (%s)\n"  , param.hsm_bin_name );
		printf("MICOM binary file : (%s)\n"  , param.micom_bin_name );
		printf("CERT binary file: (%s)\n\n", param.cert_bin_name );
		printf("UPDATE binary file: (%s)\n\n", param.update_bin_name );

	    if (!tcc_sfmc_mk_bootrom(&param)) {
	        printf("make fail!!! \n");
	    } else {
	        ret = 0;
	    }
	} else {
		help_msg(argv[0]);
	}

    return ret;
}

