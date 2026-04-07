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

#include <debug.h>
#include <stdlib.h>
#include <sal_internal.h>

#include <bsp.h>
#include <mpu.h>
#include "eflash.h"


#if (DEBUG_ENABLE)
    #include "debug.h"
    #define FLASH_D(fmt,args...)       {LOGD(DBG_TAG_EFLASH, fmt, ## args)}
    #define FLASH_E(fmt,args...)       {LOGE(DBG_TAG_EFLASH, fmt, ## args)}
#else
    #define FLASH_D(fmt,args...)
    #define FLASH_E(fmt,args...)
#endif

#define FCONT_SIZE_16B             (0x10u)

#define FCONT_SIZE_256B            (0x0100u)

#define FCONT_SIZE_1K              (0x0400u)
#define FCONT_SIZE_2K              (0x0800u)
#define FCONT_SIZE_8K              (0x2000u)


#define FCONT_SIZE_32K             (0x8000u)
#define FCONT_SIZE_64K             (0x10000u)


#define FCONT_SIZE_128K            (0x20000u)
#define FCONT_SIZE_256K            (0x40000u)


#define FCONT_SIZE_512K            (0x80000u)

#define FCONT_SIZE_2M              (0x200000u)


#define PFLASH_ADDRESS                  (0xA1000000u)
#define DFLASH_ADDRESS                  (0xA1080000u)

#define FCONT_TIMEOUT_CNT          (0x30000u)


#define PFLASH_BASE_PTR                 (0x20000000u)
#define DFLASH_BASE_PTR                 (0x30000000u)

#define FCONT_FRWCON_PROG          (0x00000002U)
#define FCONT_FRWCON_ERASE         (0x00000001U)

#define EF_LDT1_BASE                   (0xA1011800u)

#ifdef __GNU_C__

#define FCONT_CACHE_FLUSH()            \
    asm("DSB");                         \
    asm("MCR p15, 0, r0, c7, c5, 0");   \
    asm("MCR p15, 0, r0, c15, c5, 0");  \
    asm("ISB")

#else

__asm volatile void FCONT_CACHE_FLUSH()
{
    DSB;
    MCR p15, 0, r0, c7, c5, 0;
    MCR p15, 0, r0, c15, c5, 0;
    ISB;
}
#endif






