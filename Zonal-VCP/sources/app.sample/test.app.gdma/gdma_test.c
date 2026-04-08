// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gdma_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

#include "gdma_test.h"
#include "gdma.h"
#include "clock.h"
#include <gic.h>
#include <debug.h>
#include <mpu.h>

#if (DEBUG_ENABLE)
#define GDMA_DBG(fmt, args...)          {LOGD(DBG_TAG_GDMA, fmt, ## args)}
#else
#define GDMA_DBG(fmt, args...)
#endif

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/


 /*
***************************************************************************************************
*                                          GDMA_SampleForM2M
*
* Sample test function for transferring data using dma by memory to memory scheme.
*
* @param    none.
* @return
*
* Notes
*
***************************************************************************************************
*/

static GDMAInformation_t sample_pGdmaControl;
void GDMA_SampleForM2M
(
    void
)
{
    uint32 request_size =  (1024 + (4*16) + 3);
    uint32 src_base = MPU_GetDMABaseAddress();
    uint32 dest_base = (MPU_GetDMABaseAddress() + 0x1000UL);
    //GDMAControlData_t sample_pGdmaControl;
    sint8   * pSrcPtr;
    sint8   * pDstPtr;
    sint32 ret = 0;
    uint32 i;
    sample_pGdmaControl.iCon = 0;
    sample_pGdmaControl.iCh= 0;

    GDMA_DBG("\ngdma_m2m_sample test start \n");
    //make source pattern & clear destination memory by 0
    pSrcPtr = (sint8 *)(src_base);
    pDstPtr = (sint8 *)(dest_base);
    for(i =0; i < request_size; i++)
    {
        *(pSrcPtr+i) = (sint8)127;//0x7F
        *(pDstPtr+i) = (sint8)0;
    }
    // initialize GDMA CH0
    GDMA_DBG("GDMA_Initialize call start \n");
    ret= GDMA_Init(&sample_pGdmaControl, GIC_PRIORITY_NO_MEAN);

    if (ret == 0)
    {
        GDMA_DBG("GDMA_Initialize ret = %d Success \n", ret);

        GDMA_DBG("gdma_transfer_m2m call M2M source %x to destination %x requested bytes %d \n",src_base, dest_base, request_size );
        // Requst GDMA Momory To Momory Transfer.
        (void)GDMA_TransferMem2Mem(&sample_pGdmaControl, src_base, dest_base, request_size);
        GDMA_DBG("compare souce and destination memory\n");
        // compare souce and destination memory
        for(i =0; i < request_size; i++)
        {
            if (*(pSrcPtr+i) != *(pDstPtr+i))
            {
                break;
            }
        }

        if ( i == request_size)
        {
            GDMA_DBG("GDMA M2M Verify Success \n");
        }
        else
        {
            GDMA_DBG("GDMA M2M Verify  Fail \n");
        }
    }
}

#endif // ( MCU_BSP_SUPPORT_TEST_APP_GDMA == 1 )

