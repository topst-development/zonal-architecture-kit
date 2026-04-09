/*
 * Copyright Telechips Inc.
 *
 * TCC Version 1.0
 *
 * This source code contains confidential information of Telechips.
 *
 * Any unauthorized use without a written permission of Telechips including not
 * limited to re-distribution in source or binary form is strictly prohibited.
 *
 * This source code is provided "AS IS" and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without
 * limitation, any warranty of merchantability, fitness for a particular purpose
 * or non-infringement of any patent, copyright or other third party intellectual
 * property right.
 * No warranty is made, express or implied, regarding the information's accuracy,
 * completeness, or performance.
 *
 * In no event shall Telechips be liable for any claim, damages or other
 * liability arising from, out of or in connection with this source code or
 * the use in the source code.
 *
 * This source code is provided subject to the terms of a Mutual Non-Disclosure
 * Agreement between Telechips and Company.
 */

#ifndef TELECHIPS_NPU_API_H
#define TELECHIPS_NPU_API_H

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dlfcn.h>

#include <npu.h>

#include <stdio.h>
#if 0
//#define TELECHIPS_NPU_API_DEBUG
#ifdef TELECHIPS_NPU_API_DEBUG
#   define npu_api_fin()  \
        do {(void)fprintf(stderr, "NPUAPI --> %4d:%s()\n", __LINE__,__func__);} while(0)
#   define npu_api_fout() \
        do {(void)fprintf(stderr, "NPUAPI <-- %4d:%s()\n", __LINE__,__func__);} while(0)