enum {
    MAT0                                = (uint32)0x1u,
    MAT1                                = (uint32)0x2u,
    MAT2                                = (uint32)0x4u,
    MAT3                                = (uint32)0x8u,
    MATx                                = (uint32)0xFu,
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
#define FCONT_STS_ERASE_FAIL               (0x00000001U)
#define FCONT_STS_READ_FAIL                (0x00000002U)
#define FCONT_STS_PROG_ADDR_FAIL           (0x00000004U)
#define FCONT_STS_PROG_DATA_FAIL           (0x00000008U)

#define FCONT_STS_CMD_READY                (0x00000010U)
#define FCONT_STS_AND_READY                (0x00000020U)
#define FCONT_STS_ADDR_LATCHED             (0x00000040U)
#define FCONT_STS_DATA_LATCHED             (0x00000080U)
#define FCONT_STS_MAT0_BUSY                (0x00000100U)
#define FCONT_STS_MAT1_BUSY                (0x00000200U)
#define FCONT_STS_MAT2_BUSY                (0x00000400U)
#define FCONT_STS_MAT3_BUSY                (0x00000800U)
#define FCONT_STS_MATx_BUSY                (0x00000F00U)

#define STS_MAT_SHIFT                   (8u)

#define FLASHCONT_SET_ERASEMODE(x)             ((((uint32)(x) & 0x3u) << 3U) | (FCONT_FRWCON_ERASE))

typedef enum {
    FLS_FRWCON_PAGE_ERASE               = 0u,
    FLS_FRWCON_SECTOR_ERASE             = 1u,
    FLS_FRWCON_MAT_ERASE                = 2u,
    FLS_FRWCON_SAMEAS_SECTOR_ERASE      = 3u,
} FCONT_Erase_Mode;


/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/**************************************************************************************************/

static uint8 * uiDataBuffer = (uint8 *)0;

#define PFLASH_FRWCON                 (PFLASH_ADDRESS + 0x000u)
#define PFLASH_FSHSTAT                (PFLASH_ADDRESS + 0x004u)
#define PFLASH_FSHADDR                (PFLASH_ADDRESS + 0x200u)
#define PFLASH_FSHWDATA_1             (PFLASH_ADDRESS + 0x204u)
#define PFLASH_FSHWDATA_2             (PFLASH_ADDRESS + 0x210u)
#define PFLASH_FSHWDATA_3             (PFLASH_ADDRESS + 0x214u)
#define PFLASH_FSHWDATA_4             (PFLASH_ADDRESS + 0x218u)

#define DFLASH_FRWCON                 (DFLASH_ADDRESS + 0x000u)
#define DFLASH_FSHSTAT                (DFLASH_ADDRESS + 0x004u)
#define DFLASH_FSHADDR                (DFLASH_ADDRESS + 0x200u)
#define DFLASH_FSHWDATA_1             (DFLASH_ADDRESS + 0x204u)
#define DFLASH_FSHWDATA_2             (DFLASH_ADDRESS + 0x210u)

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
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type EFLASH_BusyCheck
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
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
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
*  (FCONT_RET_TIMEOUT is returned if FSHStatus is not equal to the uiExpectStatus value
*   until passing FCONT_TIMEOUT_CNT.)
*
* @param    uiStatus       : FSHStatus bitMask
*           uiExpectStatus : Expected status
*           uiEfType       : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type EFLASH_CheckFSHStatus
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
*  (FCONT_RET_TIMEOUT is returned if Mats are still busy until passing FCONT_TIMEOUT_CNT.)
*
* @param    uiMatNumFlag   : Mat Flag, each bit field means mat number, refer to eflash.h
*           uiEfType       : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type EFLASH_WaitMatBusy
(
    uint32  uiMatNumFlag,
    EF_type uiEfType
);


/*
***************************************************************************************************
*                                       FLASH_EraseMode
*
*  Erase eflash data
*
* @param    uiAddress : erase start address
*           uiMode    : erase mode, refer to FCONT_Erase_Mode at the eflash.h
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type FLASH_EraseMode
(
    uint32 uiAddress,
    FCONT_Erase_Mode uiMode,
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
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
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
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
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


/*
***************************************************************************************************
*                                       EFLASH_CheckDFLASHAreaAllow
*
*  check the area is belong to R5
*
* @param    uiStartAddress : address
*           uiSize
: size
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_NOT_ALLOWD_AREA
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type EFLASH_CheckDFLASHAreaAllow
(
    const uint32 uiStartAddress,
    const uint32 uiSize
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
*           FCONT_RET_OK,
*           FCONT_RET_D_SEMA_FAIL
*
* Notes
*
***************************************************************************************************
*/

static FCONT_RET_type Fls_CheckNSetHWSema
(
    const uint32 Set
);


/*
***************************************************************************************************
*                                              LOCAL FUNCTION
***************************************************************************************************
*/
static inline uint32 FLASH_READREG(uint32 uiAddr)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

static inline void FLASH_WRITEREG(uint32 uiValue, uint32 uiAddr)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}


static FCONT_RET_type EFLASH_CheckDFLASHAreaAllow
(
    const uint32 uiStartAddress,
    const uint32 uiSize
)
{
    FCONT_RET_type uiRet;
    uint32 LockNSel = FLASH_READREG(FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS);

    uint8 Lock = (uint8)((LockNSel & 0x40u) >> 6u);
    uint8 Sel  = (uint8)((LockNSel & 0x30u) >> 4u);
    uint8 Pkg  = (uint8)(LockNSel & 0x03u);
    uint8 MatNum  = (uint8)((uiStartAddress / FLS_HARDWARE_E_DFLASH_MAT_UNIT) & 0xFFu);
    uint8 Offset  = (uint8)(((uiStartAddress % FLS_HARDWARE_E_DFLASH_MAT_UNIT) /FLS_HARDWARE_E_DFLASH_PKG_UNIT) & 0xFFu);


    uint8 MatNum_end  = 0u;
    uint8 Offset_end  = 0u;

    if((uiStartAddress < FCONT_SIZE_256K) && (uiSize < FCONT_SIZE_256K) && ((uiStartAddress + uiSize) <= FCONT_SIZE_256K))
    {/* CERT INT30-C (uiStartAddress + uiSize) */
        MatNum_end  = (uint8)(((uiStartAddress + uiSize - 1u) / FLS_HARDWARE_E_DFLASH_MAT_UNIT) & 0xFFu);
        Offset_end  = (uint8)((((uiStartAddress + uiSize - 1u) % FLS_HARDWARE_E_DFLASH_MAT_UNIT) /FLS_HARDWARE_E_DFLASH_PKG_UNIT) & 0xFFu);
    }

    if(Lock == 0u)
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else if((Sel > MatNum) || (Sel > MatNum_end))
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else if((Offset > Pkg) || (Offset_end > Pkg))
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else
    {
        uiRet = FCONT_RET_OK;
    }

    return uiRet;
}

