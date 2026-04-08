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
#include <os.h>
#include <cpu.h>
#include <os_cfg_app.h>
#include <bsp_os.h>

#include <sal_api.h>
#include <version.h>

#include <lib_def.h>
#ifdef SAL_MEM_FUNC_IMPL
#   include <lib_ascii.h>
#   include <lib_math.h>
#   include <lib_mem.h>
#endif

#ifdef SAL_STRING_FUN_IMPL
#   include <lib_str.h>
#endif

/*
***************************************************************************************************
*                                             [DEFINITIONS]
***************************************************************************************************
*/
/*
 * Debugger
 */
#if (DEBUG_ENABLE)
// Deviation Record - MISRA C-2012 Rule 20.10, Preprocessing Directives
#   include <debug.h>
#   define SAL_D(fmt, args...)          {LOGD(DBG_TAG_SAL, fmt, ## args)}
#   define SAL_E(fmt, args...)          {LOGE(DBG_TAG_SAL, fmt, ## args)}
#else
#   define SAL_D(fmt, args...)
#   define SAL_E(fmt, args...)
#endif

#ifndef SAL_MEM_FUNC_IMPL
typedef int                             UCInt32;/* For stdio api                        */
#endif

/* Task */
typedef struct SALTaskiInfoTable
{
    boolean                             tiUsed; /* if already used or not               */
    OS_TCB                              tiCb;   /* pointer to Task Control Block        */

} SALTask_t ;

/* queue */
typedef struct SALQueueInfoTable
{
    boolean                             qiUsed; /* if already used or not               */
    OS_Q                                qiCb;   /* pointer to Queue control block       */

} SALQueue_t;

/* semaphore */
typedef struct SALSemaphoreInfoTable
{
    boolean                             siUsed; /* if already used or not               */
    OS_SEM                              siCb;   /* pointer to Semaphore control block   */

} SALSemaphore_t;

/* event */
typedef struct SALEventInfoTable
{
    boolean                             eiUsed; /* if already used or not               */
    OS_FLAG_GRP                         eiCb;   /* pointer to Event control block        */

} SALEvent_t;

/*
***************************************************************************************************
*                                             [LOCAL VARIABLES]
***************************************************************************************************
*/
static uint64                           gSystemMainTickCnt = 26; //(BOOT_CHECK_ELAPSED_TIME) + (18)

static uint32                           gCPU_SR = 0;

static OS_SEM                           gSemObj;
static SALErrorInfo_t                   gErrorInfo;

/* OS object */
static SALTask_t                        gTaskObj[SAL_MAX_TASK];
static SALQueue_t                       gQueueObj[SAL_MAX_QUEUE];
static SALSemaphore_t                   gSemaObj[SAL_MAX_SEMA];
static SALEvent_t                       gEventObj[SAL_MAX_EVENT];

/*
***************************************************************************************************
*                                         [FUNCTION PROTOTYPES]
***************************************************************************************************
*/

static SALRetCode_t UC_CoreMB
(
    void
);

static SALRetCode_t UC_CoreCriticalEnter
(
    void
);

static SALRetCode_t UC_CoreCriticalExit
(
    void
);

static SALRetCode_t UC_CoreWaitForEvent
(
    void
);

static SALRetCode_t UC_CoreDiv64To32
(
    uint64 *                            pullDividend,
    uint32                              uiDivisor,
    uint32 *                            puiRem
);

static SALRetCode_t UC_OSInitFuncs
(
    void
);

static SALRetCode_t UC_OSStart
(
    void
);

static SALRetCode_t UC_MemSet
(
    void *                              pMem,
    uint8                               ucValue,
    SALSize                             uiSize
);

static SALRetCode_t UC_MemCopy
(
    void *                              pDest,
    const void *                        pSrc,
    SALSize                             uiSize
);

static SALRetCode_t UC_MemCmp
(
    const void *                        pMem1,
    const void *                        pMem2,
    SALSize                             uiSize,
    sint32 *                            piRetCmp
);

static SALRetCode_t UC_StrCopy
(
    uint8 *                             pucDestString,
    const uint8 *                       pucSrcString
);

static SALRetCode_t UC_StrCmp
(
    const uint8 *                       puc1String,
    const uint8 *                       puc2String,
    sint32 *                            piRetCmp
);

static SALRetCode_t UC_StrNCmp
(
    const uint8 *                       puc1String,
    const uint8 *                       puc2String,
    SALSize                             uiLength,
    sint32 *                            piRetCmp
);

static SALRetCode_t UC_StrLength
(
    const int8 *                       pucString,
    SALSize *                           puiLength
);

static SALRetCode_t UC_QueueCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiDepth,
    SALSize                             uiDataSize
);

static SALRetCode_t UC_QueueDelete
(
    uint32                              uiId
);

