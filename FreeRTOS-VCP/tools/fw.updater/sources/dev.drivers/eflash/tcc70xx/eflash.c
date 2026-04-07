// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eflash.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

#include <stdlib.h>
#include <sal_impl.h>

#include <bsp.h>
#include <mpu.h>
#include "eflash.h"
#include <debug.h>


#if 0//(DEBUG_ENABLE)
    #include "debug.h"
    #define EFLASH_D(fmt,args...)       {LOGD(DBG_TAG_EFLASH, fmt, ## args)}
    #define EFLASH_E(fmt,args...)       {LOGE(DBG_TAG_EFLASH, fmt, ## args)}
#else
    #define EFLASH_D(fmt,args...)
    #define EFLASH_E(fmt,args...)
#endif

#define EF_SIZE_16B                     (0x10u)

#define EF_SIZE_256B                    (0x0100u)

#define EF_SIZE_1K                      (0x0400u)
#define EF_SIZE_2K                      (0x0800u)
#define EF_SIZE_8K                      (0x2000u)


#define EF_SIZE_32K                     (0x8000u)

#define EF_SIZE_128K                    (0x20000u)

#define EF_SIZE_512K                    (0x80000u)

#define EF_SIZE_2M                      (0x200000u)


#define PFLASH_ADDRESS                  (0xA1000000u)
#define DFLASH_ADDRESS                  (0xA1080000u)

#define EF_TIMEOUT_CNT                  (0x30000u)


#define PFLASH_BASE_ADDR                (0x20000000u)
#define DFLASH_BASE_ADDR                (0x30000000u)

#define EF_FRWCON_PROG                  (0x00000002U)
#define EF_FRWCON_ERASE                 (0x00000001U)




#define EFLASH_CACHE_FLUSH()            \
    asm("DSB");                         \
    asm("MCR p15, 0, r0, c7, c5, 0");   \
    asm("MCR p15, 0, r0, c15, c5, 0");  \
    asm("ISB")


enum {
    MAT0                                = 0x1u,
    MAT1                                = 0x2u,
    MAT2                                = 0x4u,
    MAT3                                = 0x8u,
    MATx                                = 0xFu,
};

/*************************************************************************/
/*    CHECK FOR ACCESS PERMISSION                                        */
/*************************************************************************/
#define FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS                  (0xA10A001CU)
#define FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS              (0xA10A0014U)

#define FLS_HARDWARE_E_DFLASH_PKG_UNIT                      (16384u)
#define FLS_HARDWARE_E_DFLASH_MAT_UNIT                      (65536u)

/*************************************************************************/
/*    TYPE DEFINITIONS FOR STATUS FOR PFLASH STATUS REGISTER             */
/*************************************************************************/
#define EF_STS_ERASE_FAIL               (0x00000001U)
#define EF_STS_READ_FAIL                (0x00000002U)
#define EF_STS_PROG_ADDR_FAIL           (0x00000004U)
#define EF_STS_PROG_DATA_FAIL           (0x00000008U)

#define EF_STS_CMD_READY                (0x00000010U)
#define EF_STS_AND_READY                (0x00000020U)
#define EF_STS_ADDR_LATCHED             (0x00000040U)
#define EF_STS_DATA_LATCHED             (0x00000080U)
#define EF_STS_MAT0_BUSY                (0x00000100U)
#define EF_STS_MAT1_BUSY                (0x00000200U)
#define EF_STS_MAT2_BUSY                (0x00000400U)
#define EF_STS_MAT3_BUSY                (0x00000800U)
#define EF_STS_MATx_BUSY                (0x00000F00U)

#define STS_MAT_SHIFT                   (8u)

#define EF_SET_ERASEMODE(x)             ((((uint32)x & 0x3u) << 3U) | EF_FRWCON_ERASE)

typedef enum {
    FLS_FRWCON_PAGE_ERASE               = 0u,
    FLS_FRWCON_SECTOR_ERASE             = 1u,
    FLS_FRWCON_MAT_ERASE                = 2u,
    FLS_FRWCON_SAMEAS_SECTOR_ERASE      = 3u,
} EF_Erase_Mode;


/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/**************************************************************************************************/

uint8 * uiDataBuffer = NULL;
volatile EflashRegisterType * HwPflash_RegisterMap = (volatile EflashRegisterType *)PFLASH_ADDRESS;
volatile EflashRegisterType * HwDflash_RegisterMap = (volatile EflashRegisterType *)DFLASH_ADDRESS;

