/* SPDX-License-Identifier: GPL-2.0+ WITH Linux-syscall-note */
/*
 * Copyright (C) Telechips Inc.
 */
#ifndef TCC_OVERLAY_IOCTL_H
#define TCC_OVERLAY_IOCTL_H

#if defined(CONFIG_TELECHIPS_SCREEN_SHARE)
#define OVERLAY_PUSH_SHARED_BUFFER (91U)
#endif
#define OVERLAY_SET_CONFIGURE      (50U)
#define OVERLAY_SET_LAYER          (51U)
#define OVERLAY_PUSH_VIDEO_BUFFER  (90U)
#define OVERLAY_PUSH_VIDEO_BUFFER_SCALING  (92U)
#define OVERLAY_GET_LAYER          (52U)
#define OVERLAY_DISALBE_LAYER      (53U)
#define OVERLAY_SET_OVP            (54U)
#define OVERLAY_GET_OVP            (55U)
#define OVERLAY_GET_PANEL_SIZE     (56U)
#define OVERLAY_SET_OVP_KERNEL     (154U)
#define OVERLAY_GET_OVP_KERNEL     (155U)
#define OVERLAY_OTF_PUSH_FRAME	   (131U)

struct panel_size_t {
	unsigned int xres;
	unsigned int yres;
};

typedef struct {
	unsigned int sx;
	unsigned int sy;
	unsigned int width;
	unsigned int height;
	unsigned int format;
	unsigned int transform;
} overlay_config_t;

typedef struct {
	overlay_config_t cfg;
	unsigned int addr;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int afbc_dec_num;
	unsigned int afbc_dec_need;
} overlay_video_buffer_t;

typedef struct {
	unsigned int sx;
	unsigned int sy;
	unsigned int width;
	unsigned int height;
	unsigned int dstwidth;
	unsigned int dstheight;
	unsigned int format;
	unsigned int transform;
} overlay_config_scaling_t;

typedef struct {
	overlay_config_scaling_t cfg;
	unsigned int addr;
	unsigned int addr1;
	unsigned int addr2;
} overlay_video_buffer_scaling_t;

#if defined(CONFIG_TELECHIPS_SCREEN_SHARE)
typedef struct {
	unsigned int src_addr;
	unsigned int src_x;
	unsigned int src_y;
	unsigned int src_w;
	unsigned int src_h;
	unsigned int dst_x;
	unsigned int dst_y;
	unsigned int dst_w;
	unsigned int dst_h;
	unsigned int frm_w;
	unsigned int frm_h;
	unsigned int fmt;
	unsigned int layer;
	unsigned int rgb_swap;
} overlay_shared_buffer_t;
#endif

#endif
