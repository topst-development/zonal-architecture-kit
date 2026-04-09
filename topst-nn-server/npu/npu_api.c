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

#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>

#include "npu.h"
#include "npu_api.h"


//-------------------------------------------------------------------
// NPU APIs
//-------------------------------------------------------------------
#define NUM_NPU_DEV 2

npu_t* npu_open(int minor)
{
    static npu_t npu_devs[NUM_NPU_DEV];

    npu_t* npu;

    // npu_api_fin();

    if ((minor == 0) || (minor == 1)) {
        npu = &npu_devs[minor];

        if (minor == 0) {
            npu->fd = open("/dev/npu0", O_RDWR);
        }
        else {
            npu->fd = open("/dev/npu1", O_RDWR);
        }

        if (npu->fd < 0) {
            npu = NULL;
        }
    }
    else {
        npu = NULL;
    }

    // if (npu != NULL) {
    //     npu_api_print_err("failed: %d\n", minor);
    // }

    // npu_api_fout();

    return npu;
}

int npu_reset(
    npu_t* npu, 
    unsigned int soft_reset,
    npu_ecc_wdt_cfg_t* param
)
{
    npu_init_req_t req;

    int ret;

    // npu_api_fin();

    req.soft_reset = soft_reset; 

    if (param == NULL) {
        req.disable_ue_fail = 0;
        req.disable_ce_fail = 0;
        req.disable_wdt = 0;
    }
    else {
        req.disable_ue_fail = param->disable_ue_fail;
        req.disable_ce_fail = param->disable_ce_fail;
        req.disable_wdt     = param->disable_wdt;
    }

    if ((npu == NULL) || (npu->fd < 0)) {
        ret = -1;
    }
    else {
        if (ioctl(npu->fd, NPU_IOCTL_RESET_NPU, &req) == 0) {
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int npu_close(npu_t* npu)
{
    int ret;

    // npu_api_fin();

    if ((npu == NULL) || (npu->fd < 0)) {
        ret = -1;
    }
    else {
        if (close(npu->fd) == 0) {
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int npu_write_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int    data
)
{
    int ret;

    // npu_api_fin();

    if ((npu == NULL) || (npu->fd < 0)) {
        ret = -1;
    }
    else {
        reg_access_req_t req;
        req.addr = addr;
        req.data = data;

        if (ioctl(npu->fd, NPU_IOCTL_WRITE_REG, &req)==0) {
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int npu_read_reg(
    npu_t*          npu,
    unsigned int    addr,
    unsigned int*   data
)
{
    int ret;

    // npu_api_fin();

    if ((npu == NULL) || (npu->fd < 0) || (data == NULL)) {
        ret = -1;
    }
    else {
        reg_access_req_t req;
        req.addr = addr;

        if (ioctl(npu->fd, NPU_IOCTL_READ_REG, &req) == 0) {
            *data = req.data;
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int npu_write_test_cfg (
    npu_t* npu, 
    npu_chiptest_t* cfg_param
)
{
    test_cfg_wr_req_t req;

    int ret;

    // npu_api_fin();

    if (cfg_param == NULL) {
        req.mlx_bin_idx = 0;
        req.wdt_ext_cnt = 0;
        req.wdt_int_cnt = 0;
        req.ecc_test_ctrl = 0;
        req.mlx_err_inj_mask_data = 0;
        req.mlx_err_inj_mask_par  = 0;
    }
    else {
        req.mlx_bin_idx           = cfg_param->mlx_bin_idx;
        req.wdt_ext_cnt           = cfg_param->wdt_ext_cnt;
        req.wdt_int_cnt           = cfg_param->wdt_int_cnt;
        req.ecc_test_ctrl         = cfg_param->ecc_test_ctrl;
        req.mlx_err_inj_mask_data = cfg_param->mlx_err_inj_mask_data;
        req.mlx_err_inj_mask_par  = cfg_param->mlx_err_inj_mask_par;
    }

    if ((npu == NULL) || (npu->fd < 0)) {
        ret = -1;
    }
    else {
        if (ioctl(npu->fd, NPU_IOCTL_WRITE_TEST_CFG, &req) == 0) {
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int npu_read_err_status(
    npu_t*          npu,
    npu_err_rd_req_t* err_status 
)
{
    int ret;

    // npu_api_fin();
    if ((npu == NULL) || (npu->fd < 0) || (err_status == NULL)) {
        ret = -1;
    }
    else {
        ret = ioctl(npu->fd, NPU_IOCTL_READ_NPU_ERR, err_status);
    }

    // if (ret != 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

//-------------------------------------------------------------------
// NPU Network
//-------------------------------------------------------------------
npu_net_t* network_load(
    npu_t*  npu,
    char*   cmd,
    size_t  cmd_size,
    char*   param,
    size_t  param_size
)
{
    npu_net_t* net;

    // npu_api_fin();

    if ((npu == NULL) || (npu->fd < 0) || (cmd == NULL) || (param == NULL)) {
        net = NULL;
    }
    else {
        int fd;
        net_load_req_t req;

        req.cmd_data = cmd;
        req.cmd_size = cmd_size;

        req.wei_data = param;
        req.wei_size = param_size;

        fd = ioctl(npu->fd, NPU_IOCTL_LOAD_NETWORK, &req);

        if (fd >= 0) {
            //net = npu_api_mem_alloc(sizeof(npu_net_t));
            net = npu_net_malloc();
            if (net == NULL) {
                (void)close(fd);
            } else {
                net->fd = fd;
                net->methods = NULL;
                net->dl = NULL;
            }
        }
        else {
            net = NULL;
        }
    }

    // if (net == NULL) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return net;
}

int network_set_color_format(
    npu_net_t*  net,
    int         color_format
)
{
    int ret;

    // npu_api_fin();

    if ((net == NULL) || (net->fd < 0) || (color_format >= NPU_COLOR_END)) {
        ret = -1;
    }
    else {
        ret = ioctl(net->fd, NPU_NET_IOCTL_SET_COLOR_FMT, color_format);
        if (ret == 0) {
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_run_mode_t  mode,
    npu_perf_t*     perf,
    int             timeout_in_ms
)
{
    int ret = 0;

    if (mode == NPU_RUN_SYNC) {
        ret = network_run_sync(net, in, out, status, perf);
    }
    else {// mode == ASYNC
        ret = network_run_async(net, in, out, status, perf, timeout_in_ms);
    }

    return ret;
}


int network_run_async(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_perf_t*     perf,
    int         timeout_in_ms
)
{
    int ret;

    // npu_api_fin();
    //TODO: remove a spend time calculate in application level.
    struct timespec start, end;
    long long time_start, time_end, time_diff;

    clock_gettime(CLOCK_MONOTONIC, &start);
    ret = network_issue_run(net, in, out, status);

    if (ret == 0) {
        ret = network_wait_done(net, perf, timeout_in_ms);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    time_start = (long long)start.tv_sec * 1000000000LL + start.tv_nsec;
    time_end = (long long)end.tv_sec * 1000000000LL + end.tv_nsec;
    time_diff = time_end - time_start;

    perf->elapsed_in_us = (time_diff / 1000L);
    perf->dma = 0u;
    perf->comp = 0u;

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_issue_run(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status
)
{
    int ret;

    // npu_api_fin();

    if ((net == NULL) || (net->fd < 0) ||
        (in == NULL) || (in->fd < 0) ||
        (out == NULL) || (out->fd < 0)) {
        ret = -1;
    }
    else {
        net_run_req_t req;
        req.in_fd = in->fd;
        req.out_fd = out->fd;
        req.err_status = status;

        if (ioctl(net->fd, NPU_NET_IOCTL_RUN, &req) == 0){
            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_wait_done(
    npu_net_t*  net,
    npu_perf_t* perf,
    int         timeout_in_ms
)
{
    int ret;
    (void) perf;

    // npu_api_fin();

    if ((net == NULL) || (net->fd < 0) || (timeout_in_ms < 0)) {
        ret = -1;
    }
    else {
        struct pollfd fd;
        int poll_ret;

        fd.fd = net->fd;
        fd.events = (short int)((uint8_t)POLLIN | (uint8_t)POLLERR);
        fd.revents = 0;

        do {
            poll_ret = poll(&fd, 1, timeout_in_ms);
        // cppcheck-suppress misra-c2012-22.10; poll is errno setting func
        } while ((poll_ret < 0));

        if ((fd.revents & POLLIN) == 0) {
            ret = -1;
        }
        else {
            //TODO: for official release
            // net_current_state_req_t req;
            // if (ioctl(net->fd, NPU_NET_GET_LASTEST_STATUS, &req) == 0) {
            //     perf->elapsed_in_us = req.elapsed_in_us;
            //     // perf->dma = req.dma;
            //     // perf->comp = req.comp;
            //     // perf->all = 0u;
            //     ret = 0;
            // }
            // else {
            //     ret = -1;
            // }
            ret = 0;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_run_sync(
    npu_net_t*  net,
    npu_buf_t*  in,
    npu_buf_t*  out,
    npu_err_bits_t* status,
    npu_perf_t* perf
)
{
    int ret;

    // npu_api_fin();

    if ((net == NULL) || (net->fd < 0) ||
        (in == NULL) || (in->fd < 0) ||
        (out == NULL) || (out->fd < 0) ||
        (perf == NULL)) {
        ret = -1;
    }
    else {
        net_profile_req_t req;
        req.in_fd = in->fd;
        req.out_fd = out->fd;
        req.err_status = status;

        if (ioctl(net->fd, NPU_NET_IOCTL_PROFILE, &req) == 0) {
            perf->elapsed_in_us = req.elapsed_in_us;
            perf->dma = req.dma;
            perf->comp = req.comp;
            perf->all = req.all;

            ret = 0;
        }
        else {
            ret = -1;
        }
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_get_input_size(npu_net_t* net)
{
    int ret;

    if ((net == NULL) || (net->methods == NULL)) {
        ret = -1;
    }
    else {
        ret = net->methods->input_size;
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    return ret;
}

int network_get_output_size(npu_net_t* net)
{
    int ret;

    if ((net == NULL) || (net->methods == NULL)) {
        ret = -1;
    }
    else {
        ret = net->methods->output_size;
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed/n");
    // }

    return ret;
}

int network_get_input_width(npu_net_t* net)
{
    int ret;

    if ((net == NULL) || (net->methods == NULL)) {
        ret = -1;
    }
    else {
        ret = net->methods->img_size[2];
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed/n");
    // }

    return ret;
}

int network_get_input_height(npu_net_t* net)
{
    int ret;

    if ((net == NULL) || (net->methods == NULL)) {
        ret = -1;
    }
    else {
        ret = net->methods->img_size[1];
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    return ret;
}

int network_get_type(npu_net_t* net)
{
    int ret;

    if ((net == NULL) || (net->methods == NULL)) {
        ret = -1;
    }
    else {
        ret = net->methods->post_type;
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    return ret;
}

int network_run_postprocess(
    npu_net_t* net,
    npu_buf_t* out,
    void* result
)
{
    int ret;

    // npu_api_fin();

    if ((net == NULL) || (net->methods == NULL) ||
        (out == NULL) || (out->fd < 0) ||
        (result == NULL)) {
        ret = -1;
    }
    else {
        int batch_size = net->methods->batch_size;
        char* buf = buffer_get_addr(out);

        (void)net->methods->run_post(buf, batch_size, result);

        ret = 0;
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

int network_close(npu_net_t* net)
{
    int ret;

    // npu_api_fin();

    if ((net == NULL) || (net->fd < 0) || (net->dl == NULL)) {
        ret = -1;
    }
    else {

        if (close(net->fd) == 0) {
            if(dlclose(net->dl) == 0) {
                ret = 0;
            }
            else {
                ret = -1;
            }
        }
        else {
            ret = -1;
        }

        enlight_net_free(net->methods);
        npu_net_free(net);
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

//-------------------------------------------------------------------
// NPU Buffer
//-------------------------------------------------------------------
npu_buf_t* buffer_alloc(npu_t* npu, int size)
{
    npu_buf_t* buf;

    // npu_api_fin();

    if ((npu == NULL) || (npu->fd < 0) || (size <= 0)) {
        buf = NULL;
    }
    else {
        //buf = npu_api_mem_alloc(sizeof(npu_buf_t));
        buf = npu_buf_malloc();

        if (buf != NULL) {
            int fd;

            buf_alloc_req_t req;
            req.size = size;

            fd = ioctl(npu->fd, NPU_IOCTL_ALLOC_BUFFER, &req);

            if (fd >= 0) {
                buf->fd = fd;
                buf->size = size;
                size_t alloc_memory_size = (size_t)size;
                buf->caddr = (char*)mmap((void*)0,
                                     alloc_memory_size,
                                     (int)((unsigned int)PROT_READ| (unsigned int)PROT_WRITE),
                                     MAP_SHARED,
                                     buf->fd,
                                     0);
                buf->paddr = req.addr;
            }
            else {
                npu_buf_free(buf);
                buf = NULL;
            }
        }
        // else {
        //     npu_api_print_err("npu_buf_malloc failed(%d)\n", size);
        // }
    }

    // if (buf == NULL) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return buf;
}

int buffer_close(npu_buf_t* buf)
{
    int ret;

    // npu_api_fin();

    if ((buf == NULL) || (buf->fd < 0)|| (buf->caddr == NULL) || (buf->size < 0)) {
        ret = -1;
    }
    else {
        size_t buf_size = (size_t)buf->size;
        int ret_val = 0;
        
        if(munmap(buf->caddr, buf_size) != 0) {
            // npu_api_print_err("fail to munmap addr=%d, size=%ld\n", buf->fd, buf_size);
            ret_val = -1;
        }
        if(close(buf->fd) != 0) {
            // npu_api_print_err("fail to close fd=%d\n", buf->fd);
            ret_val = -1;
        }
        npu_buf_free(buf);
        ret = ret_val;
    }

    // if (ret < 0) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return ret;
}

char* buffer_get_addr(npu_buf_t* buf)
{
    char* addr;

    // npu_api_fin();

    if ((buf == NULL) || (buf->fd < 0)) {
        addr = NULL;
    }
    else {
        addr = buf->caddr;
    }

    // if (addr == NULL) {
    //     npu_api_print_err("failed\n");
    // }

    // npu_api_fout();

    return addr;
}