/*
***************************************************************************************************
*                                              LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                       EFLASH_BusyCheck
*
* Use this function to find MATs related to an erase or write operation and check it is busy or not
*
* @param    uiAddress : erase or write start address
*           uiSize    : erase  Size
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_BusyCheck
(
    uint32  uiAddress,
    uint32  uiSize,
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_getMATNum
*
*  find MATs related to an erase or write operation
*
* @param    uiAddress : erase or write start address
*           uiSize    : erase  Size
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_getMATNum
(
    uint32  uiAddress,
    uint32  uiSize,
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_CheckFSHStatus
*
*  This function reads FSHStatus and waits until it equals the value of uiExpectStatus.
*  (EFLASH_RET_TIMEOUT is returned if FSHStatus is not equal to the uiExpectStatus value
*   until passing EF_TIMEOUT_CNT.)
*
* @param    uiStatus       : FSHStatus bitMask
*           uiExpectStatus : Expected status
*           uiEfType       : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_CheckFSHStatus
(
    uint32  uiStatus,
    uint32  uiExpectStatus,
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_WaitMatBusy
*
*  Wait until MATs in uiMatNumFlag are not busy.
*  (EFLASH_RET_TIMEOUT is returned if Mats are still busy until passing EF_TIMEOUT_CNT.)
*
* @param    uiMatNumFlag   : Mat Flag, each bit field means mat number, refer to eflash.h
*           uiEfType       : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_WaitMatBusy
(
    uint32  uiMatNumFlag,
    EF_type uiEfType
);


/*
***************************************************************************************************
*                                       EFLASH_EraseMode
*
*  Erase eflash data
*
* @param    uiAddress : erase start address
*           uiMode    : erase mode, refer to EF_Erase_Mode at the eflash.h
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_EraseMode
(
    uint32 uiAddress,
    EF_Erase_Mode uiMode,
    EF_type uiEfType
);


/*
***************************************************************************************************
*                                       EFLASH_ReadByte
*
*  Copy data from eflash
*
* @param    uiAddress : start address to read
*           pBuffer   : Dest buffer
*           uiLength  : Dest buffer length
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/
static void EFLASH_ReadByte
(
    uint32 address,
    void *pBuffer,
    uint32 length,
    EF_type uiEfType
);

/*
***************************************************************************************************
*                                       EFLASH_PFLASH_WriteByte
*
*  Write data to Pflash
*
* @param    uiAddress : start address to read
*           pBuffer   : Dest buffer
*           uiLength  : Dest buffer length
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_PFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
);

#if (SWL_TOTALSIZE == 0u)

/*
***************************************************************************************************
*                                       EFLASH_DFLASH_WriteByte
*
*  Write data to Dflash
*
* @param    uiAddress : start address to read
*           pBuffer   : Dest buffer
*           uiLength  : Dest buffer length
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_DFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
);

#endif

/*
***************************************************************************************************
*                                       EFLASH_CheckDFLASHAreaAllow
*
*  check the area is belong to R5
*
* @param    StartAddress : address
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_NOT_ALLOWD_AREA
*
* Notes
*
***************************************************************************************************
*/

static uint32 EFLASH_CheckDFLASHAreaAllow
(
    const uint32 StartAddress
);

/*
***************************************************************************************************
*                                       Fls_CheckNSetHWSema
*
*  set semaphore
*
* @param    Set : semaphore
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_D_SEMA_FAIL
*
* Notes
*
***************************************************************************************************
*/

static uint32 Fls_CheckNSetHWSema
(
    const uint32 Set
);


/*
***************************************************************************************************
*                                              LOCAL FUNCTION
***************************************************************************************************
*/

static uint32 EFLASH_CheckDFLASHAreaAllow
(
    const uint32 StartAddress
)
{
    uint32 ret;
    uint32 LockNSel = *(volatile uint32 *)(FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS);

    uint8 Lock = (LockNSel & 0x40u) >> 6u;
    uint8 Sel  = (LockNSel & 0x30u) >> 4u;
    uint8 Pkg  = (LockNSel & 0x03u);
    uint8 MatNum  = (StartAddress / FLS_HARDWARE_E_DFLASH_MAT_UNIT);
    uint8 Offset  = ((StartAddress % FLS_HARDWARE_E_DFLASH_MAT_UNIT) /FLS_HARDWARE_E_DFLASH_PKG_UNIT);

    if(Lock == 0u)
    {
        ret = EFLASH_RET_NOT_ALLOWD_AREA;
    }
    else if(Sel > MatNum)
    {
        ret = EFLASH_RET_NOT_ALLOWD_AREA;
    }
    else if(Offset > Pkg)
    {
        ret = EFLASH_RET_NOT_ALLOWD_AREA;
    }
    else
    {
        ret = EFLASH_RET_OK;
    }

    return ret;
}

static uint32 Fls_CheckNSetHWSema
(
    const uint32 Set
)
{
    uint32 ret = EFLASH_RET_OK;
    volatile uint32 *HwSema = (volatile uint32 *)(FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);

    if(Set == 1u)
    {
        *HwSema = 1u;
        if(*HwSema != 1u)
        {
            ret = EFLASH_RET_D_SEMA_FAIL;
        }
    }
    else
    {
        *HwSema = 0u;
        if(*HwSema != 0u)
        {
            ret = EFLASH_RET_D_SEMA_FAIL;
        }
    }

    return ret;
}