static FCONT_RET_type Fls_CheckNSetHWSema
(
    const uint32 Set
)
{
    FCONT_RET_type uiRet = FCONT_RET_OK;
    uint32 HwSema = FLASH_READREG(FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);

    if(Set == 1u)
    {
        FLASH_WRITEREG(0x1u, FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);
        HwSema = FLASH_READREG(FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);

        if(HwSema != 1u)
        {
            uiRet = FCONT_RET_D_SEMA_FAIL;
        }
    }
    else
    {
        FLASH_WRITEREG(0x0u, FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);
        HwSema = FLASH_READREG(FLS_HARDWARE_E_DFLASH_SEMA_ADDRESS);

        if(HwSema != 0u)
        {
            uiRet = FCONT_RET_D_SEMA_FAIL;
        }
    }

    return uiRet;
}







static FCONT_RET_type EFLASH_BusyCheck
(
    uint32  uiAddress,
    uint32  uiSize,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet = FCONT_RET_OK;
    uint32 uiMatNumFlag = 0;

    FLASH_D("%s\n", __func__);

    uiMatNumFlag = EFLASH_getMATNum(uiAddress, uiSize, uiEfType);

    if( uiMatNumFlag == 0u )
    {
        FLASH_D("[%s] get mat num error\n", __func__);
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

    FLASH_D("%s\n", __func__);

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        uiAddrMaxRange = FCONT_SIZE_2M;
        uiSizeMax      = FCONT_SIZE_512K;
    }
    else
    {
        uiAddrMaxRange = FCONT_SIZE_256K;
        uiSizeMax      = FCONT_SIZE_64K;
    }

    if (uiAddress  > uiAddrMaxRange)
    {
        FLASH_D("[%s]Not allowed address \n", __func__);
        uiRet = 0u;
    }
    else if (uiSize > uiSizeMax)
    {
        FLASH_D("[%s] Size is over than Mat size\n", __func__);
        uiRet = 0u;
    }
    else
    {
        if(uiAddress < uiSizeMax)
        {
            uiRet = (uint32)MAT0;
        }
        else if (uiAddress < (uiSizeMax*2u))
        {
            uiRet = (uint32)MAT1;
        }
        else if (uiAddress < (uiSizeMax*3u))
        {
            uiRet = (uint32)MAT2;
        }
        else
        {
            uiRet = (uint32)MAT3;
        }

        if ((uiAddress + uiSize) <= (uiSizeMax))
        {
            ;
        }
        else if((uiAddress + uiSize) <= (uiSizeMax*2u))
        {
            uiRet |= (uint32)MAT1;
        }
        else if ((uiAddress + uiSize) <= (uiSizeMax*3u))
        {
            uiRet |= (uint32)MAT2;
        }
        else //  ((address+size) < (FCONT_SIZE_512K*4))
        {
            uiRet |= (uint32)MAT3;
        }

    }
    return uiRet;

}

static FCONT_RET_type EFLASH_CheckFSHStatus
(
    uint32  uiStatus,
    uint32  uiExpectStatus,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet      = FCONT_RET_OK;
    uint32 uiRetryCnt = FCONT_TIMEOUT_CNT;
    uint32 uiFshStat;

    FLASH_D("%s\n", __func__);

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        uiFshStat = FLASH_READREG(PFLASH_FSHSTAT);
    }
    else
    {
        uiFshStat = FLASH_READREG(DFLASH_FSHSTAT);
    }

    while(((uiFshStat & uiStatus) != uiExpectStatus) && (uiRetryCnt > 0u))
    {
        if(uiEfType == FCONT_TYPE_PFLASH)
        {
            uiFshStat = FLASH_READREG(PFLASH_FSHSTAT);
        }
        else
        {
            uiFshStat = FLASH_READREG(DFLASH_FSHSTAT);
        }

        uiRetryCnt--;

    }

    if(uiRetryCnt == 0u)
    {
        FLASH_D("flash x%x] status x%x expect_status x%x timeout \n",uiEfType, uiStatus, uiExpectStatus);
        uiRet = FCONT_RET_TIMEOUT;
    }

    return uiRet;
}


static FCONT_RET_type EFLASH_WaitMatBusy
(
    uint32  uiMatNumFlag,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet = FCONT_RET_OK;

    uint32 uiMatnumStatus = ((uiMatNumFlag & (uint32)MATx) << STS_MAT_SHIFT);

    FLASH_D("%s\n", __func__);

    uiRet = EFLASH_CheckFSHStatus(uiMatnumStatus, 0, uiEfType);


    return uiRet;
}

