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

/**
 * @file    npu_api_mem.c
 * @brief   NPU APIs for memory allocation and free
 *
 *              char*       npu_api_malloc(int size);
 *              void        npu_api_free(void *addr);
 *              npu_buf_t*  npu_buf_malloc(void);
 *              void        npu_buf_free(void* addr);
 *              npu_net_t*  npu_net_malloc(void);
 *              void        npu_net_free(void *addr);
 *              struct enlight_net*
 *                          enlight_net_malloc(void);
 *              void        enlight_net_free(void *addr);
 *
 *          NPU APIs programmer can implement static memory allocation
 *          by means of replacing malloc/free of stdlib.h
 */

#include <stdlib.h>
#include "npu_api.h"

char* npu_api_malloc(size_t size)
{
    // npu_api_fin();
    void* ptr = malloc(size);
    // if(ptr == NULL) {
    //     // telechips_print("fail of allocation\n");
    //     return NULL;
    // }
    // npu_api_fout();

    return (char*)ptr;
}

void npu_api_free(void *addr)
{
    // npu_api_fin();

    free(addr);

    // npu_api_fout();
}

npu_buf_t* npu_buf_malloc(void)
{
    // npu_api_fin();

    // addr = (npu_buf_t*)malloc(sizeof(npu_buf_t));
    void* ptr = malloc(sizeof(npu_buf_t));
    // if(ptr == NULL) {
    //     // telechips_print("fail of allocation\n");
    //     return NULL;
    // }

    // npu_api_fout();

    return (npu_buf_t*)ptr;
}

void npu_buf_free(void* addr)
{
    // npu_api_fin();

    free(addr);

    // npu_api_fout();
}

npu_net_t* npu_net_malloc(void)
{
    // npu_api_fin();

    // addr = (npu_net_t*)malloc(sizeof(npu_net_t));
    void* ptr = malloc(sizeof(npu_net_t));
    // if(ptr == NULL) {
    //     // telechips_print("fail of allocation\n");
    //     return NULL;
    // }

    // npu_api_fout();

    return (npu_net_t*)ptr;
}

void npu_net_free(void *addr)
{
    // npu_api_fin();

    free(addr);

    // npu_api_fout();
}

struct enlight_net* enlight_net_malloc(void)
{
    // npu_api_fin();

    // addr = (struct enlight_net *)malloc(sizeof(struct enlight_net));
    void* ptr = malloc(sizeof(struct enlight_net));
    // if(ptr == NULL) {
    //     // telechips_print("fail of allocation\n");
    //     return NULL;
    // }

    // npu_api_fout();

    return (struct enlight_net *)ptr;
}

void enlight_net_free(void *addr)
{
    // npu_api_fin();

    free(addr);

    // npu_api_fout();
}