static uint32 EFLASH_BusyCheck
(
    uint32  uiAddress,
    uint32  uiSize,
    EF_type uiEfType
)
{
    uint32 uiRet = EFLASH_RET_OK;
    uint32 uiMatNumFlag = 0;

    EFLASH_D("%s\n", __func__);

    uiMatNumFlag = EFLASH_getMATNum(uiAddress, uiSize, uiEfType);

    if( uiMatNumFlag == 0 )
    {
        EFLASH_D("[%s] get mat num error\n", __func__);
    }
    else
    {
        uiRet = EFLASH_WaitMatBusy(uiMatNumFlag, uiEfType);
    }

    return uiRet;
}

static uint32 EFLASH_getMATNum
(
    uint32  uiAddress,
    uint32  uiSize,
    EF_type uiEfType
)
{
    uint32 uiRet = 0;
    uint32 uiAddrMaxRange;
    uint32 uiSizeMax;

    EFLASH_D("%s\n", __func__);

    if(uiEfType == EF_PFLASH)
    {
        uiAddrMaxRange = EF_SIZE_2M;
        uiSizeMax      = EF_SIZE_512K;
    }
    else
    {
        uiAddrMaxRange = EF_SIZE_128K;
        uiSizeMax      = EF_SIZE_32K;
    }

    if ((uiAddress + uiSize) > uiAddrMaxRange)
    {
        EFLASH_D("[%s]Exceed Size\n", __func__);
        uiRet = 0;
    }
    else if (uiSize > uiSizeMax)
    {
        EFLASH_D("[%s] Size is over than Mat size\n", __func__);
        uiRet = 0;
    }
    else
    {
        if(uiAddress < uiSizeMax)
        {
            uiRet = MAT0;
        }
        else if (uiAddress < (uiSizeMax*2))
        {
            uiRet = MAT1;
        }
        else if (uiAddress < (uiSizeMax*2))
        {
            uiRet = MAT2;
        }
        else
        {
            uiRet = MAT3;
        }

        if ((uiAddress + uiSize) < (uiSizeMax*2))
        {
            uiRet |= MAT1;
        }
        else if ((uiAddress + uiSize) < (uiSizeMax*3))
        {
            uiRet |= MAT2;
        }
        else //  ((address+size) < (EF_SIZE_512K*4))
        {
            uiRet |= MAT3;
        }

    }
    return uiRet;

}

static uint32 EFLASH_CheckFSHStatus
(
    uint32  uiStatus,
    uint32  uiExpectStatus,
    EF_type uiEfType
)
{
    uint32 uiRet      = EFLASH_RET_OK;
    uint32 uiRetryCnt = EF_TIMEOUT_CNT;
    uint32 uiFshStat;

    EFLASH_D("%s\n", __func__);

    if(uiEfType == EF_PFLASH)
    {
        uiFshStat = HwPflash_RegisterMap->FSHSTAT;
    }
    else
    {
        uiFshStat = HwDflash_RegisterMap->FSHSTAT;
    }

    while(((uiFshStat & uiStatus) != uiExpectStatus) && (uiRetryCnt > 0))
    {
        if(uiEfType == EF_PFLASH)
        {
            uiFshStat = HwPflash_RegisterMap->FSHSTAT;
        }
        else
        {
            uiFshStat = HwDflash_RegisterMap->FSHSTAT;
        }

        if(uiRetryCnt > 0)
        {
            uiRetryCnt--;
        }

    }

    if(uiRetryCnt == 0)
    {
        EFLASH_D("flash x%x] status x%x expect_status x%x timeout \n",uiEfType, uiStatus, uiExpectStatus);
        uiRet = EFLASH_RET_TIMEOUT;
    }

    return uiRet;
}


static uint32 EFLASH_WaitMatBusy
(
    uint32  uiMatNumFlag,
    EF_type uiEfType
)
{
    uint32 uiRet = EFLASH_RET_OK;

    uint32 uiMatnumStatus = ((uiMatNumFlag & MATx) << STS_MAT_SHIFT);

    EFLASH_D("%s\n", __func__);

    uiRet = EFLASH_CheckFSHStatus(uiMatnumStatus, 0, uiEfType);


    return uiRet;
}