static SALRetCode_t UC_QueueGet
(
    uint32                              uiId,
    void *                              pData,
    uint32 *                            puiSizeCopied,
    uint32a                             iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t UC_QueuePut
(
    uint32                              uiId,
    void *                              pData,
    SALSize                             uiSize,
    sint32                              iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t UC_SemaphoreCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiInitialValue,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t UC_SemaphoreDelete
(
    uint32                              uiId
);

static SALRetCode_t UC_SemaphoreRelease
(
    uint32                              uiId
);

static SALRetCode_t UC_SemaphoreWait
(
    uint32                              uiId,
    sint32                              iTimeout,
    SALBlockingOption_t                 ulOptions
);

static SALRetCode_t UC_EventCreate
(
    uint32 *                            puiId,
    const uint8 *                       pucName,
    uint32                              uiInitialValue
);

static SALRetCode_t UC_EventDelete
(
    uint32                              uiId
);

static SALRetCode_t UC_EventSet
(
    uint32                              uiId,
    uint32                              uiEvent,
    SALEventOption_t                    ulOptions
);

static SALRetCode_t UC_EventGet
(
    uint32                              uiId,
    uint32                              uiEvent,
    uint32                              iTimeout,
    uint32                              uiOptions,
    uint32 *                            puiFlags
);

static SALRetCode_t UC_TaskCreate
(
    uint32 *                            puiTaskId,
    const uint8 *                       pucTaskName,
    SALTaskFunc                         fnTask,
    uint32 *                            puiStackPtr,
    uint32                              uiStackSize,
    SALTaskPriority_t                   uiPriority,
    void *                              pTaskParam
);

static SALRetCode_t UC_TaskSleep
(
    uint32                              uiMilliSec
);

static SALRetCode_t UC_ReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId, 
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
);


static SALRetCode_t UC_GetCurrentError
(
    SALErrorInfo_t *                    psInfo
);

static SALRetCode_t UC_GetTickCount
(
    uint32 *                            puiTickCount
);

static SALRetCode_t UC_GetMcuVersionInfo
(
    SALMcuVersionInfo_t *               psVersion
);

static SALRetCode_t UC_GetSystemTickCount
(
    uint64 *                            pullSysTickCount
);

static SALRetCode_t UC_IncreaseSystemTick
(
    uint64 *                            pullSysTickCount
);

/*
***************************************************************************************************
*                                         [FUNCTIONS]
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          [UC_CoreMB]
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_CoreMB (void)
{
    CPU_DSB();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [UC_CoreCriticalEnter]
*
* Save CPU status word & Disable interrupts.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_CoreCriticalEnter (void)
{
    gCPU_SR = 0;
    gCPU_SR = CPU_SR_Save();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [UC_CoreCriticalExit]
*
* Restore CPU status word & Re-enable interrupts.
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_CoreCriticalExit (void)
{
    CPU_SR_Restore((CPU_SR)gCPU_SR);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [UC_CoreWaitForEvent]
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_CoreWaitForEvent (void)
{
    CPU_WaitForEvent();

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [UC_CoreDiv64To32]
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
static SALRetCode_t UC_CoreDiv64To32
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
                retVal = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_CORE_DIV_64_32,
                                        SAL_ERR_INVALID_PARAMETER, 
                                        __FUNCTION__);
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
                        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                                SAL_API_CORE_DIV_64_32,
                                                SAL_ERR_INVALID_PARAMETER, 
                                                __FUNCTION__);
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
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_CORE_DIV_64_32,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    if (puiRem != NULL_PTR)
    {
        *puiRem = (uint32)rem;
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_OSInitFuncs]
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_OSInitFuncs (void)
{
#if (OS_CFG_STAT_TASK_EN > 0U)
    OS_ERR err;
#endif

    SALRetCode_t retval                 = SAL_RET_SUCCESS;

    OS_TmrTickInit();

    CPU_Init();                         /* Initialize the uC/CPU services               */

#if (OS_CFG_STAT_TASK_EN > 0U)
    OSStatTaskCPUUsageInit(&err);       /* Compute CPU capacity with no task running    */

    if (err != OS_ERR_NONE)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_OS_INIT_FUNCS,
                                SAL_ERR_INIT, 
                               __FUNCTION__);
    }
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    (void)CPU_IntDisMeasMaxCurReset();
#endif

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_OSStart]
*
*
* @return
*
* Notes
*s
***************************************************************************************************
*/
static SALRetCode_t UC_OSStart (void)
{
    OS_ERR err;

    SAL_D("~ Done to initialize Free uC/OS-III Operating System ~\n");

    /* Start multitasking (i.e. give control to uC/OS-III). */
    OSStart(&err);

    (void)err;  // Unused variable - Always gonna be ok

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [UC_MemSet]
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
static SALRetCode_t UC_MemSet
(
    void * pMem,
    uint8 ucValue,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if (pMem != NULL_PTR)
    {
#ifdef SAL_MEM_FUNC_IMPL
        Mem_Set(pMem, ucValue, (CPU_SIZE_T)uiSize);
#else
        (void)memset(pMem, (UCInt32)ucValue, uiSize); //QAC-Not use return value
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_SET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_MemCopy]
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
static SALRetCode_t UC_MemCopy
(
    void * pDest,
    const void * pSrc,
    SALSize uiSize
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pDest != NULL_PTR) && (pSrc != NULL_PTR))
    {
#ifdef SAL_MEM_FUNC_IMPL
        Mem_Copy(pDest, pSrc, (CPU_SIZE_T)uiSize);
#else
        (void)memcpy(pDest, pSrc, (size_t)uiSize); //QAC-Not use return value
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_COPY,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_MemCmp]
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
static SALRetCode_t UC_MemCmp
(
    const void * pMem1,
    const void * pMem2,
    SALSize uiSize,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pMem1 != NULL_PTR) && (pMem2 != NULL_PTR) && (uiSize > 0UL) && (piRetCmp != NULL_PTR))
    {
#ifdef SAL_MEM_FUNC_IMPL
        *piRetCmp = (sint32)Mem_Cmp(pMem1, pMem2, (CPU_SIZE_T)uiSize);
#else
        *piRetCmp = (sint32)memcmp(pMem1, pMem2, (size_t)uiSize);
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_MEM_CMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}