static FCONT_RET_type FLASH_EraseMode
(
    uint32 uiAddress,
    FCONT_Erase_Mode uiMode,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet = FCONT_RET_OK;

    uint32 uiSize = 0u;

    FLASH_D("%s\n", __func__);
    if(uiAddress > FCONT_SIZE_2M)
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else
    {

        if(uiEfType == FCONT_TYPE_PFLASH)
        {
            if(uiMode == FLS_FRWCON_PAGE_ERASE)
            {
                uiSize = FCONT_SIZE_2K;
            }
            else if(uiMode == FLS_FRWCON_SECTOR_ERASE)
            {
                uiSize = FCONT_SIZE_8K;
            }
            else
            {
                uiSize = FCONT_SIZE_512K;
            }
        }
        else
        {
            if(uiMode == FLS_FRWCON_PAGE_ERASE)
            {
                uiSize = FCONT_SIZE_256B;
            }
            else if(uiMode == FLS_FRWCON_SECTOR_ERASE)
            {
                uiSize = FCONT_SIZE_1K;
            }
            else
            {
                uiSize = FCONT_SIZE_64K;
            }
        }


        if(uiEfType == FCONT_TYPE_DFLASH)
        {
            if(EFLASH_CheckDFLASHAreaAllow(uiAddress, uiSize) == FCONT_RET_OK)
            {
                uiRet = Fls_CheckNSetHWSema(1u);
            }
            else
            {
                uiRet = FCONT_RET_NOT_ALLOWD_AREA;
            }
        }

        if(uiRet == FCONT_RET_OK)
        {
            uiRet = EFLASH_BusyCheck(uiAddress, uiSize, uiEfType);

            if(uiRet == FCONT_RET_OK)
            {
                uiRet = EFLASH_CheckFSHStatus( FCONT_STS_ADDR_LATCHED, 0, uiEfType);

                if(uiRet == FCONT_RET_OK)
                {
                    if(uiEfType == FCONT_TYPE_PFLASH)
                    {
                        FLASH_WRITEREG(uiAddress, PFLASH_FSHADDR);
                        FLASH_WRITEREG(FLASHCONT_SET_ERASEMODE(uiMode), PFLASH_FRWCON);
                    }
                    else
                    {
                        FLASH_WRITEREG(uiAddress, DFLASH_FSHADDR);
                        FLASH_WRITEREG(FLASHCONT_SET_ERASEMODE(uiMode), DFLASH_FRWCON);
                    }

                    uiRet = EFLASH_BusyCheck(uiAddress, uiSize, uiEfType);
                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = EFLASH_CheckFSHStatus( (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                                     (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                                      uiEfType);
                    }
                    FCONT_CACHE_FLUSH();
                }
            }

            if(uiEfType == FCONT_TYPE_DFLASH)
            {
                (void)Fls_CheckNSetHWSema(0u);
            }

        }
    }

    return uiRet;
}


uint32 EFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength,
    EF_type uiEfType
)
{
    uint32 uiRet               = (uint32)FCONT_RET_OK;

    if(uiEfType == FCONT_TYPE_PFLASH)
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
    uint32 uiSourceAddr = 0u;

    if(address < FCONT_SIZE_2M)
    {
        if(uiEfType == FCONT_TYPE_PFLASH)
        {
            uiSourceAddr = PFLASH_BASE_PTR + address;
        }
        else
        {
            uiSourceAddr = DFLASH_BASE_PTR + address;
        }

        SAL_MemCopy(pBuffer, (const void *)uiSourceAddr, length);
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
    FCONT_RET_type uiRet               = FCONT_RET_OK;
    const uint32 * pSrc = (const uint32 *)pBuffer;
    uint32 uiBuf = 0xFFFFFFFFu;
    uint32 uiTargetAddr        = uiAddress;

    FLASH_D("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", uiAddress, pBuffer, uiLength);

    if((uiAddress > FCONT_SIZE_2M) || (uiLength > FCONT_SIZE_2M) || ((uiAddress + uiLength) > FCONT_SIZE_2M))
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else if((uiAddress & 0xFu) != 0u ) /* 16 bytes align needed */
    {
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else
    {
        uiRet = EFLASH_BusyCheck(uiTargetAddr, uiLength, FCONT_TYPE_PFLASH);
    }

    while((uiLength > 0u) && (uiRet == FCONT_RET_OK))
    {
        uiRet = EFLASH_CheckFSHStatus(FCONT_STS_AND_READY, FCONT_STS_AND_READY, FCONT_TYPE_PFLASH);
        if(uiRet == FCONT_RET_OK)
        {
            FLASH_WRITEREG(uiTargetAddr, PFLASH_FSHADDR);
            if(uiLength >= 4u)
            {
                FLASH_WRITEREG(*pSrc, PFLASH_FSHWDATA_1);
                pSrc ++;
                uiLength = uiLength - 4u;

                if(uiLength >= 4u)
                {
                    FLASH_WRITEREG(*pSrc, PFLASH_FSHWDATA_2);
                    pSrc ++;
                    uiLength = uiLength - 4u;

                    if(uiLength >= 4u)
                    {
                        FLASH_WRITEREG(*pSrc, PFLASH_FSHWDATA_3);
                        pSrc ++;
                        uiLength = uiLength - 4u;

                        if(uiLength >= 4u)
                        {
                            FLASH_WRITEREG(*pSrc, PFLASH_FSHWDATA_4);
                            pSrc ++;
                            uiLength = uiLength - 4u;
                        }
                        else
                        {
                            SAL_MemCopy(&uiBuf, pSrc, uiLength);
                            FLASH_WRITEREG(uiBuf, PFLASH_FSHWDATA_4);
                            uiLength = 0;
                        }
                    }
                    else
                    {
                        SAL_MemCopy(&uiBuf, pSrc, uiLength);
                        FLASH_WRITEREG(uiBuf, PFLASH_FSHWDATA_3);
                        FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_4);
                        uiLength = 0;
                    }
                }
                else
                {
                    SAL_MemCopy(&uiBuf, pSrc, uiLength);
                    FLASH_WRITEREG(uiBuf, PFLASH_FSHWDATA_2);
                    FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_3);
                    FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_4);

                    uiLength = 0;
                }

            }
            else
            {
                SAL_MemCopy(&uiBuf, pSrc, uiLength);
                FLASH_WRITEREG(uiBuf, PFLASH_FSHWDATA_1);
                FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_2);
                FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_3);
                FLASH_WRITEREG(0xFFFFFFFFu, PFLASH_FSHWDATA_4);
                uiLength = 0;
            }

            FLASH_WRITEREG(FCONT_FRWCON_PROG, PFLASH_FRWCON);
            uiRet = EFLASH_BusyCheck(uiTargetAddr, 16u, FCONT_TYPE_PFLASH);
            uiTargetAddr += 16u;
            if(uiRet == FCONT_RET_OK)
            {
                uiRet = EFLASH_CheckFSHStatus( (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                               (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                               FCONT_TYPE_PFLASH);
            }
        }


    }


    FCONT_CACHE_FLUSH();

    return (uint32)uiRet;
}

