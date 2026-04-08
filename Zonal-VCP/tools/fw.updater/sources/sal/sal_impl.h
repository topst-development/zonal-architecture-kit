// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : sal_freertos_impl.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#ifndef SAL_IMPL_HEADER
#define SAL_IMPL_HEADER

/*
***************************************************************************************************
*                                             [INCLUDE FILES]
***************************************************************************************************
*/

//#include <sal_internal.h>
#include <sal_com.h>


/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/

#ifdef __cplusplus
extern "C"
{
#endif

/*
***************************************************************************************************
*                                         [FUNCTION PROTOTYPES]
***************************************************************************************************
*/
extern SALRetCode_t SAL_CoreDiv64To32
(
    uint64 *                            pullDividend,
    uint32                              uiDivisor,
    uint32 *                            puiRem
);

extern SALRetCode_t SAL_MemSet
(
    void *                              pMem,
    uint8                               ucValue,
    SALSize                             uiSize
);

extern SALRetCode_t SAL_MemCopy
(
    void *                              pDest,
    const void *                        pSrc,
    SALSize                             uiSize
);

extern SALRetCode_t SAL_MemCmp
(
    const void *                        pMem1,
    const void *                        pMem2,
    SALSize                             uiSize,
    sint32 *                            piRetCmp
);

extern SALRetCode_t SAL_CoreMB
(
    void
);

extern SALRetCode_t SAL_CoreCriticalEnter
(
    void
);

extern SALRetCode_t SAL_CoreCriticalExit
(
    void
);

extern SALRetCode_t SAL_TaskSleep
(
    uint32 uiMilliSec
);

SALRetCode_t SAL_DbgReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId,
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
);


/* Defined on ASM */
extern void CPU_DSB
(
    void
);

extern uint32 CPU_SR_Save
(
    void
);

extern void CPU_SR_Restore
(
    uint32 uiStatus
);

extern void CPU_WaitForEvent
(
    void
);

#ifdef __cplusplus
}
#endif

#endif // SAL_FREERTOS_IMPL_HEADER