/*
***************************************************************************************************
*                                          [UC_StrCopy]
*
*
* @param    pucDestString [out]
* @param    pucSrcString [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_StrCopy
(
    uint8 * pucDestString,
    const uint8 * pucSrcString
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pucDestString != NULL_PTR) && (pucSrcString != NULL_PTR))
    {
#ifdef SAL_STRING_FUN_IMPL
        (void)Str_Copy((CPU_CHAR *)pucDestString, (const CPU_CHAR *)pucSrcString);
#else
        uint32 str_len = 0;
        str_len = strnlen((const int8 *)pucSrcString, (size_t)(SAL_MAX_INT_VAL - 1UL));

        if (str_len != 0UL)
        {
            (void)strncpy((int8 *)pucDestString, (const int8 *)pucSrcString, (size_t)str_len); //Codesonar Security Warning
        }
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_COPY,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_StrCmp]
*
*
* @param    puc1String [in]
* @param    puc2String [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_StrCmp
(
    const uint8 * puc1String,
    const uint8 * puc2String,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((puc1String != NULL_PTR) && (puc2String != NULL_PTR) && (piRetCmp != NULL_PTR))
    {
#ifdef SAL_STRING_FUN_IMPL
        *piRetCmp = Str_Cmp((const CPU_CHAR *)puc1String, (const CPU_CHAR *)puc2String);
#else
        *piRetCmp = strcmp((const int8 *)puc1String, (const int8 *)puc2String);
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_CMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_StrNCmp]
*
*
* @param    puc1String [in]
* @param    puc2String [in]
* @param    uiLength [in]
* @param    piRetCmp [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_StrNCmp
(
    const uint8 * puc1String,
    const uint8 * puc2String,
    SALSize uiLength,
    sint32 * piRetCmp
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((puc1String != NULL_PTR) && (puc2String!= NULL_PTR) && (uiLength > 0UL) && (piRetCmp != NULL_PTR))
    {
#ifdef SAL_STRING_FUN_IMPL
        *piRetCmp = Str_Cmp_N((const CPU_CHAR *)puc1String, (const CPU_CHAR *)puc2String, (CPU_SIZE_T)uiLength);
#else
        *piRetCmp = strncmp((const int8 *)puc1String, (const int8 *)puc2String, (size_t)uiLength);
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_NCMP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_StrLength]
*
*
* @param    pucString [in]
* @param    puiLength [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_StrLength
(
    const int8 * pucString,
    SALSize * puiLength
) {
    SALRetCode_t retVal = SAL_RET_SUCCESS;

    if ((pucString != NULL_PTR) && (puiLength != NULL_PTR))
    {
#ifdef SAL_STRING_FUN_IMPL
        *puiLength = Str_Len((const CPU_CHAR *)pucString);
#else
        *puiLength = strnlen(pucString, (size_t)(SAL_MAX_INT_VAL - 1UL)); //Codesonar Security Warning
#endif
    }
    else
    {
        retVal = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_STR_LENGTH,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retVal;
}

/*
***************************************************************************************************
*                                          [UC_QueueCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiDepth [in]
* @param    uiDataSize [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_QueueCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiDepth,
    SALSize uiDataSize
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    uint32 search_id                    = 999999;
    uint32 i;
    OS_ERR err;
    CPU_TS ts;

    (void)uiDataSize;       // unused parameter

    if (puiId == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if ((pucName == NULL_PTR) 
        || (uiDepth < 1UL)
        || ((SAL_MAX_INT_VAL / uiDepth) < uiDataSize))
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiId = search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        (void)OSSemPend((OS_SEM *)&gSemObj,
                        (OS_TICK)0,
                        (OS_OPT)OS_OPT_PEND_BLOCKING,
                        (CPU_TS *)&ts,
                        (OS_ERR *)&err);

        *puiId = search_id;

        if (err == OS_ERR_NONE)
        {
            /*search free table*/
            for (i = 0UL; i < (uint32)SAL_MAX_QUEUE; i++)
            {
                if (gQueueObj[i].qiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id < (uint32)SAL_MAX_QUEUE)// CS : Redundant Condition
            {
                static CPU_CHAR tempName[255] = {0};
                SALSize nameSize = 0;

                (void)UC_StrLength((const int8 *)pucName, &nameSize);
                (void)UC_MemCopy(tempName, pucName, nameSize);
                
                // Queue
                OSQCreate((OS_Q *)&gQueueObj[search_id].qiCb,
                          tempName,
                          (OS_MSG_QTY)uiDepth,
                          (OS_ERR *)&err);

                if (err == OS_ERR_NONE)
                {
                    gQueueObj[search_id].qiUsed = TRUE;
                    *puiId = search_id;
                }
                else
                {
                    retval = UC_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_DATA_QUEUE_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_DATA_QUEUE_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            (void)OSSemPost((OS_SEM *)&gSemObj,
                            (OS_OPT)OS_OPT_POST_NONE,
                            (OS_ERR *)&err);
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_CREATE,
                                    SAL_ERR_FAIL_CREATE, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_QueueDelete]
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
#if (OS_CFG_Q_DEL_EN > 0U)
static SALRetCode_t UC_QueueDelete(uint32 uiId)
{
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;

    if ((uiId < (uint32)SAL_MAX_QUEUE) && gQueueObj[uiId].qiUsed == TRUE)
    {
        (void)OSQDel((OS_Q *)&gQueueObj[uiId].qiCb, OS_OPT_DEL_ALWAYS, &err);

        if (err == OS_ERR_NONE)
        {
            gQueueObj[uiId].qiUsed = FALSE;
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_DELETE,
                                    SAL_ERR_FAIL_DELETE, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_DELETE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}
#endif      // OS_CFG_Q_DEL_EN > 0u

/*
***************************************************************************************************
*                                          [UC_QueueGet]
*
*
* @param    uiId [in]
* @param    pData [out]
* @param    puiSizeCopied [out]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_QueueGet
(
    uint32 uiId,
    void * pData,
    uint32 * puiSizeCopied,
    uint32a iTimeout,
    SALBlockingOption_t  ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;
    CPU_TS ts;
    OS_MSG_SIZE msg_copied              = 0;
    
    (void)ulOptions;        // unused paramter

    if (puiSizeCopied != NULL_PTR)
    {
        *puiSizeCopied = 0;
    }

    if ((uiId < (uint32)SAL_MAX_QUEUE) 
        && (gQueueObj[uiId].qiUsed == TRUE) 
        && (pData != NULL_PTR) 
        && (puiSizeCopied != NULL_PTR))
    {
        const void * queue_data = OSQPend((OS_Q *)&gQueueObj[uiId].qiCb,
                                          iTimeout,
                                          (OS_OPT)OS_OPT_PEND_BLOCKING,
                                          (OS_MSG_SIZE *)&msg_copied,
                                          (CPU_TS *)&ts,
                                          (OS_ERR *)&err);

        if ((err == OS_ERR_NONE) && (queue_data != NULL_PTR) && (msg_copied > 0UL))
        {
            (void)UC_MemCopy(pData, queue_data, (SALSize)msg_copied);
            *puiSizeCopied = (uint32)msg_copied;
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_GET,
                                    SAL_ERR_FAIL_GET_DATA, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_QueuePut]
*
*
* @param    uiId [in]
* @param    pData [in]
* @param    uiSize [in]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_QueuePut
(
    uint32 uiId,
    void * pData,
    SALSize uiSize,
    sint32 iTimeout,
    SALBlockingOption_t  ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;

    (void)iTimeout;
    (void)ulOptions;        // unused paramter

    if ((uiId < (uint32)SAL_MAX_QUEUE) 
        && (gQueueObj[uiId].qiUsed == TRUE) 
        && (pData != NULL_PTR) 
        && (uiSize > 0UL))
    {
        OSQPost((OS_Q *)&gQueueObj[uiId].qiCb,
                (void *)pData,
                (OS_MSG_SIZE)uiSize,
                (OS_OPT)OS_OPT_POST_FIFO,
                (OS_ERR *)&err);

        if (err != OS_ERR_NONE)
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_DATA_QUEUE_PUT,
                                    SAL_ERR_FAIL_SEND_DATA, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_DATA_QUEUE_PUT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_SemaphoreCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiInitialValue [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_SemaphoreCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiInitialValue,
    SALBlockingOption_t ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    uint32 search_id                    = 99999;
    uint32 i;
    OS_ERR err;
    CPU_TS ts;

    (void)ulOptions;

    if (puiId == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL,
                                SAL_API_SEMAPHORE_CREATE,
                                SAL_ERR_INVALID_PARAMETER,
                                __FUNCTION__);
    }
    else if ((pucName == NULL_PTR) || (uiInitialValue == 0UL))
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiId = search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        (void)OSSemPend((OS_SEM *)&gSemObj,
                        (OS_TICK)0,
                        (OS_OPT)OS_OPT_PEND_BLOCKING,
                        (CPU_TS *)&ts,
                        (OS_ERR *)&err);

        *puiId = search_id;

        if (err == OS_ERR_NONE)
        {
            /*search free table*/
            for (i = 0UL; i < (uint32)SAL_MAX_SEMA; i++)
            {
                if (gSemaObj[i].siUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if ( search_id < (uint32)SAL_MAX_SEMA)
            {
                static CPU_CHAR tempName[255] = {0};
                SALSize nameSize = 0;

                (void)UC_StrLength((const int8 *)pucName, &nameSize);
                (void)UC_MemCopy(tempName, pucName, nameSize);
                
                // Semaphore
                OSSemCreate((OS_SEM *)&gSemaObj[search_id].siCb,
                            tempName,
                            (OS_SEM_CTR)uiInitialValue,
                            (OS_ERR *)&err);

                if (err == OS_ERR_NONE)
                {
                    gSemaObj[search_id].siUsed = TRUE;
                    *puiId = search_id;
                }
                else
                {
                    retval = UC_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_SEMAPHORE_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_SEMAPHORE_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            (void)OSSemPost((OS_SEM *)&gSemObj,
                            (OS_OPT)OS_OPT_POST_NONE,
                            (OS_ERR *)&err);
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_CREATE,
                                    SAL_ERR_FAIL_GET_KEY, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_SemaphoreDelete]
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
#if (OS_CFG_SEM_DEL_EN > 0u)
static SALRetCode_t UC_SemaphoreDelete (uint32 uiId)
{
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;

    if ((uiId < (uint32)SAL_MAX_SEMA) && (gSemaObj[uiId].siUsed == TRUE))
    {
        (void)OSSemDel((OS_SEM *)&gSemaObj[uiId].siCb, OS_OPT_DEL_ALWAYS, &err);

        if (err == OS_ERR_NONE)
        {
            gSemaObj[uiId].siUsed = FALSE;
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_DELETE,
                                    SAL_ERR_FAIL_DELETE, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_DELETE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}
#endif      // OS_CFG_SEM_DEL_EN > 0u

/*
***************************************************************************************************
*                                          [UC_SemaphoreRelease]
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_SemaphoreRelease (uint32 uiId)
{
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;

    if ((uiId < (uint32)SAL_MAX_SEMA) && (gSemaObj[uiId].siUsed == TRUE))
    {
        (void)OSSemPost((OS_SEM *)&gSemaObj[uiId].siCb,
                        (OS_OPT )OS_OPT_POST_NONE,
                        (OS_ERR *)&err);

        if (err != OS_ERR_NONE)
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_RELEASE,
                                    SAL_ERR_FAIL_RELEASE, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_RELEASE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_SemaphoreWait]
*
*
* @param    uiId [in]
* @param    iTimeout [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_SemaphoreWait
(
    uint32 uiId,
    sint32 iTimeout,
    SALBlockingOption_t ulOptions
) {
    SALRetCode_t retval;
    OS_ERR err;
    CPU_TS ts;
    OS_OPT opt; // CS : Useless Assignment

    (void)iTimeout;

    if ((uiId < (uint32)SAL_MAX_SEMA) && (gSemaObj[uiId].siUsed == TRUE))
    {
        if (ulOptions == SAL_OPT_BLOCKING) //CS : Redundant Condition
        {
            opt = OS_OPT_PEND_BLOCKING;
        }
        else if (ulOptions == SAL_OPT_NON_BLOCKING)  //CS : Redundant Condition
        {
            opt = OS_OPT_PEND_NON_BLOCKING;
        }
        else
        {
            opt = OS_OPT_PEND_NON_BLOCKING;
        }

        (void)OSSemPend((OS_SEM *)&gSemaObj[uiId].siCb,
                        (OS_TICK)0,
                        (OS_OPT)opt,
                        (CPU_TS *)&ts,
                        (OS_ERR *)&err);

        if (err == OS_ERR_NONE)
        {
            retval = SAL_RET_SUCCESS;
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_SEMAPHORE_WAIT,
                                    SAL_ERR_FAIL_GET_DATA, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_SEMAPHORE_WAIT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_EventCreate]
*
*
* @param    puiId [out]
* @param    pucName [in]
* @param    uiInitialValue [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_EventCreate
(
    uint32 * puiId,
    const uint8 * pucName,
    uint32 uiInitialValue
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    uint32 search_id                    = 99999;
    uint32 i;
    OS_ERR err;
    CPU_TS ts;

    if (puiId == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if (pucName == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_CREATE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiId = search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        (void)OSSemPend((OS_SEM *)&gSemObj,
                        (OS_TICK)0,
                        (OS_OPT)OS_OPT_PEND_BLOCKING,
                        (CPU_TS *)&ts,
                        (OS_ERR *)&err);

        *puiId = search_id;
        
        if (err == OS_ERR_NONE)
        {
            /*search free table*/
            for (i = 0; i < (uint32)SAL_MAX_EVENT; i++)
            {
                if (gEventObj[i].eiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id < (uint32)SAL_MAX_EVENT)// CS : Redundant Condition
            {
                static CPU_CHAR tempName[255] = {0};
                SALSize nameSize = 0;

                (void)UC_StrLength((const int8 *)pucName, &nameSize);
                (void)UC_MemCopy(tempName, pucName, nameSize);
                
                // Event flags
                OSFlagCreate((OS_FLAG_GRP *)&gEventObj[search_id].eiCb,
                            tempName,
                            (OS_FLAGS)uiInitialValue,
                            &err);

                if (err == OS_ERR_NONE)
                {
                    gEventObj[search_id].eiUsed = TRUE;
                    *puiId = search_id;
                }
                else
                {
                    retval = UC_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_EVENT_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_EVENT_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }

            /*release sema*/
            (void)OSSemPost((OS_SEM *)&gSemObj,
                            (OS_OPT)OS_OPT_POST_NONE,
                            (OS_ERR *)&err);
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_CREATE,
                                    SAL_ERR_FAIL_GET_KEY, 
                                    __FUNCTION__);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_EventDelete]
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
#if (OS_CFG_FLAG_DEL_EN > 0u)
static SALRetCode_t UC_EventDelete(uint32 uiId)
{
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;

    if ((uiId < (uint32)SAL_MAX_EVENT) && (gEventObj[uiId].eiUsed == TRUE))
    {
        (void)OSFlagDel((OS_FLAG_GRP *)&gEventObj[uiId].eiCb, OS_OPT_DEL_ALWAYS, &err);

        if (err == OS_ERR_NONE)
        {
            gEventObj[uiId].eiUsed = FALSE;
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_DELETE,
                                    SAL_ERR_FAIL_DELETE, 
                                    __FUNCTION__);
        }
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_DELETE,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}
#endif // (OS_CFG_FLAG_DEL_EN > 0u)

/*
***************************************************************************************************
*                                          [UC_EventSet]
*
*
* @param    uiId [in]
* @param    uiEvent [in]
* @param    ulOptions [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_EventSet
(
    uint32 uiId,
    uint32 uiEvent,
    SALEventOption_t ulOptions
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    OS_ERR err;
    OS_OPT opt;

    if (uiId < (uint32)SAL_MAX_EVENT)
    {
        if (gEventObj[uiId].eiUsed == TRUE)
        {
            if (((uint32)ulOptions & 0x00000001UL) == (uint32)SAL_EVENT_OPT_FLAG_SET)
            {
                opt = OS_OPT_POST_FLAG_SET;
            }
            else if(((uint32)ulOptions & 0x00000001UL) == (uint32)SAL_EVENT_OPT_CLR_ALL)
            {
                opt = OS_OPT_POST_FLAG_CLR;
            }
            else
            {
                opt = OS_OPT_POST_FLAG_SET;
            }

            //flags = OSFlagPost((OS_SEM *)&gEventObj[event_id].eiCb, event, opt, &err);
            (void)OSFlagPost((OS_FLAG_GRP *)&gEventObj[uiId].eiCb, (OS_FLAGS)uiEvent, opt, &err);//CS:Unused Value

            if (err != OS_ERR_NONE)
            {
                retval = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_EVENT_SET,
                                        SAL_ERR_FAIL_SET_CONFIG, 
                                        __FUNCTION__);
            }
            else {;}
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_SET,
                                    SAL_ERR_NOT_USEFUL, 
                                    __FUNCTION__);
        }
    }
    else
    {
        /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_SET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_EventGet]
*
*
* @param    uiId [in]
* @param    uiEvent [in]
* @param    iTimeout [in]
* @param    options [in]
* @param    puiFlags [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_EventGet
(
    uint32 uiId,
    uint32 uiEvent,
    uint32 iTimeout,
    uint32 uiOptions,
    uint32 * puiFlags
) {
    SALRetCode_t retval = SAL_RET_SUCCESS;
    OS_ERR ucos_err;
    CPU_TS ts;
    OS_OPT opt;

    if (puiFlags == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if (uiId < (uint32)SAL_MAX_EVENT)
    {
        if (gEventObj[uiId].eiUsed == TRUE)
        {
            opt = (OS_OPT)(uiOptions & 0xFFFFUL);

            *puiFlags = OSFlagPend((OS_FLAG_GRP *)&gEventObj[uiId].eiCb,
                                   (OS_FLAGS)uiEvent,
                                   (OS_TICK)iTimeout,
                                   opt,
                                   (CPU_TS *)&ts,
                                   (OS_ERR *)&ucos_err);

            switch (ucos_err)
            {
                case OS_ERR_NONE :
                {
                    break;
                }

                case OS_ERR_TIMEOUT :
                {
                    SAL_D("EventGet : The event bits being waited for became set, or the block time expired\n");
                    
                    *puiFlags = 0;
                    break;
                }

                default :
                {
                    retval = UC_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_EVENT_GET,
                                            SAL_ERR_NOT_USEFUL, 
                                            __FUNCTION__);
                    *puiFlags = 0;
                    break;
                }
            }
        }
        else
        {
            retval = UC_ReportError(SAL_DRVID_SAL, 
                                    SAL_API_EVENT_GET,
                                    SAL_ERR_NOT_USEFUL, 
                                    __FUNCTION__);
            *puiFlags = 0;
        }
    }
    else
    {
        /* MISRA C:2012-14.3, All if... else if constructs shall be terminated with an else statement  */
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_EVENT_GET,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiFlags = 0;
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_TaskCreate]
*
*
* @param    puiTaskId [out]
* @param    pucTaskName [in]
* @param    fnTask [in]
* @param    puiStackPtr [out]
* @param    uiStackSize [in]
* @param    uiPriority [in]
* @param    pTaskParam [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
// Deviation Record - HIS metric violation (HIS_PARAM)
static SALRetCode_t UC_TaskCreate
(
    uint32 * puiTaskId,
    const uint8 * pucTaskName,
    SALTaskFunc fnTask,
    uint32 * puiStackPtr,
    uint32 uiStackSize,
    SALTaskPriority_t uiPriority,
    void * pTaskParam
) {
    SALRetCode_t retval                 = SAL_RET_SUCCESS;
    uint32 search_id                    = 99999;
    uint32 i;
    OS_ERR err;
    CPU_TS ts;

    if (puiTaskId == NULL_PTR)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_TASK_CREATE, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else if ((pucTaskName == NULL_PTR) || (fnTask == NULL_PTR) ||(puiStackPtr == NULL_PTR))
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_TASK_CREATE, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
        *puiTaskId = search_id;
    }
    else
    {
        /* obtain sema. wait forever*/
        (void)OSSemPend((OS_SEM *)&gSemObj,
                        (OS_TICK)0,
                        (OS_OPT)OS_OPT_PEND_BLOCKING,
                        (CPU_TS *)&ts,
                        (OS_ERR *)&err);

        *puiTaskId = search_id;

        if (err == OS_ERR_NONE)
        {
            /*search free table*/
            for (i = 0; i < (uint32)SAL_MAX_TASK; i++)
            {
                if (gTaskObj[i].tiUsed == FALSE)
                {
                    search_id = i;
                    break;
                }
            }

            if (search_id < (uint32)SAL_MAX_TASK)
            {
                static CPU_CHAR tempName[255] = {0};
                SALSize nameSize = 0;
                CPU_STK * puiValidStack;

                (void)UC_StrLength((const int8 *)pucTaskName, &nameSize);
                (void)UC_MemCopy(tempName, pucTaskName, nameSize);
                (void)UC_MemCopy(&puiValidStack, (const void *)&puiStackPtr, sizeof(CPU_STK *));

                OSTaskCreate((OS_TCB *)&gTaskObj[search_id].tiCb,
                             tempName,
                             (OS_TASK_PTR )fnTask,
                             (void *)pTaskParam,
                             (OS_PRIO )uiPriority,
                             puiValidStack,
                             (CPU_STK_SIZE)0,
                             (CPU_STK_SIZE)uiStackSize,
                             (OS_MSG_QTY )0, // not use task queue
                             (OS_TICK )0, // time slice default tick rate/10
                             (void *)0,
                             (OS_OPT )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                             (OS_ERR *)&err);

                if (err == OS_ERR_NONE)
                {
                    gTaskObj[search_id].tiUsed = TRUE;
                    *puiTaskId = search_id;
                }
                else
                {
                    retval = UC_ReportError(SAL_DRVID_SAL, 
                                            SAL_API_TASK_CREATE,
                                            SAL_ERR_FAIL_CREATE, 
                                            __FUNCTION__);
                }
            }
            else
            {
                retval = UC_ReportError(SAL_DRVID_SAL, 
                                        SAL_API_TASK_CREATE,
                                        SAL_ERR_NO_SPACE, 
                                        __FUNCTION__);
            }
            /*release sema*/
            (void)OSSemPost((OS_SEM *)&gSemObj,
                            (OS_OPT)OS_OPT_POST_NONE,
                            (OS_ERR *)&err);
        }
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_TaskSleep]
*
*
* @param    uiMilliSec [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_TaskSleep (uint32 uiMilliSec)
{
    SALRetCode_t            retval = SAL_RET_SUCCESS;
    uint32                  ticks = 0;
    OS_ERR                  err;

    if ((SAL_MAX_INT_VAL / OS_CFG_TICK_RATE_HZ) < uiMilliSec)
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_TASK_SLEEP,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }
    else
    {
        ticks = (uint32)((uiMilliSec * OS_CFG_TICK_RATE_HZ) / 1000UL);
        OSTimeDly((OS_TICK)ticks, (OS_OPT)OS_OPT_TIME_DLY, (OS_ERR *)&err);
    }
    
    return retval;
}

/*
***************************************************************************************************
*                                          [UC_ReportError]
*
*
* @param    uiDrvierId [in]
* @param    uiApiId [in]
* @param    uiErrorCode [in]
* @param    pucEtc [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
// Deviation Record - HIS metric violation (HIS_CALLING)
static SALRetCode_t UC_ReportError
(
    SALDriverId_t                       uiDriverId,
    uint32                              uiApiId, 
    SALErrorCode_t                      uiErrorCode,
    const int8 *                        pucEtc
) {
    SAL_E("[%d][API : %d][ERROR Code: %d][From: %s]\n", uiDriverId, uiApiId, uiErrorCode, pucEtc);

    (void)UC_CoreCriticalEnter();
    gErrorInfo.eiDrvId = uiDriverId;
    gErrorInfo.eiApiId = uiApiId;
    gErrorInfo.eiErrorId = uiErrorCode;
    (void)UC_CoreCriticalExit();

    return SAL_RET_FAILED;
}

/*
***************************************************************************************************
*                                          [UC_GetCurrentError]
*
*
* @param    psInfo [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_GetCurrentError(SALErrorInfo_t * psInfo) 
{
    SALRetCode_t retval = SAL_RET_SUCCESS;
    
    if (psInfo != NULL_PTR)
    {
        (void)UC_CoreCriticalEnter();
        (void)UC_MemCopy(psInfo, &gErrorInfo, sizeof(SALErrorInfo_t));

        // Right after returing the information of error, info table is initialized.
        (void)UC_MemSet(&gErrorInfo, 0, sizeof(SALErrorInfo_t));
        (void)UC_CoreCriticalExit();
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_CURRENT_ERR, 
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}


/*
***************************************************************************************************
*                                          [UC_GetTickCount]
*
*
* @param    puiTickCount [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_GetTickCount (uint32 * puiTickCount)
{
    SALRetCode_t retval         = SAL_RET_SUCCESS;
    OS_ERR err;

    if (puiTickCount != NULL_PTR)
    {
        *puiTickCount = OSTimeGet(&err);
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_TICKCOUNT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_GetMcuVersionInfo]
*
* @param    psVersion [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_GetMcuVersionInfo (SALMcuVersionInfo_t * psVersion)
{
    SALRetCode_t retval = SAL_RET_SUCCESS;

    static const SALMcuVersionInfo_t MCU_Version =
    {
        SDK_MAJOR_VERSION,
        SDK_MINOR_VERSION,
        SDK_PATCH_VERSION,
        0U
    };

    if (psVersion != NULL_PTR)
    {
        (void)UC_MemCopy(psVersion, &MCU_Version, sizeof(SALMcuVersionInfo_t));
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_VERSION,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_GetSystemTickCount]
*
*
* @param    pullSysTickCount [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_GetSystemTickCount (uint64 * pullSysTickCount)
{
    SALRetCode_t retval = SAL_RET_SUCCESS;

    if (pullSysTickCount != NULL_PTR)
    {
        *pullSysTickCount = gSystemMainTickCnt;
    }
    else
    {
        retval = UC_ReportError(SAL_DRVID_SAL, 
                                SAL_API_GET_SYSTEM_TICK_CNT,
                                SAL_ERR_INVALID_PARAMETER, 
                                __FUNCTION__);
    }

    return retval;
}

/*
***************************************************************************************************
*                                          [UC_IncreaseSystemTick]
*
*
* @param    pullSysTickCount [out]
* @return
*
* Notes
*
***************************************************************************************************
*/
static SALRetCode_t UC_IncreaseSystemTick (uint64 * pullSysTickCount)
{
    if ((0xFFFFFFFFFFFFFFFFULL - 1ULL) < gSystemMainTickCnt)
    {
        gSystemMainTickCnt = 0;
    }
    else
    {
        gSystemMainTickCnt++;
    }

    if (pullSysTickCount != NULL_PTR)
    {
        *pullSysTickCount = gSystemMainTickCnt;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          [SAL_Init]
*
* Initializes every elements and functions
*
* @return
*
* Notes
*
***************************************************************************************************
*/
// Deviation Record - HIS metric violation (HIS_CALLS)
SALRetCode_t SAL_Init (void)
{
    uint32 initIndex;
    OS_ERR err;
    SALRetCode_t ret                    = SAL_RET_SUCCESS;

    (void)SAL_RegisterFunction(SAL_API_CORE_MB, &UC_CoreMB, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_CRITICAL_ENTER, &UC_CoreCriticalEnter, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_CRITICAL_EXIT, &UC_CoreCriticalExit, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_WAITFOREVENT, &UC_CoreWaitForEvent, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_CORE_DIV_64_32, &UC_CoreDiv64To32, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_OS_INIT_FUNCS, &UC_OSInitFuncs, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_OS_START, &UC_OSStart, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_MEM_SET, &UC_MemSet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_MEM_COPY, &UC_MemCopy, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_MEM_CMP, &UC_MemCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_COPY, &UC_StrCopy, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_CMP, &UC_StrCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_NCMP, &UC_StrNCmp, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_STR_LENGTH, &UC_StrLength, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_CREATE, &UC_QueueCreate, NULL_PTR);
#if (OS_CFG_Q_DEL_EN > 0u)
    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_DELETE, &UC_QueueDelete, NULL_PTR);
#endif
    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_GET, &UC_QueueGet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_DATA_QUEUE_PUT, &UC_QueuePut, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_EVENT_CREATE, &UC_EventCreate, NULL_PTR);
#if (OS_CFG_FLAG_DEL_EN > 0u)
    (void)SAL_RegisterFunction(SAL_API_EVENT_DELETE, &UC_EventDelete, NULL_PTR);
#endif
    (void)SAL_RegisterFunction(SAL_API_EVENT_SET, &UC_EventSet, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_EVENT_GET, &UC_EventGet, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_CREATE, &UC_SemaphoreCreate, NULL_PTR);
#if (OS_CFG_SEM_DEL_EN > 0u)
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_DELETE, &UC_SemaphoreDelete, NULL_PTR);
#endif
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_RELEASE, &UC_SemaphoreRelease, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_SEMAPHORE_WAIT, &UC_SemaphoreWait, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_TASK_CREATE, &UC_TaskCreate, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_TASK_SLEEP, &UC_TaskSleep, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_GET_TICKCOUNT, &UC_GetTickCount, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_GET_VERSION, &UC_GetMcuVersionInfo, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_GET_SYSTEM_TICK_CNT, &UC_GetSystemTickCount, NULL_PTR);
    (void)SAL_RegisterFunction(SAL_API_INCREASE_SYSTEM_TICK, &UC_IncreaseSystemTick, NULL_PTR);

    (void)SAL_RegisterFunction(SAL_API_DBG_REPORT_ERR, &UC_ReportError, NULL_PTR);

    /* Init uC/OS-III */
    OSInit(&err);

    /*OASL default manage object*/
    OSSemCreate((OS_SEM *)&gSemObj,
                NULL_PTR,
                (OS_SEM_CTR)1U,
                (OS_ERR *)&err);

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0U) || (OS_CFG_ARG_CHK_EN > 0U)
    if (err == OS_ERR_NONE) // CS : Redundant Condition
    {
#endif
        /* Init Object */
        for (initIndex = 0; initIndex < (uint32)SAL_MAX_TASK; initIndex++)
        {
            //SAL_MemSet(&gTaskObj[index], 0x00, sizeof(SALTask_t));
            //modify for time save
            gTaskObj[initIndex].tiUsed = FALSE;
        }

        for (initIndex = 0; initIndex < (uint32)SAL_MAX_QUEUE; initIndex++)
        {
            //SAL_MemSet(&gQueueObj[index], 0x00, sizeof(SALQueue_t));
            //modify for time save
            gQueueObj[initIndex].qiUsed = FALSE;
        }

        for (initIndex = 0; initIndex < (uint32)SAL_MAX_SEMA; initIndex++)
        {
            //SAL_MemSet(&gSemaObj[index], 0x00, sizeof(SALSemaphore_t));
            //modify for time save
            gSemaObj[initIndex].siUsed = FALSE;
        }

        for (initIndex = 0; initIndex < (uint32)SAL_MAX_EVENT; initIndex++)
        {
            //SAL_MemSet(&gEventObj[index], 0x00, sizeof(SALEvent_t));
            //modify for time save
            gEventObj[initIndex].eiUsed = FALSE;
        }
#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0U) || (OS_CFG_ARG_CHK_EN > 0U)
    }
    else
    {
        ret = UC_ReportError(SAL_DRVID_SAL, 
                             SAL_API_COMMON_INIT,
                             SAL_ERR_INIT, 
                             __FUNCTION__);
    }
#endif

    return ret;
}