uint32 EFLASH_DFLASH_WriteByte
(
    uint32 uiAddress,
    const void *pBuffer,
    uint32 uiLength
)
{
    FCONT_RET_type uiRet               = FCONT_RET_OK;
    const uint32 * pSrc = (const uint32 *)pBuffer;
    uint32 uiBuf = 0xFFFFFFFFu;
    uint32 uiTargetAddr        = uiAddress;

    FLASH_D("Write address: 0x%08X, buffer: 0x%08X, length: %d\n", uiAddress, (uint32)&pBuffer, uiLength);

    if((uiTargetAddr > FCONT_SIZE_2M) || (uiLength > FCONT_SIZE_2M))
    {
        uiRet = FCONT_RET_NOT_ALLOWD_AREA;
    }
    else if((uiAddress & 0x7u) != 0u ) /* 8 bytes align needed */
    {
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else
    {
        if(EFLASH_CheckDFLASHAreaAllow(uiTargetAddr, uiLength) == FCONT_RET_OK)
        {

            if(Fls_CheckNSetHWSema(1u) == FCONT_RET_OK)
            {
                uiRet = EFLASH_BusyCheck(uiTargetAddr, uiLength, FCONT_TYPE_DFLASH);

                while((uiLength > 0u) && (uiRet == FCONT_RET_OK))
                {
                    uiRet = EFLASH_CheckFSHStatus(FCONT_STS_AND_READY, FCONT_STS_AND_READY, FCONT_TYPE_DFLASH);
                    if(uiRet ==FCONT_RET_OK)
                    {
                        FLASH_WRITEREG(uiTargetAddr, DFLASH_FSHADDR);

                        if(uiLength >= 4u)
                        {
                            FLASH_WRITEREG( *pSrc, DFLASH_FSHWDATA_1);
                            pSrc ++;
                            uiLength = uiLength - 4u;

                            if(uiLength >= 4u)
                            {
                                FLASH_WRITEREG( *pSrc, DFLASH_FSHWDATA_2);
                                pSrc ++;
                                uiLength = uiLength - 4u;

                            }
                            else
                            {
                                SAL_MemCopy(&uiBuf, pSrc, uiLength);
                                FLASH_WRITEREG(uiBuf, DFLASH_FSHWDATA_2);
                                uiLength = 0;
                            }

                        }
                        else
                        {
                            SAL_MemCopy(&uiBuf, pSrc, uiLength);
                            FLASH_WRITEREG(uiBuf, DFLASH_FSHWDATA_1);
                            FLASH_WRITEREG(0xFFFFFFFFu, DFLASH_FSHWDATA_2);
                            uiLength = 0;
                        }

                        FLASH_WRITEREG(FCONT_FRWCON_PROG, DFLASH_FRWCON);
                        uiRet = EFLASH_BusyCheck(uiTargetAddr, 8u, FCONT_TYPE_DFLASH);
                        if(uiRet == FCONT_RET_OK)
                        {
                            uiRet = EFLASH_CheckFSHStatus( (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                                           (FCONT_STS_AND_READY | FCONT_STS_CMD_READY), \
                                                           FCONT_TYPE_DFLASH);
                        }
                    }
                    uiTargetAddr += 8u;

                }
                (void)Fls_CheckNSetHWSema(0u);
            }

        }
    }

    FCONT_CACHE_FLUSH();

    return (uint32)uiRet;
}

uint32 EFLASH_Erase
(
    uint32 uiAddress,
    uint32 uiSize,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet                = FCONT_RET_OK;
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

    FLASH_D("%s\n", __func__);

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        /*??Erase Unit: 2 KB (Page), 8 KB (Sector), 512 KB (Bank) */
        uiAddrMaxRange          = FCONT_SIZE_2M;
        uiMatSize               = FCONT_SIZE_512K;
        uiSectorSize            = FCONT_SIZE_8K;
        uiPageSize              = FCONT_SIZE_2K;
    }
    else
    {
        /*??Erase Unit: 256 B (Page), 1 KB (Sector), 32 KB (Bank) */
        uiAddrMaxRange          = FCONT_SIZE_256K;
        uiMatSize               = FCONT_SIZE_64K;
        uiSectorSize            = FCONT_SIZE_1K;
        uiPageSize              = FCONT_SIZE_256B;
    }

    uiAddrStartAlignMask = (uiPageSize - 1u);

    if (((uiAddress & uiAddrStartAlignMask) != 0u )
        || (uiSize == 0u) || (uiAddress  > uiAddrMaxRange)
        || ( (uiAddress + uiSize) > uiAddrMaxRange))
    {
        //Address Error
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else
    {
        //check uiSize is more than 2 Mat or 1Mat at 0 position

        uiCntMat = uiSize / uiMatSize;

        if((uiCntMat >= 2u)||((uiCntMat == 1u) && ((uiAddress % uiMatSize) != 0u)))
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
            if((uiAddress % uiSectorSize) == 0u)
            {
                uiRet = FLASH_EraseMode(uiAddress, FLS_FRWCON_SECTOR_ERASE, uiEfType);

                uiAddress += uiSectorSize;

                if((uiRet != FCONT_RET_OK) || (uiSize < uiSectorSize))
                {
                    uiSize = 0u;
                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = FCONT_RET_LENGTH_FAIL;
                    }
                }
                else
                {
                    uiSize -= uiSectorSize;
                }

            }
            else
            {
                uiRet = FLASH_EraseMode(uiAddress, FLS_FRWCON_PAGE_ERASE, uiEfType);
                uiAddress += uiPageSize;

                if((uiRet != FCONT_RET_OK) || (uiSize < uiPageSize))
                {
                    uiSize = 0u;
                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = FCONT_RET_LENGTH_FAIL;
                    }
                }
                else
                {
                    uiSize -= uiPageSize;
                }

            }

        }


        /////////////////////////////////
        //middle erase
        //recalculate uiCntMat
        uiCntMat = uiSize / uiMatSize;

        for (uiI = 0u; uiI < uiCntMat; uiI++)
        {
            uiRet = FLASH_EraseMode(uiAddress, FLS_FRWCON_MAT_ERASE, uiEfType);

            if((uiRet != FCONT_RET_OK) || (uiSize < uiMatSize) || ((uiAddress + uiMatSize) > uiAddrMaxRange))
            {
                uiSize = 0u;
                if(uiRet == FCONT_RET_OK)
                {
                    uiRet = FCONT_RET_LENGTH_FAIL;
                }
                break;
            }
            else
            {
                uiAddress += uiMatSize;
                uiSize -= uiMatSize;
            }

        }

        if (uiSize > 0u)
        {
            uiCntSector = uiSize / uiSectorSize;

            for (uiI = 0u; uiI < uiCntSector; uiI ++)
            {
                uiRet = FLASH_EraseMode(uiAddress, FLS_FRWCON_SECTOR_ERASE, uiEfType);

                uiAddress += uiSectorSize;

                if((uiRet != FCONT_RET_OK) || (uiSize < uiSectorSize))
                {
                    uiSize = 0u;
                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = FCONT_RET_LENGTH_FAIL;
                    }
                    break;
                }
                else
                {
                    uiSize -= uiSectorSize;
                }

            }
        }

        if ((uiSize > 0u) && (uiSize < uiSectorSize ))
        {
            uiCntPage = uiSize / uiPageSize;
            uiCntRemainder = uiSize % uiPageSize;

            if(uiCntRemainder > 0u)
            {
                uiCntPage ++;
            }

            for (uiI = 0u; uiI < uiCntPage; uiI ++)
            {
                uiRet = FLASH_EraseMode(uiAddress, FLS_FRWCON_PAGE_ERASE, uiEfType);
                uiAddress += uiPageSize;

                if(uiRet != FCONT_RET_OK)
                {
                    break;
                }

            }

        }
    }

    return (uint32)uiRet;
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
    uint32 uiRet = (uint32)FCONT_RET_OK;
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
* @param    uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
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
    uint32 uiRet  = (uint32)FCONT_RET_OK;
    uint32 uiSize = FCONT_SIZE_2M;

    if(uiEfType == FCONT_TYPE_DFLASH)
    {
        uiSize = FCONT_SIZE_64K;
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
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
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
    FCONT_RET_type uiRet           = FCONT_RET_PARAM_OUTOFRANGE;
    uint32 uiAddrMaxRange  = 0u;

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        uiAddrMaxRange     = FCONT_SIZE_2M;
    }
    else
    {
        uiAddrMaxRange     = FCONT_SIZE_256K;
    }

    if ((uiAddress  > uiAddrMaxRange) || (uiLength  > uiAddrMaxRange) || ((uiAddress + uiLength) > (uiAddrMaxRange)))
    {
        FLASH_D("[%s] Exceed address Range\n", __func__);
    }
    else if (uiLength == 0u)
    {
        FLASH_D("[%s]length is 0\n", __func__);
    }
    else if (pBuf == NULL)
    {
        FLASH_D("[%s] null ptr\n", __func__);
    }
    else
    {
        EFLASH_ReadByte(uiAddress, pBuf, uiLength, uiEfType);
        uiRet = FCONT_RET_OK;
    }

    return (uint32)uiRet;
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
*           uiEfType  : FCONT_TYPE_PFLASH, FCONT_TYPE_DFLASH
*
* @return
*           FCONT_RET_OK,
*           FCONT_RET_TIMEOUT,
*           FCONT_RET_PARAM_OUTOFRANGE,
*           FCONT_RET_LENGTH_FAIL,
*           FCONT_RET_DRV_NOT_INIT_YET,
*           FCONT_RET_INIT_FAIL
*
* Notes
*
***************************************************************************************************
*/