static uint32 EFLASH_EraseMode
(
    uint32 uiAddress,
    EF_Erase_Mode uiMode,
    EF_type uiEfType
)
{
    uint32 uiRet = EFLASH_RET_OK;

    uint32 uiSize = 0;

    EFLASH_D("%s\n", __func__);
    if(uiEfType == EF_PFLASH)
    {
        if(uiMode == FLS_FRWCON_PAGE_ERASE)
        {
            uiSize = EF_SIZE_2K;
        }
        else if(uiMode == FLS_FRWCON_SECTOR_ERASE)
        {
            uiSize = EF_SIZE_8K;
        }
        else
        {
            uiSize = EF_SIZE_512K;
        }
    }
    else
    {
        if(uiMode == FLS_FRWCON_PAGE_ERASE)
        {
            uiSize = EF_SIZE_256B;
        }
        else if(uiMode == FLS_FRWCON_SECTOR_ERASE)
        {
            uiSize = EF_SIZE_1K;
        }
        else
        {
            uiSize = EF_SIZE_32K;
        }
    }

    if(uiEfType == EF_DFLASH)
    {
        if((EFLASH_CheckDFLASHAreaAllow(uiAddress) == EFLASH_RET_OK) && \
           (EFLASH_CheckDFLASHAreaAllow(uiAddress+uiSize) == EFLASH_RET_OK))
        {
            uiRet = Fls_CheckNSetHWSema(1u);
        }
        else
        {
            uiRet = EFLASH_RET_NOT_ALLOWD_AREA;
        }
    }

    if(uiRet == EFLASH_RET_OK)
    {
        uiRet = EFLASH_BusyCheck(uiAddress, uiSize, uiEfType);

        if(uiRet == EFLASH_RET_OK)
        {
            uiRet = EFLASH_CheckFSHStatus( EF_STS_ADDR_LATCHED, 0, uiEfType);

            if(uiRet == EFLASH_RET_OK)
            {
                if(uiEfType == EF_PFLASH)
                {
                    HwPflash_RegisterMap->FSHADDR = uiAddress;
                    HwPflash_RegisterMap->FRWCON =  EF_SET_ERASEMODE(uiMode);
                }
                else
                {
                    HwDflash_RegisterMap->FSHADDR = uiAddress;
                    HwDflash_RegisterMap->FRWCON =  EF_SET_ERASEMODE(uiMode);
                }

                uiRet = EFLASH_BusyCheck(uiAddress, uiSize, uiEfType);
                if(uiRet == EFLASH_RET_OK)
                {
                    uiRet = EFLASH_CheckFSHStatus( (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                                 (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                                  uiEfType);
                }
                EFLASH_CACHE_FLUSH();
            }
        }

        if(uiEfType == EF_DFLASH)
        {
            (void)Fls_CheckNSetHWSema(0u);
        }

    }

    return uiRet;
}

//static -> api .. for eflashTest .. max clk test
uint32 EFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength,
    EF_type uiEfType
)
{
    uint32 uiRet               = EFLASH_RET_OK;

    if(uiEfType == EF_PFLASH)
    {
        uiRet = EFLASH_PFLASH_WriteByte(uiAddress, pBuffer, uiLength);
    }
    else
    {
        uiRet = EFLASH_DFLASH_WriteByte(uiAddress, pBuffer, uiLength);
    }

    return uiRet;

}


static void EFLASH_ReadByte
(
    uint32 address,
    void *pBuffer,
    uint32 length,
    EF_type uiEfType
)
{
    if(uiEfType == EF_PFLASH)
    {
        SAL_MemCopy(pBuffer, (void *)(PFLASH_BASE_ADDR + address), length);
    }
    else
    {
        SAL_MemCopy(pBuffer, (void *)(DFLASH_BASE_ADDR + address), length);
    }

    return;
}

static uint32 EFLASH_PFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
)
{
    uint32 uiRet               = EFLASH_RET_OK;
    const unsigned char * pSrc = (const uint8 *)pBuffer;

    unsigned char pBuf[4]      = {0xff, 0xff, 0xff, 0xff};
    unsigned char pBuf_ff[4]   = {0xff, 0xff, 0xff, 0xff};
    uint32 uiTargetAddr        = uiAddress;

    EFLASH_D("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", uiAddress, (unsigned int)pBuffer, uiLength);

    uiRet = EFLASH_BusyCheck(uiTargetAddr, uiLength, EF_PFLASH);

    while((uiLength > 0) && (uiRet == EFLASH_RET_OK))
    {
        uiRet = EFLASH_CheckFSHStatus(EF_STS_AND_READY, EF_STS_AND_READY, EF_PFLASH);
        if(uiRet == 0)
        {
            HwPflash_RegisterMap->FSHADDR = uiTargetAddr;
            if(uiLength >= 4u)
            {
                HwPflash_RegisterMap->FSHWDATA_1 = *((const uint32 *)pSrc);
                pSrc += 4u;
                uiLength = uiLength - 4u;

                if(uiLength >= 4u)
                {
                    HwPflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pSrc);
                    pSrc += 4u;
                    uiLength = uiLength - 4u;

                    if(uiLength >= 4u)
                    {
                        HwPflash_RegisterMap->FSHWDATA_3 = *((const uint32 *)pSrc);
                        pSrc += 4u;
                        uiLength = uiLength - 4u;

                        if(uiLength >= 4u)
                        {
                            HwPflash_RegisterMap->FSHWDATA_4 = *((const uint32 *)pSrc);
                            pSrc += 4u;
                            uiLength = uiLength - 4u;
                        }
                        else
                        {
                            SAL_MemCopy(pBuf, pSrc, uiLength);
                            HwPflash_RegisterMap->FSHWDATA_4 = *((const uint32 *)pBuf);
                            uiLength = 0;
                        }
                    }
                    else
                    {
                        SAL_MemCopy(pBuf, pSrc, uiLength);
                        HwPflash_RegisterMap->FSHWDATA_3 = *((const uint32 *)pBuf);
                        HwPflash_RegisterMap->FSHWDATA_4 = *((const uint32 *)pBuf_ff);
                        uiLength = 0;
                    }
                }
                else
                {
                    SAL_MemCopy(pBuf, pSrc, uiLength);
                    HwPflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pBuf);

                    HwPflash_RegisterMap->FSHWDATA_3 = *((const uint32 *)pBuf_ff);
                    HwPflash_RegisterMap->FSHWDATA_4 = *((const uint32 *)pBuf_ff);
                    uiLength = 0;
                }

            }
            else
            {
                SAL_MemCopy(pBuf, pSrc, uiLength);
                HwPflash_RegisterMap->FSHWDATA_1 = *((const uint32 *)pBuf);

                HwPflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pBuf_ff);
                HwPflash_RegisterMap->FSHWDATA_3 = *((const uint32 *)pBuf_ff);
                HwPflash_RegisterMap->FSHWDATA_4 = *((const uint32 *)pBuf_ff);
                uiLength = 0;
            }

            HwPflash_RegisterMap->FRWCON =  EF_FRWCON_PROG;
            uiRet = EFLASH_BusyCheck(uiTargetAddr, 16u, EF_PFLASH);
            if(uiRet == EFLASH_RET_OK)
            {
                uiRet = EFLASH_CheckFSHStatus( (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                               (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                               EF_PFLASH);
            }
        }
        uiTargetAddr += 16u;

    }


    EFLASH_CACHE_FLUSH();

    return uiRet;
}