#   define npu_api_debug(fmt, args...) \
        do {(void)fprintf(stderr, "NPUAPI %4d:%s:" fmt, __LINE__, __func__, ##args);} while(0)
#   define npu_api_print_err(fmt, args...) \
        do { (void)fprintf(stderr, "NPUAPI %4d:%s: " fmt, __LINE__, __func__, ##args); } while(0)
#   define telechips_print(fmt, args...) \
        do { (void)fprintf(stdout, "NPUAPI %4d:%s: " fmt, __LINE__, __func__, ##args); } while(0)
#else
#   define npu_api_fin()                do {} while(0)
#   define npu_api_fout()               do {} while(0)
#   define npu_api_debug(fmt, args...)  do {} while(0)
#   define telechips_print(fmt, args...) //
#endif

#define npu_api_print_err(fmt, args...) \
        do { (void)fprintf(stderr, "NPUAPI %4d:%s: " fmt, __LINE__, __func__, ##args); } while(0)
#endif

/** @brief Struct of NPU device
 */
typedef struct {
    int fd;                 /**< file descriptor        */
} npu_t;


/** @brief Struct of NPU DMA buffer
 */
typedef struct {
    int fd;                 /**<  file descriptor       */
    int size;               /**<  buffer size           */

    char* caddr;            /**<  addr of mmap w/ DMA fd*/
    unsigned long paddr;    /**<  addr of pysical address*/
} npu_buf_t;


/** @brief Struct of network
 */
typedef struct {
    int fd;                 /**<  file descriptor       */

    struct enlight_net* methods;
                            /**<  arributes and methods */
    void* dl;               /**<  dynamic shared object */
} npu_net_t;


/** @brief Struct of performance
 */
typedef struct {
    unsigned int elapsed_in_us;      /**<  inference time in micro-sec */
    unsigned int dma;                /**<  ADDR_NPU_PERF_DMA reg value */
    unsigned int comp;               /**<  ADDR_NPU_PERF_COMP reg value*/
    unsigned int all;                /**<  ADDR_NPU_PERF_ALL reg value */
} npu_perf_t;

/** @brief Struct of ECC and WDT
 */
enum {
    TELECHIPS_NPU_ECC_FAIL_ENABLE       = 0x0,
    TELECHIPS_NPU_ECC_FAIL_DISABLE      = 0x7,
    TELECHIPS_NPU_ECC_FAIL_DISABLE_SRAM = 0x1,
    TELECHIPS_NPU_ECC_FAIL_DISABLE_GBUF = 0x2,
    TELECHIPS_NPU_ECC_FAIL_DISABLE_CBUF = 0x4,
};

/** @brief enum for npu run mode
 */
typedef enum {
    NPU_RUN_SYNC       = 0x0,
    NPU_RUN_ASYNC      = 0x1
} npu_run_mode_t;


/** @brief Struct of chip test. Debug Only
 */
typedef struct {
    unsigned int mlx_bin_idx;           /**<  MLX test bin idx 1~8            */   
    unsigned int wdt_ext_cnt;           /**<  WDT timeout count               */   
    unsigned int wdt_int_cnt;           /**<  WDT rearm   count               */
    unsigned int ecc_test_ctrl;         /**<  [26:18]  err_bit pos1           */
                                        /**<  [16: 8]  err_bit pos0           */
                                        /**<  [ 7: 4]  ld_mask_gbuf           */
    unsigned int mlx_err_inj_mask_data; /**<  MLX err injection mask to data  */   
    unsigned int mlx_err_inj_mask_par;  /**<  MLX err injection mask to parity*/   
} npu_chiptest_t;

typedef struct {
    unsigned int disable_ue_fail;           /**< disable fail by ue  */
    unsigned int disable_ce_fail;           /**< disable fail by ce  */
    unsigned int disable_wdt;               /**< disable WDT timeout */
} npu_ecc_wdt_cfg_t;

/** @brief Open NPU device
 *
 *  @param[in]  minor NPU device cluster number. 0 or 1 for minor is allowed
 *  @return     npu   NPU device handle or NULL if failed
 */
npu_t* npu_open(int minor);

/** @brief Reset NPU device
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  sw_reset           0: NPU hard reset, 1: NPU soft reset
 *  @param[in]  ecc_pram         ECC, WDT disable param
 *  @return     ret  NPU device reset status. 0 for OK, NOK otherwise
 */
int npu_reset(
    npu_t* npu, 
    unsigned int soft_reset,
    npu_ecc_wdt_cfg_t* param
);

/** @brief Close NPU device
 *
 *  @param[in]  npu  NPU device handle
 *  @return     ret  NPU device reset status. 0 for OK, NOK otherwise
 */
int npu_close(npu_t* npu);

/** @brief Write data to NPU device register
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  addr write addr
 *  @param[in]  data write data
 *  @return     ret  NPU device write status. 0 for OK, NOK otherwise
 */
int npu_write_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int    data);

/** @brief Read data from NPU device register
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  addr        read addr
 *  @param[in]  data        read data
 *  @return     ret         NPU device read status. 0 for OK, NOK otherwise
 */
int npu_read_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int*   data);

/** @brief Write npu chiptest config param
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  cfg_param   chiptest config param
 *  @return     ret         NPU read status. 0 for OK, NOK otherwise
 */
int npu_write_test_cfg(
    npu_t* npu, 
    npu_chiptest_t* cfg_param
);

/** @brief Read data from NPU device register
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  err_status  NPU ECC, WDT status
 *  @return     ret         NPU read status. 0 for OK, NOK otherwise
 */
int npu_read_err_status(
    npu_t*              npu,
    npu_err_rd_req_t*   err_status
);


// NETWORK
/** @brief Load network to NPU DMA buffer
 *  @param[in]  npu         NPU device handle
 *  @param[in]  sofile      network shared object file name
 *  @param[in]  cmdfile     network command file name
 *  @param[in]  paramfile   network parameter file name
 *  @return     net         loaded network handle, NULL if failed.
 */
npu_net_t* network_load_from_file(
    npu_t*      npu,
    char*       sofile,
    char*       cmdfile,
    char*       paramfile);

/** @brief Load network cmd, param to NPU DMA buffer
 *
 *  @param[in]  npu         NPU device handle
 *  @param[in]  cmd         npu cmd buffer base
 *  @param[in]  cmd_size    npu cmd buffer size
 *  @param[in]  param       npu parameter buffer base
 *  @param[in]  param_size  npu parameter buffer size
 *  @return     net         loaded network handle or NULL if failed.
 */
npu_net_t* network_load(
    npu_t*      npu,
    char*       cmd,
    size_t      cmd_size,
    char*       param,
    size_t      param_size);

/** @brief Set input image color format
 *
 *  @param[in]  net   network handle
 *  @param[in]  color color format, NPU_COLOR_RGB, NPU_COLOR_YUV
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_set_color_format(
    npu_net_t*  net,
    int         color_format);

/** @brief Run NPU to do inference and wait until inference done
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  status   ECC, WDT status
 *  @param[in]  mode   select npu running mode sync/async
 *  @param[in]  perf   performance report buffer handle
 *  @param[in]  timeout_in_ms
 *                    inference time out in ms
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_run_mode_t  mode,
    npu_perf_t*     perf,
    int             timeout_in_ms);

/** @brief  Run NPU to do inference
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  status   ECC, WDT status
 *  @param[in]  perf   performance report buffer handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run_async(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_perf_t*     perf,
    int             timeout_in_ms);

/** @brief  Run NPU to do inference
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  status   ECC, WDT status
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_issue_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status);

/** @brief  Wait for inference done
 *
 *  @param[in]  net   network handle
 *  @param[in]  perf   performance report buffer handle
 *  @param[in]  timeout_in_ms
 *                    inference time out in ms
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_wait_done(
    npu_net_t*  net,
    npu_perf_t* perf,
    int         timeout_in_ms);

/** @brief Run NPU to do inference for performance profiling
 *
 *  @param[in]  net   network handle
 *  @param[in]  in    input image buffer handle
 *  @param[in]  out   output buffer handle
 *  @param[in]  status   ECC, WDT status
 *  @param[in]  perf  performance report buffer handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run_sync(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_perf_t* perf);

/** @brief Get network input image buffer size
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_size(npu_net_t* net);

/** @brief Get network output buffer size
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_output_size(npu_net_t* net);

/** @brief Get network input image width
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_width(npu_net_t* net);

/** @brief Get network input image height
 *
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_input_height(npu_net_t* net);

/** @brief Get network post-process type
 *
 *          post-process types = {
 *              TELECHIPS_NPU_POST_NONE       = 0
 *              TELECHIPS_NPU_POST_CLASSIFIER = 1
 *              TELECHIPS_NPU_POST_DETECTOR   = 2
 *              TELECHIPS_NPU_POST_CUSTOM     = 3
 *          }
 *  @param[in]  net   network handle
 *  @return     ret   valid if ret > 0,
 *                    error if ret <= 0.
 */
int network_get_type(npu_net_t* net);

/** @brief Run network post-process
 *
 *  @param[in]  net   network handle
 *  @param[in]  out   output buffer handle
 *  @param[out] result   post-process output
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_run_postprocess(
    npu_net_t*  net,
    npu_buf_t*  out,
    void*       result);

/** @brief Close network
 *
 *  @param[in]  net   network handle
 *  @return     ret   status. 0 for OK, NOK otherwise
 */
int network_close(npu_net_t* net);


// BUFFER
/** @brief Alloc NPU buffer
 *
 *  @param[in]  npu  NPU device handle
 *  @param[in]  size buffer size
 *
 *  @return     NPU buffer handle
 */
npu_buf_t* buffer_alloc(npu_t* npu, int size);

/** @brief Release NPU buffer
 *
 *  @param[in]  npu  NPU device handle
 *
 *  @return     close status. 0 for OK, NOK otherwise
 */
int buffer_close(npu_buf_t* buf);

/** @brief Get address of NPU buffer
 *
 *  @param[in]  npu  NPU buffer handle
 *
 *  @return     NPU buffer addr, or NULL in case of error
 */
char* buffer_get_addr(npu_buf_t* buf);

/*
    npu_api_mem.c
*/

/** @brief NPU API memory allocation
 *
 *  @param[in]  size  buffer handle
 *  @return     addr  memory base addr
 */
char* npu_api_malloc(size_t size);

/** @brief NPU API memory free
 *
 *  @param[in]  addr  memory base addr
 */
void npu_api_free(void* addr);

/** @brief Memory allocation of npu_buf_t
 *
 *  @return     addr  memory base addr
 */
npu_buf_t* npu_buf_malloc(void);

/** @brief Memmory free of npu_buf_t
 *
 *  @param[in]  addr  memory base addr
 */
void npu_buf_free(void* addr);

/** @brief Memory allocation of npu_net_t
 *
 *  @return     addr  memory base addr
 */
npu_net_t* npu_net_malloc(void);

/** @brief Memmory free of npu_net_t
 *
 *  @param[in]  addr  memory base addr
 */
void npu_net_free(void* addr);

/** @brief Memory allocation of struct enlight_net
 *
 *  @return     addr  memory base addr
 */
struct enlight_net* enlight_net_malloc(void);

/** @brief Memmory free of struct enlight_net
 *
 *  @param[in]  addr  memory base addr
 */
void enlight_net_free(void* addr);

/** @brief Struct of single object
 */
typedef struct {
    float           x_min;      /**< object box position        */
    float           x_max;      /**< object box position        */
    float           y_min;      /**< object box position        */
    float           y_max;      /**< object box position        */
    int16_t         cls;        /**< object class               */
    float           score;      /**< object confidence score    */
    int16_t         img_w;      /**< input image width          */
    int16_t         img_h;      /**< input image height         */
} enlight_obj_t;

/** @brief Struct of detected objects,
 *              output of object detection network's post-process
 */
typedef struct {
    int cnt;                    /**< number of detected objects */
    enlight_obj_t   obj[256];   /**< MAX_DETECT_BOX=256         */
                                /**< detected objs description  */
} enlight_objs_t;


/** @brief Struct of classification
 *              output of classificatton network's post-process
 */
typedef struct {
    int class_ids[8];           /**< MAX_BATCH_NUM=8            */
                                /**< detected cls description   */
} enlight_batch_cls_t;

/** @brief Struct of custom network output
 *      FIXME.
 *      the struct of custom network output should be modified
 *      according to custome network post processing
 */
typedef struct {
    int output_data;
} enlight_custom_t;

/** @brief Enum of logistic type
 */
typedef enum {
    TELECHIPS_NPU_POST_NONE,          /**< post-process : not defined */
    TELECHIPS_NPU_POST_CLASSIFIER,    /**< post-process : classifier  */
    TELECHIPS_NPU_POST_DETECTOR,      /**< post-process : detector    */
    TELECHIPS_NPU_POST_CUSTOM,        /**< post-process : unknwon     */
} enlight_postproc_t;

/** @brief Struct of network
 */
struct enlight_net {
    int         fd;
    int         cmd_size;       /**< command buffer size        */
    int         weight_size;    /**< weight buffer size         */
    int         work_size;      /**< work buffer size           */
    int         input_size;     /**< input buffer size          */
    int         output_size;    /**< output buffer size         */
    int         post_type;      /**< post-process type          */
    int*        img_size;       /**< input dimension            */
    long long   conv_mac_num;   /**< convolution MAC number     */
    int         batch_size;     /**< batch size                 */
    char        name[128];      /**< network name               */

    int         (*get_buffer_size)(int idx);
    int*        (*get_image_size)(void);
    char*       (*get_network_name)(void);
    int         (*get_post_type)(void);
    int         (*run_post)(void* output_base, int batch_size, void* post_output);
    int         (*get_output_tensor_num)(void);
    int         (*get_output_tensor)(int idx);
    int         (*get_tensor_size)(int idx);
    int         (*get_tensor_offset)(int idx);
    char*       (*get_tensor_name)(int idx);
};

#endif //ENLIGHT_NPU_API