uint32 EFLASH_FWDN_Write
(
    uint32 uiAddress,
    uint32 uiLength,
    const uint8 *pBuf,
    EF_type uiEfType
)
{
    FCONT_RET_type uiRet           = FCONT_RET_OK;
    sint32 sRet            = 0;   /* to get the return of SAL_MemCmp */
    uint32 uiAddrMaxRange  = 0u;
    uint32 uiPageSize      = 0u;
    uint32 uiPageAlignMask = 0u;
    uint32 uiSize          = 0u;
    uint32 uiI             = 0u;
    uint32 uiTotalPageCnt  = 0u;
    uint32 uiSrcIdx        = 0u;
    const uint8 *pData;

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        uiAddrMaxRange     = FCONT_SIZE_2M;
    }
    else
    {
        uiAddrMaxRange     = FCONT_SIZE_256K;
    }

    if(uiEfType == FCONT_TYPE_PFLASH)
    {
        /*??Erase Unit: 2 KB (Page), 8 KB (Sector), 512 KB (Bank) */
        uiAddrMaxRange     = FCONT_SIZE_2M;
        uiPageSize         = FCONT_SIZE_2K;
    }
    else
    {
        /*??Erase Unit: 256 B (Page), 1 KB (Sector), 32 KB (Bank) */
        uiAddrMaxRange     = FCONT_SIZE_256K;
        uiPageSize         = FCONT_SIZE_256B;
    }

    uiPageAlignMask = (uiPageSize - 1u);

    FLASH_D("%s[%d]: address=0x%X, length=0x%X, buf=0x%p\n", __func__, __LINE__, uiAddress, uiLength, pBuf);

    if ((uiAddress  > uiAddrMaxRange) || (uiLength  > uiAddrMaxRange) || ((uiAddress + uiLength) > (uiAddrMaxRange)))
    {
        FLASH_D("[%s] Exceed address Range\n", __func__);
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else if (uiLength == 0u)
    {
        FLASH_D("[%s] length is 0\n", __func__);
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else if (pBuf == NULL)
    {
        FLASH_D("[%s] null ptr\n", __func__);
        uiRet = FCONT_RET_PARAM_OUTOFRANGE;
    }
    else if (uiDataBuffer == NULL)
    {
        FLASH_D("[%s] Write Buff is null ptr\n", __func__);
        uiRet = FCONT_RET_DRV_NOT_INIT_YET;
    }
    else
    {
        pData = pBuf;

        if ((uiAddress % uiPageSize) != 0u)
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

            FLASH_D("Erase & Write less than 2KB of data, address: 0x%X\n", uiAddress);
            SAL_MemCmp((const void *)&uiDataBuffer[uiI], (const void *)pData, uiSize, &sRet);

            if (sRet != 0)
            {
                /* Data Merge */
                SAL_MemCopy(&uiDataBuffer[uiI], pData, uiSize);

                uiRet = (FCONT_RET_type)FLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                if(uiRet == FCONT_RET_OK)
                {
                    uiRet = (FCONT_RET_type)EFLASH_WriteByte((uiAddress & ~(uiPageAlignMask)), uiDataBuffer, uiPageSize, uiEfType);
                }

            }

            uiAddress += uiSize;
            uiSrcIdx += uiSize;

            if(uiLength >= uiSize)
            {
                uiLength -= uiSize;
            }
            else
            {
                uiLength = 0u;
                if(uiRet != FCONT_RET_OK)
                {
                    uiRet = FCONT_RET_LENGTH_FAIL;
                }
            }
        }

        if(uiRet == FCONT_RET_OK)
        {
            /* Write data in 16 Bytes */
            uiTotalPageCnt = (uiLength / uiPageSize);
            FLASH_D("Erase & Write data in 2KBytes, address: 0x%X, uiTotalPageCnt = %d\n", uiAddress, uiTotalPageCnt);

            for (uiI = 0u; uiI < uiTotalPageCnt; uiI++)
            {
                EFLASH_ReadByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);

                SAL_MemCmp((const void *)uiDataBuffer, &pData[uiSrcIdx], uiPageSize, &sRet);

                if (sRet != 0)
                {
                    uiRet = (FCONT_RET_type)FLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = (FCONT_RET_type)EFLASH_WriteByte(uiAddress, &pData[uiSrcIdx], uiPageSize, uiEfType);
                    }
                }

                uiAddress += uiPageSize;

                if((uiLength >= uiPageSize) && (uiSrcIdx < uiAddrMaxRange))
                {
                    uiSrcIdx += uiPageSize;
                    uiLength -= uiPageSize;
                }
                else
                {
                    uiLength = 0u;
                    if(uiRet != FCONT_RET_OK)
                    {
                        uiRet = FCONT_RET_LENGTH_FAIL;
                    }
                }
            }
        }
        /* Remaining data */
        if((uiRet == FCONT_RET_OK) && (uiLength > 0u))
        {
            if(uiLength > uiPageSize)
            {
                uiRet = FCONT_RET_LENGTH_FAIL;
            }
            else
            {
                FLASH_D("Erase & Write remaining data, address: 0x%X, length=%d\n", uiAddress, uiLength);
                EFLASH_ReadByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);
                SAL_MemCmp((const void *)uiDataBuffer, &pData[uiSrcIdx], uiLength, &sRet);
                if (sRet != 0)
                {
                    SAL_MemCopy(uiDataBuffer, &pData[uiSrcIdx], uiLength);

                    uiRet = (FCONT_RET_type)FLASH_EraseMode((uiAddress & ~(uiPageAlignMask)), FLS_FRWCON_PAGE_ERASE, uiEfType);

                    if(uiRet == FCONT_RET_OK)
                    {
                        uiRet = (FCONT_RET_type)EFLASH_WriteByte(uiAddress, uiDataBuffer, uiPageSize, uiEfType);
                    }

                 }
            }
        }
    }
    return (uint32)uiRet;
}

void FLASH_ReadLDT
(
    uint32 uiAddress,
    uint32 uiCount,
    uint32 *pBuf
)
{
    uint32 addr, i;

    for (i = 0; i < uiCount; i++)
    {
        addr = EF_LDT1_BASE + (uiAddress + ((i / 4) * 0x40));
        pBuf[i] = FLASH_READREG(addr + ((i % 4) * 4));
    }
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_EFLASH == 1 )