#if (SWL_TOTALSIZE == 0u)
static uint32 EFLASH_DFLASH_WriteByte
#else
uint32 EFLASH_DFLASH_WriteByte
#endif
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
)
{
    uint32 uiRet               = EFLASH_RET_OK;
    const unsigned char * pSrc = (const uint8 *)pBuffer;

    unsigned char pBuf[4]      = {0xff, 0xff, 0xff, 0xff};
    unsigned char pBuf_ff[4]   = {0xff, 0xff, 0xff, 0xff};
    uint32 uiTargetAddr        = uiAddress;

    EFLASH_D("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", uiAddress, (unsigned int)pBuffer, uiLength);

    if((EFLASH_CheckDFLASHAreaAllow(uiTargetAddr) == EFLASH_RET_OK) && \
       (EFLASH_CheckDFLASHAreaAllow(uiTargetAddr+uiLength) == EFLASH_RET_OK))
    {

        if(Fls_CheckNSetHWSema(1u) == EFLASH_RET_OK)
        {
            uiRet = EFLASH_BusyCheck(uiTargetAddr, uiLength, EF_DFLASH);

            while((uiLength > 0) && (uiRet == EFLASH_RET_OK))
            {
                uiRet = EFLASH_CheckFSHStatus(EF_STS_AND_READY, EF_STS_AND_READY, EF_DFLASH);
                if(uiRet == 0)
                {
                    HwDflash_RegisterMap->FSHADDR = uiTargetAddr;
                    if(uiLength >= 4u)
                    {
                        HwDflash_RegisterMap->FSHWDATA_1 = *((const uint32 *)pSrc);
                        pSrc += 4u;
                        uiLength = uiLength - 4u;

                        if(uiLength >= 4u)
                        {
                            HwDflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pSrc);
                            pSrc += 4u;
                            uiLength = uiLength - 4u;

                        }
                        else
                        {
                            SAL_MemCopy(pBuf, pSrc, uiLength);
                            HwDflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pBuf);

                            uiLength = 0;
                        }

                    }
                    else
                    {
                        SAL_MemCopy(pBuf, pSrc, uiLength);
                        HwDflash_RegisterMap->FSHWDATA_1 = *((const uint32 *)pBuf);

                        HwDflash_RegisterMap->FSHWDATA_2 = *((const uint32 *)pBuf_ff);
                        uiLength = 0;
                    }

                    HwDflash_RegisterMap->FRWCON =  EF_FRWCON_PROG;
                    uiRet = EFLASH_BusyCheck(uiTargetAddr, 8u, EF_DFLASH);
                    if(uiRet == EFLASH_RET_OK)
                    {
                        uiRet = EFLASH_CheckFSHStatus( (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                                       (EF_STS_AND_READY | EF_STS_CMD_READY), \
                                                       EF_DFLASH);
                    }
                }
                uiTargetAddr += 8u;

            }
            (void)Fls_CheckNSetHWSema(0u);
        }

    }

    EFLASH_CACHE_FLUSH();

    return uiRet;
}

