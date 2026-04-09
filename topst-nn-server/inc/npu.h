/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * telechips npu driver
 *
 * Copyright (C) 2020 Telechips
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef TELECHIPS_NPU_H
#define TELECHIPS_NPU_H

#include <linux/ioctl.h>

typedef struct buf_alloc_req             buf_alloc_req_t;
typedef struct net_load_req              net_load_req_t;
typedef struct net_run_req               net_run_req_t;
typedef struct net_profile_req           net_profile_req_t;
typedef struct npu_init_req              npu_init_req_t;
//TODO: For next update
// typedef struct net_current_state_req     net_current_state_req_t;
typedef struct reg_access_req            reg_access_req_t;
typedef struct test_cfg_wr_req           test_cfg_wr_req_t;
typedef struct npu_err_rd_req            npu_err_rd_req_t;
typedef struct npu_ecc_cbuf              npu_ecc_cbuf_t;
typedef struct npu_ecc_gbuf              npu_ecc_gbuf_t;
typedef struct npu_ecc_sram              npu_ecc_sram_t;
typedef union  npu_err_bits              npu_err_bits_t;

#define NPU_COLOR_YUV (0U)
#define NPU_COLOR_RGB (1U)
#define NPU_COLOR_END (2U)

struct buf_alloc_req {
	unsigned int size;
	unsigned long addr;
};

struct net_load_req {
	char *cmd_data;
	unsigned long cmd_size;

	char *wei_data;
	unsigned long wei_size;
};

struct net_run_req {
	unsigned int in_fd;
	unsigned int out_fd;
	npu_err_bits_t *err_status;
};

struct net_profile_req {
	unsigned int in_fd;
	unsigned int out_fd;
	npu_err_bits_t *err_status;

	unsigned int elapsed_in_us;

	unsigned int dma;
	unsigned int comp;
	unsigned int all;
};

struct npu_init_req {
	unsigned int disable_ue_fail;
	unsigned int disable_ce_fail;
	unsigned int disable_wdt;
	unsigned int soft_reset;
};

//TODO: For next update
// struct net_current_state_req {
// 	unsigned int dma;
// 	unsigned int comp;
// 	unsigned int elapsed_in_us;;
// };

struct reg_access_req {
	unsigned int addr;
	unsigned int data;
};

struct npu_ecc_sram {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_status;
	unsigned int ce_status;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
	unsigned int ce_addr;
	unsigned int ce_data;
	unsigned int ue_addr;
	unsigned int ue_data;
};

struct npu_ecc_gbuf {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
};

struct npu_ecc_cbuf {
	unsigned int ue_irq_flag;
	unsigned int ce_irq_flag;
	unsigned int ue_cnt;
	unsigned int ce_cnt;
};

#define MAX_NUM_NPU_CORE    (0x4U)

struct npu_err_rd_req {
	npu_ecc_cbuf_t cbuf;
	npu_ecc_gbuf_t gbuf[MAX_NUM_NPU_CORE];
	npu_ecc_sram_t sram[MAX_NUM_NPU_CORE];
	unsigned int wdt_to;
	unsigned int irq_reason;
};

union npu_err_bits {
	struct {
		unsigned int ce_sram : 1;
		unsigned int ce_gbuf : 1;
		unsigned int ce_cbuf : 1;
		unsigned int ce_rsv  : 1;
		unsigned int ue_sram : 1;
		unsigned int ue_gbuf : 1;
		unsigned int ue_cbuf : 1;
		unsigned int ue_rsv  : 1;
		unsigned int wdt_to  : 1;
		unsigned int wdt_rsv : 7;
		unsigned int reserved: 16;
	} as_field;

	unsigned int as_word;
};

struct test_cfg_wr_req {
	unsigned int mlx_bin_idx;
	unsigned int wdt_ext_cnt;
	unsigned int wdt_int_cnt;
	unsigned int ecc_test_ctrl;
	unsigned int mlx_err_inj_mask_data;
	unsigned int mlx_err_inj_mask_par;
};

#define NPU_IOCTL_MAGIC                 'k'

#define NPU_IOCTL_ALLOC_BUFFER          _IOW(NPU_IOCTL_MAGIC,  0, buf_alloc_req_t *)
#define NPU_IOCTL_LOAD_NETWORK          _IOW(NPU_IOCTL_MAGIC,  1, net_load_req_t *)
#define NPU_IOCTL_READ_REG              _IOWR(NPU_IOCTL_MAGIC, 2, reg_access_req_t *)
#define NPU_IOCTL_WRITE_REG             _IOW(NPU_IOCTL_MAGIC,  3, reg_access_req_t *)
#define NPU_IOCTL_RESET_NPU             _IOW(NPU_IOCTL_MAGIC,  4, npu_init_req_t *)
#define NPU_IOCTL_WRITE_TEST_CFG        _IOWR(NPU_IOCTL_MAGIC, 5, test_cfg_wr_req_t *)
#define NPU_IOCTL_READ_NPU_ERR          _IOWR(NPU_IOCTL_MAGIC, 6, npu_err_rd_req_t *)

#define NPU_NET_IOCTL_RUN               _IOW(NPU_IOCTL_MAGIC,  0, net_run_req_t *)
#define NPU_NET_IOCTL_PROFILE           _IOWR(NPU_IOCTL_MAGIC, 1, net_profile_req_t *)
#define NPU_NET_IOCTL_SET_COLOR_FMT     _IOW(NPU_IOCTL_MAGIC,  2, int)
//TODO: For next update
// #define NPU_NET_GET_LASTEST_STATUS      _IOW(NPU_IOCTL_MAGIC,  3, net_current_state_req_t)

#endif //TELECHIPS_NPU_H
