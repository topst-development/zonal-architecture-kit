// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : sal_freertos_impl.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                             [INCLUDE FILES]
***************************************************************************************************
*/
//#include <FreeRTOS.h>
//#include <task.h>
//#include <semphr.h>
//#include <event_groups.h>
#include <stdio.h> //QAC-<stdio.h> shall not be used in production code, but freertos recommends using <stdio.h>
#include <string.h>

#include <sal_impl.h>

//#include <version.h>

/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/
/*
 * Debugger
 */
static uint32                           gCPU_SR = 0;


/*
***************************************************************************************************
*                                          [SAL_CoreDiv64To32]
*
*
* @param    pullDividend [in]
* @param    uiDivisor [in]
* @param    puiRem [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t SAL_CoreDiv64To32
(
    uint64 * pullDividend,
    uint32 uiDivisor,
    uint32 * puiRem
) {
    SALRetCode_t retVal     = SAL_RET_SUCCESS;
    uint64 rem              = 0;
    uint64 b                = uiDivisor;
    uint64 d                = 1;
    uint64 res              = 0;
    uint32 high             = 0;

    if (pullDividend != NULL_PTR)
    {
        rem = *pullDividend;
        high = (uint32)(rem >> 32ULL);

        /* Reduce the thing a bit first */
        if (high >= uiDivisor)
        {
            high /= uiDivisor;
            res = ((uint64)high) << 32ULL;

            // CERT-C Integers (CERT INT30-C) : Ensure that unsigned integer operations do not wrap
            if ((uiDivisor > 0UL) && ((rem / (uint64)uiDivisor) >= (uint64)high))
            {
#if 0
                retVal = FR_ReportError(SAL_DRVID_SAL,
                                        SAL_API_CORE_DIV_64_32,
                                        SAL_ERR_INVALID_PARAMETER,
                                        __FUNCTION__);
#endif
            }
            else
            {
                rem -= (((uint64)high * (uint64)uiDivisor) << 32ULL);
            }
        }

        if (retVal == SAL_RET_SUCCESS)
        {
            while (((b > 0ULL) && (b < rem)))
            {
                b = b+b;
                d = d+d;
            }

            do
            {
                if (rem >= b)
                {
                    rem -= b;

                    if ((0xFFFFFFFFFFFFFFFFULL - d) < res)
                    {
#if 0

                        retVal = FR_ReportError(SAL_DRVID_SAL,
                                                SAL_API_CORE_DIV_64_32,
                                                SAL_ERR_INVALID_PARAMETER,
                                                __FUNCTION__);
#endif
                        break;
                    }
                    else
                    {
                        res += d;
                    }
                }

                b >>= 1UL;
                d >>= 1UL;
            }
            while (d != 0ULL);

            if (retVal == SAL_RET_SUCCESS)
            {
                *pullDividend = res;
            }
        }
     }
    else
    {
#if 0
        retVal = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_CORE_DIV_64_32,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
#endif
    }

    if (puiRem != NULL_PTR)
    {
        *puiRem = (uint32)rem;
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [SAL_MemSet]
*
*
* @param    pMem [out]
* @param    ucValue [in]
* @param    uiSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t SAL_MemSet
(
    void * pMem,
    uint8 ucValue,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if (pMem != NULL_PTR)
    {
        (void)memset(pMem, (int32)ucValue, (size_t)uiSize); //QAC-Not use return value
    }
    else
    {
#if 0
        retVal = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_MEM_SET,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
#endif
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [SAL_MemCopy]
*
*
* @param    pDest [out]
* @param    pSrc [in]
* @param    uiSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
// Deviation Record - HIS metric violation (HIS_CALLING)
SALRetCode_t SAL_MemCopy
(
    void * pDest,
    const void * pSrc,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pDest != NULL_PTR) && (pSrc != NULL_PTR))
    {
        (void)memcpy(pDest, pSrc, (size_t)uiSize); //QAC-Not use return value
    }
    else
    {
#if 0
        retVal = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_MEM_COPY,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
#endif
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [SAL_MemCmp]
*
*
* @param    pMem1 [in]
* @param    pMem2 [in]
* @param    uiSize [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t SAL_MemCmp
(
    const void * pMem1,
    const void * pMem2,
    SALSize uiSize,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pMem1 != NULL_PTR) && (pMem2 != NULL_PTR) && (uiSize > 0UL) && (piRetCmp != NULL_PTR))
    {
        *piRetCmp = (sint32)memcmp(pMem1, pMem2, (size_t)uiSize);
    }
    else
    {
#if 0
        retVal = FR_ReportError(SAL_DRVID_SAL,
                                SAL_API_MEM_CMP,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
#endif
    }

    return retVal;
}

SALRetCode_t SAL_CoreMB (void)
{
    CPU_DSB();

    return SAL_RET_SUCCESS;
}

SALRetCode_t SAL_CoreCriticalEnter (void)
{
    gCPU_SR = 0;
    gCPU_SR = CPU_SR_Save();

    return SAL_RET_SUCCESS;
}

SALRetCode_t SAL_CoreCriticalExit (void)
{
    CPU_SR_Restore(gCPU_SR);

    return SAL_RET_SUCCESS;
}

SALRetCode_t SAL_TaskSleep (uint32 uiMilliSec)
{
    SALRetCode_t            retval = SAL_RET_SUCCESS;
    uint32                  uiCnt = 0;

    for( uiCnt = 0; uiCnt < ( uiMilliSec * 10000); uiCnt++ )
    {
        ;
    }

    return retval;
}

SALRetCode_t SAL_DbgReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId,
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
)
{
    /* nothing to do */

    return SAL_RET_FAILED;
}