uint32 EFLASH_Erase
(
    uint32 uiAddress,
    uint32 uiSize,
    EF_type uiEfType
)
{
    uint32 uiRet                = EFLASH_RET_OK;
    uint32 uiI                  = 0;
    uint32 uiCntMat             = 0;
    uint32 uiMatSize            = 0;
    uint32 uiCntSector          = 0;
    uint32 uiSectorSize         = 0;
    uint32 uiCntPage            = 0;
    uint32 uiPageSize           = 0;
    uint32 uiCntRemainder       = 0;
    uint32 uiAddrMaxRange       = 0;
    uint32 uiAddrStartAlignMask = 0;
    uint32 uiAddress_MATalign   = 0;
    uint32 uiAddress_Sectoralign= 0;

    EFLASH_D("%s\n", __func__);

    if(uiEfType == EF_PFLASH)
    {
        /*??Erase Unit: 2 KB (Page), 8 KB (Sector), 512 KB (Bank) */
        uiAddrMaxRange          = EF_SIZE_2M;
        uiMatSize               = EF_SIZE_512K;
        uiSectorSize            = EF_SIZE_8K;
        uiPageSize              = EF_SIZE_2K;
    }
    else
    {
        /*??Erase Unit: 256 B (Page), 1 KB (Sector), 32 KB (Bank) */
        uiAddrMaxRange          = EF_SIZE_128K;
        uiMatSize               = EF_SIZE_32K;
        uiSectorSize            = EF_SIZE_1K;
        uiPageSize              = EF_SIZE_256B;
    }

    uiAddrStartAlignMask = (uiPageSize - 1u);

    if ((uiAddress & uiAddrStartAlignMask) || (uiSize == 0) || ((uiAddress + uiSize) > uiAddrMaxRange) )
    {
        //Address Error
        uiRet = EFLASH_RET_PARAM_OUTOFRANGE;
    }
    else
    {
        //check uiSize is more than 2 Mat or 1Mat at 0 position

        uiCntMat = uiSize / uiMatSize;

        if((uiCntMat >= 2u)||((uiCntMat == 1u) && (uiAddress % uiMatSize != 0u)))
        {
            //try to align mat align
            uiAddress_MATalign = (uiAddress + (uiMatSize - 1u))/uiMatSize*uiMatSize;
            uiAddress_Sectoralign = (uiAddress + (uiSectorSize - 1u))/uiSectorSize*uiSectorSize;
        }
        else
        {//need not mat erase
            uiCntMat = 0u;
            uiAddress_MATalign = 0u;
            uiAddress_Sectoralign = (uiAddress + (uiSectorSize - 1u))/uiSectorSize*uiSectorSize;
        }

        while((((uiAddress < uiAddress_Sectoralign)&&(uiCntMat == 0u))\
            ||(uiAddress < uiAddress_MATalign))\
            &&(uiSize > 0u))
        {
            if(uiAddress % uiSectorSize == 0u)
            {
                uiRet = EFLASH_EraseMode(uiAddress, FLS_FRWCON_SECTOR_ERASE, uiEfType);

                uiAddress += uiSectorSize;

                if(uiRet != EFLASH_RET_OK)
                {
                    uiSize = 0u;
                    break;
                }
                else if(uiSize >= uiSectorSize)
                {
                    uiSize -= uiSectorSize;
                }
                else
                {
                    uiSize = 0u;
                    break;
                }
            }
            else
            {
                uiRet = EFLASH_EraseMode(uiAddress, FLS_FRWCON_PAGE_ERASE, uiEfType);
                uiAddress += uiPageSize;

                if(uiRet != EFLASH_RET_OK)
                {
                    break;
                }
                else if(uiSize >= uiPageSize)
                {
                    uiSize -= uiPageSize;
                }
                else
                {
                    uiSize = 0u;
                    break;
                }
            }

        }


        /////////////////////////////////
        //middle erase
        //recalculate uiCntMat
        uiCntMat = uiSize / uiMatSize;

        for (uiI = 0; uiI < uiCntMat; uiI++)
        {
            uiRet = EFLASH_EraseMode(uiAddress, FLS_FRWCON_MAT_ERASE, uiEfType);
            uiAddress += uiMatSize;

            if(uiRet != EFLASH_RET_OK)
            {
                uiSize = 0;
                break;
            }
            else if(uiSize >= uiMatSize)
            {
                uiSize -= uiMatSize;
            }
            else
            {
                uiSize = 0;
                uiRet = EFLASH_RET_LENGTH_FAIL;
                break;
            }

        }

        if (uiSize > 0)
        {
            uiCntSector = uiSize / uiSectorSize;

            for (uiI = 0; uiI < uiCntSector; uiI ++)
            {
                uiRet = EFLASH_EraseMode(uiAddress, FLS_FRWCON_SECTOR_ERASE, uiEfType);

                uiAddress += uiSectorSize;

                if(uiRet != EFLASH_RET_OK)
                {
                    uiSize = 0;
                    break;
                }
                else if(uiSize >= uiSectorSize)
                {
                    uiSize -= uiSectorSize;
                }
                else
                {
                    uiSize = 0;
                    uiRet = EFLASH_RET_LENGTH_FAIL;
                    break;
                }

            }
        }

        if (uiSize > 0 && uiSize < uiSectorSize )
        {
            uiCntPage = uiSize / uiPageSize;
            uiCntRemainder = uiSize % uiPageSize;

            if(uiCntRemainder > 0)
            {
                uiCntPage ++;
            }

            for (uiI = 0; uiI < uiCntPage; uiI ++)
            {
                uiRet = EFLASH_EraseMode(uiAddress, FLS_FRWCON_PAGE_ERASE, uiEfType);
                uiAddress += uiPageSize;

                if(uiRet != EFLASH_RET_OK)
                {
                    break;
                }

            }

        }
    }

    return uiRet;
}

