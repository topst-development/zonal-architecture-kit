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

#include <stdio.h>
#include "npu_api.h"

static inline unsigned int get_file_size(char *path)
{
    unsigned int size = 0;

    if (!access(path, F_OK)) {
        FILE *fp;
        fp = fopen(path, "rb");
        if (!fseek(fp, 0, SEEK_END)) {
            errno = 0;
            size = ftell(fp);
            if (errno != 0) {
                size = 0;
            }
        }
        (void)fclose(fp);
    }

    return size;
}

//-------------------------------------------------------------------
// NPU Network
//-------------------------------------------------------------------
npu_net_t* network_load_from_file(
    npu_t*  npu,
    char*   sofile,
    char*   cmdfile,
    char*   paramfile
)
{
    FILE* cmd_fp = NULL;
    char* cmd_buf = NULL;
    unsigned int c_size = get_file_size(cmdfile);

    FILE* param_fp = NULL;
    char* param_buf = NULL;
    unsigned int p_size = get_file_size(paramfile);

    npu_net_t* net = NULL;

    void* dl = NULL;
    // cppcheck-suppress misra-c2012-17.7
    int (*init_net)(struct enlight_net*);
    struct enlight_net* methods = NULL;

    npu_net_t* ret;
    int err = 0;

    //npu_api_fin();

    if ((!c_size) || (!p_size)) {
        err = -1;
    }
    else {
        err = 0;
    }

    if (!err) {
        // load command, parameter
        cmd_fp = fopen(cmdfile, "rb");
        param_fp = fopen(paramfile, "rb");
        if ((!cmd_fp) || (!param_fp)) {
            err = -1;
        }
    }

    if (!err) {
        cmd_buf = npu_api_malloc(sizeof(char)*c_size);
        param_buf = npu_api_malloc(sizeof(char)*p_size);
        if ( (!cmd_buf) || (!param_buf)) {
            err = -1;
        }
    }

    if (!err) {
        if (fread(cmd_buf, sizeof(char), c_size, cmd_fp) < c_size) {
            err = -1;
        }
        if (fread(param_buf, sizeof(char), p_size, param_fp) < p_size) {
            err = -1;
        }
    }

    if (cmd_fp != NULL) {
        (void)fclose(cmd_fp);
    }
    if (param_fp != NULL) {
        (void)fclose(param_fp);
    }

    if (!err) {
        // open network
        net = network_load(npu, cmd_buf, c_size, param_buf, p_size);
        if (!net) {
            err = -1;
        }
    }

    if (cmd_buf != NULL) {
        npu_api_free(cmd_buf);
    }
    if (param_buf != NULL) {
        npu_api_free(param_buf);
    }

    if (!err) {
        // open linked methods
        dl = dlopen(sofile, RTLD_NOW);
        if (!dl) {
            err = -1;
        }
        methods = enlight_net_malloc();
        if (!methods) {
            err = -1;
        }
    }

    if (!err) {
        init_net = dlsym(dl, "init_network");
        init_net(methods);

        net->methods = methods;
        net->dl = dl;

        ret = net;
    }
    else {
        if (net != NULL) {
           (void)network_close(net);
        }

        enlight_net_free(methods);

        if (dl != NULL) {
            dlclose(dl);
        }

        ret = NULL;
    }

    if (!ret) {
        //npu_api_print_err("failed");
    }

    //npu_api_fout();

    return ret;
}