/*
***************************************************************************************************
*                                              Interface FUNCTION
***************************************************************************************************
*/



/*
***************************************************************************************************
*                                       EFLASH_Init
*
* Init write buffer
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
uint32 EFLASH_Init
(
    void
)
{
    uint32 uiRet = EFLASH_RET_OK;
    /* temp code */
    uiDataBuffer = (uint8 *)MPU_GetDMABaseAddress();
    return uiRet;
}


/*
***************************************************************************************************
*                                       EFLASH_FWDN_LowFormat
*
* Init write buffer
*
* @param    uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_LowFormat
(
    EF_type uiEfType
)
{
    uint32 uiRet  = EFLASH_RET_OK;
    uint32 uiSize = EF_SIZE_2M;

    if(uiEfType == EF_DFLASH)
    {
        uiSize = EF_SIZE_32K;
    }
    uiRet = EFLASH_Erase(0, uiSize, uiEfType);

    return uiRet;
}

/*
***************************************************************************************************
*                                       EFLASH_FWDN_Read
*
* Copy data from EFLASH
*
* @param    uiAddress : Read Address offset(Source)
*           uiLength  : Read Size
*           pBuf      : Dest Buff
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Read
(
    uint32  uiAddress,
    uint32  uiLength,
    void    *pBuf,
    EF_type uiEfType
)
{
    uint32 uiRet           = EFLASH_RET_PARAM_OUTOFRANGE;
    uint32 uiAddrMaxRange  = 0;

    if(uiEfType == EF_PFLASH)
    {
        uiAddrMaxRange     = EF_SIZE_2M;
    }
    else
    {
        uiAddrMaxRange     = EF_SIZE_128K;
    }

    if ((uiAddress + uiLength) > (uiAddrMaxRange))
    {
        EFLASH_D("[%s] Exceed address Range\n", __func__);
    }
    else if (uiLength == 0)
    {
        EFLASH_D("[%s]length is 0\n", __func__);
    }
    else if (pBuf == NULL)
    {
        EFLASH_D("[%s] null ptr\n", __func__);
    }
    else
    {
        EFLASH_ReadByte(uiAddress, pBuf, uiLength, uiEfType);
        uiRet = EFLASH_RET_OK;
    }

    return uiRet;
}

/*
***************************************************************************************************
*                                       EFLASH_FWDN_Write
*
* Copy data from EFLASH
*
* @param    uiAddress : Read Address offset(Dest)
*           uiLength  : Read Size
*           pBuf      : Source Buff
*           uiEfType  : EF_PFLASH, EF_DFLASH
*
* @return
*           EFLASH_RET_OK,
*           EFLASH_RET_TIMEOUT,
*           EFLASH_RET_PARAM_OUTOFRANGE,
*           EFLASH_RET_LENGTH_FAIL,
*           EFLASH_RET_DRV_NOT_INIT_YET,
*           EFLASH_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Write
(
    uint32 uiAddress,
    uint32 uiLength,
    void *pBuf,
    EF_type uiEfType
)
{
    uint32 uiRet           = EFLASH_RET_OK;
    sint32 sRet            = 0;   /* to get the return of SAL_MemCmp */
    uint32 uiAddrMaxRange  = 0;
    uint32 uiPageSize      = 0;
    uint32 uiPageAlignMask = 0;
    uint32 uiSize          = 0;
    uint32 uiI             = 0;
    uint32 uiTotalPageCnt  = 0;
    uint8  *pData;

    if(uiEfType == EF_PFLASH)
    {
        uiAddrMaxRange     = EF_SIZE_2M;
    }
    else
    {
        uiAddrMaxRange     = EF_SIZE_128K;
    }

    if(uiEfType == EF_PFLASH)
    {
        /*??Erase Unit: 2 KB (Page), 8 KB (Sector), 512 KB (Bank) */
        uiAddrMaxRange     = EF_SIZE_2M;
        uiPageSize         = EF_SIZE_2K;
    }
    else
    {
        /*??Erase Unit: 256 B (Page), 1 KB (Sector), 32 KB (Bank) */
        uiAddrMaxRange     = EF_SIZE_128K;
        uiPageSize         = EF_SIZE_256B;
    }

    uiPageAlignMask = (uiPageSize - 1u);

    EFLASH_D("%s[%d]: address=0x%X, length=0x%X, buf=0x%p\n", __func__, __LINE__, uiAddress, uiLength, pBuf);

    if (uiAddress + uiLength > (uiAddrMaxRange))
    {
        EFLASH_D("[%s] Exceed address Range\n", __func__);
        uiRet = EFLASH_RET_PARAM_OUTOFRANGE;
    }
    else if (uiLength == 0)
    {
        EFLASH_D("[%s] length is 0\n", __func__);
        uiRet = EFLASH_RET_PARAM_OUTOFRANGE;
    }
    else if (pBuf == NULL)
    {
        EFLASH_D("[%s] null ptr\n", __func__);
        uiRet = EFLASH_RET_PARAM_OUTOFRANGE;
    }
    else if (uiDataBuffer == NULL)
    {
        EFLASH_D("[%s] Write Buff is null ptr\n", __func__);
        uiRet = EFLASH_RET_DRV_NOT_INIT_YET;
    }
    else
    {
        pData = (unsigned char  *)pBuf;

        if ((uiAddress % uiPageSize) != 0)
        {
            /* PAGE Read */
            EFLASH_ReadByte((uiAddress & ~(uiPageAlignMask)), (void *)uiDataBuffer, uiPageSize, uiEfType);
            uiI= (uiAddress & uiPageAlignMask);

            if (uiLength > (uiPageSize - uiI))
            {
                uiSize = (uiPageSize - uiI);
            }
            else
            {
                uiSize = uiLength;
            }

            EFLASH_D("Erase & Write less than 2KB of data, address: 0x%X\n", uiAddress);
            SAL_MemCmp((const void *)&uiDataBuffer[uiI], (const void *)pData, uiSize, &sRet);

            if (sRet != 0)
            {
                /* Data Merge */
                SAL_MemCopy(&uiDataBuffer[uiI], pData, uiSize);

                uiRet = EFLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                if(uiRet != EFLASH_RET_OK)
                {
                    return uiRet;
                }

                uiRet = EFLASH_WriteByte((uiAddress & ~(uiPageAlignMask)), uiDataBuffer, uiPageSize, uiEfType);

                if(uiRet != EFLASH_RET_OK)
                {
                    return uiRet;
                }
            }

            uiAddress += uiSize;
            pData += uiSize;

            if(uiLength >= uiSize)
            {
                uiLength -= uiSize;
            }
            else
            {
                uiLength = 0;
                uiRet = EFLASH_RET_LENGTH_FAIL;
            }
        }

        /* Write data in 16 Bytes */
        uiTotalPageCnt = (uiLength / uiPageSize);
        EFLASH_D("Erase & Write data in 2KBytes, address: 0x%X, uiTotalPageCnt = %d\n", uiAddress, uiTotalPageCnt);

        for (uiI = 0; uiI < uiTotalPageCnt; uiI++)
        {
            EFLASH_ReadByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);

            SAL_MemCmp((const void *)uiDataBuffer, (const void *)pData, uiPageSize, &sRet);

            if (sRet != 0)
            {
                uiRet = EFLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                if(uiRet != 0)
                {
                    return uiRet;
                }

                uiRet = EFLASH_WriteByte(uiAddress, pData, uiPageSize, uiEfType);

                if(uiRet != 0)
                {
                    return uiRet;
                }
            }

            uiAddress += uiPageSize;
            pData += uiPageSize;

            if(uiLength >= uiPageSize)
            {
                uiLength -= uiPageSize;
            }
            else
            {
                uiLength = 0;
                uiRet = EFLASH_RET_LENGTH_FAIL;
            }
        }

        /* Remaining data */
        if (uiLength > 0)
        {
            if(uiLength > uiPageSize)
            {
                uiRet = EFLASH_RET_LENGTH_FAIL;
            }
            else
            {
                EFLASH_D("Erase & Write remaining data, address: 0x%X, length=%d\n", uiAddress, uiLength);
                EFLASH_ReadByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);
                SAL_MemCmp((const void *)uiDataBuffer, (const void *)pData, uiLength, &sRet);
                if (sRet != 0)
                {
                    SAL_MemCopy(uiDataBuffer, pData, uiLength);

                    uiRet = EFLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                    if(uiRet != 0)
                    {
                        return uiRet;
                    }

                    uiRet = EFLASH_WriteByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);

                    if(uiRet != 0)
                    {
                        return uiRet;
                    }

                 }
            }
        }
    }
    return uiRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

