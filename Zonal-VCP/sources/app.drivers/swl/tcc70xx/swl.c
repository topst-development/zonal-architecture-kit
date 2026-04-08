// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : swl.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_SWL == 1 )

#include <debug.h>
#include <stdlib.h>
#include <sal_internal.h>

#include <bsp.h>
#include <mpu.h>
#include "swl.h"
#include "eflash.h"


#if (DEBUG_ENABLE)
    #include "debug.h"
    #define SWL_D(fmt,args...)       {LOGD(DBG_TAG_SWL, fmt, ## args)}
    #define SWL_E(fmt,args...)       {LOGE(DBG_TAG_SWL, fmt, ## args)}
#else
    #define SWL_D(fmt,args...)
    #define SWL_E(fmt,args...)
#endif


#if (SWL_TOTALSIZE != 0u)
static uint32 guiLastIdx = 0u;
static uint32 guiWriteIdx = 0u;
static uint32 guiWriteCnt = 0u;

static uint8  gucScanedHeader[SWL_REGION_NO] = {0};
#endif

typedef enum {
    SWL_ERASED                          = 0u,
    SWL_DATAOCCUPIED                    = 1u,
    SWL_PREVDATA                        = 2u,
    SWL_CORRUPTED                       = 3u,
} SWL_hdType;




/*
***************************************************************************************************
*                                       SWL_SearchLastIdx
*
* Init SAV drv
*   read last saved data and erase unnecessary data, and check index
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
static uint32 SWL_SearchLastIdx
(
    void
);


/*
***************************************************************************************************
*                                       SWL_EraseUnnessary
*
* Init SAV drv
*   read last saved data and erase unnecessary data, and check index
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
static uint32 SWL_EraseUnnessary
(
    void
);
/*
***************************************************************************************************
*                                       SWL_LoadData
*
* Init SAV drv
*   read last saved data and erase unnecessary data, and check index
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
static void SWL_LoadData
(
    void * lastSavedData
);

/*
***************************************************************************************************
*                                       SWL_RelocateData
*
*   if there is no room to write, erase all space and relocate prev data at first position
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

static uint32 SWL_RelocateData
(
    const void * lastSavedData
);


static uint32 SWL_SearchLastIdx
(
    void
)
{
    uint32 uiRet = 0u;
#if (SWL_TOTALSIZE != 0u)
    uint32 uiI;
    uint32 uiSearchedHeaderIdx = 0u;
    uint32 uiSearchedIdx = 0xFFFFFFFFu;
    uint32 uiTargerAddr;
    const uint32 *dumpData;
    uint32 checkSize;
    const SWLTypeHeader *searched;

    for(uiI = 0; uiI < SWL_REGION_NO ; uiI++)
    {
        uiTargerAddr = SWL_READOFFSET + SWL_STARTOFFSET + (uiI * SWL_REGION_SIZE);
        searched = (const SWLTypeHeader *)uiTargerAddr;

        if(searched->Marker == SWL_MARKER )
        {
            gucScanedHeader[uiI] = (uint8)SWL_DATAOCCUPIED;
            if(searched->Index >= uiSearchedHeaderIdx)
            {
                uiSearchedHeaderIdx = searched->Index;
                uiSearchedIdx = uiI;
            }
        }
        else if((searched->Marker == 0xFFFFFFFFu ) && (searched->Index == 0xFFFFFFFFu ))
        {
            gucScanedHeader[uiI] = (uint8)SWL_ERASED;
        }
        else
        {
            gucScanedHeader[uiI] = (uint8)SWL_CORRUPTED;
        }
    }

    if((uiI == (uint32)SWL_REGION_NO) && (uiSearchedIdx == 0xFFFFFFFFu))
    {
        // not founded anything
        //erase all of the area
        uiRet = EFLASH_Erase((SWL_STARTOFFSET&0xFFFFFF00u),SWL_TOTALSIZE,FCONT_TYPE_DFLASH);
        if(uiRet == (uint32)FCONT_RET_OK)
        {
            guiLastIdx = 0xFFFFFFFFu;
            guiWriteIdx = 0u;
            guiWriteCnt = 0u;
            uiRet = (uint32)SWL_NOSAVED_DATA;
        }
        else
        {
            uiRet = (uint32)SWL_WRITE_ERR;
        }
    }
    else
    {
        gucScanedHeader[uiSearchedIdx] = (uint8)SWL_PREVDATA;
        guiLastIdx  = uiSearchedIdx;
        // Check if there is space for the index to be written
        guiWriteIdx = guiLastIdx + 1u;
        guiWriteCnt = uiSearchedHeaderIdx;
        if(guiWriteIdx >= SWL_REGION_NO)
        {
            guiWriteIdx = 0u;
            guiWriteCnt ++;           
        }
       
        //check write index had been erased
        uiSearchedIdx = guiWriteIdx;
        for(uiI = 0u; uiI < SWL_REGION_NO ; uiI++)
        {
            if( gucScanedHeader[uiSearchedIdx] != (uint8)SWL_ERASED)
            {
                uiSearchedIdx = (uiSearchedIdx + 1u) % SWL_REGION_NO;
            }
            else
            {
                for(checkSize = 0u; checkSize < (SWL_REGION_SIZE / 4u); checkSize++)
                {
                    uiTargerAddr = SWL_READOFFSET + SWL_STARTOFFSET + (uiSearchedIdx * SWL_REGION_SIZE) + (checkSize*4u);
                    dumpData = (uint32 *)uiTargerAddr;
                    if(*dumpData != 0xFFFFFFFFu)
                    {
                        gucScanedHeader[uiSearchedIdx] = (uint8)SWL_CORRUPTED;
                        break;
                    }
                }

                if(checkSize == (SWL_REGION_SIZE / 4u))
                {//erase check ok
                    break;
                }
            }
        }

        if(uiSearchedIdx != guiWriteIdx)
        {// not normal case.
            guiWriteIdx = uiSearchedIdx;
            uiRet = (uint32)SWL_CASEIDXGAP;
        }
        else
        {
            if(uiI == (uint32)SWL_REGION_NO)
            {// full case. //err
                uiRet = (uint32)SWL_DATASPACENOTREADY;// error  value temp
            }
            else if(uiI == 0u)
            {
                uiRet = (uint32)SWL_CASENORMAL;
            }
            else
            {
                uiRet = (uint32)SWL_ERRUNEXPECTED;
            }
        }
        
    }
#endif
    return uiRet;
}


static uint32 SWL_EraseUnnessary
(
    void
)
{
    uint32 uiRet            = 0u;
#if (SWL_TOTALSIZE != 0u)

#ifdef NEVER
/* we does not support rollover case */
    uint32 uiI;

    uint32 uiPrevPageNo     = 0u;
    uint32 uiPrevPageEnd     = 0u;
    uint32 uiTargetPageNo   = 0u;
    uint32 uiTargetPageNoEnd = 0u;

    uint32 uiPrevPageNoOver  = 0u;
    uint32 uiPageNoOver      = 0u;
    uint32 uiAbnormalCase    = 0u;
    uint32 uiTargerAddr;

    const SWLTypeHeader *searched;

    if(guiLastIdx < SWL_REGION_NO)
    {
        uiPrevPageNo = (guiLastIdx * SWL_REGION_SIZE)/(SWL_ERASE_PAGE);

        if(uiPrevPageNo != (((guiLastIdx + 1u) * SWL_REGION_SIZE - 1u)/(SWL_ERASE_PAGE)))
        {
            uiPrevPageEnd = (((guiLastIdx + 1u) * SWL_REGION_SIZE - 1u)/(SWL_ERASE_PAGE));
            if(uiPrevPageEnd > SWL_REGION_PAGE_TOTAL)
            {
                uiPrevPageNoOver = 1u;
                uiPrevPageEnd = uiPrevPageEnd % SWL_REGION_PAGE_TOTAL;
            }
        }

        uiTargetPageNo = (guiWriteIdx * SWL_REGION_SIZE)/(SWL_ERASE_PAGE);

        if(uiTargetPageNo != (((guiWriteIdx + 1u) * SWL_REGION_SIZE - 1u)/(SWL_ERASE_PAGE)))
        {
            uiTargetPageNoEnd = (((guiWriteIdx + 1u) * SWL_REGION_SIZE - 1u)/(SWL_ERASE_PAGE));
            if(uiTargetPageNoEnd > SWL_REGION_PAGE_TOTAL)
            {
                uiPageNoOver = 1u;
                uiTargetPageNoEnd = uiTargetPageNoEnd % SWL_REGION_PAGE_TOTAL;
            }
        }
        //check target page valid or not

        if(uiPrevPageNoOver)
        {
            if(uiPageNoOver)
            {
                uiAbnormalCase = 1u;
            }
            else
            {
                if((uiTargetPageNo < uiPrevPageEnd) || (uiTargetPageNoEnd > uiPrevPageNo))
                {
                    //abnormal case
                    uiAbnormalCase = 1u;
                }
            }
        }
        else
        {
            if((uiTargetPageNoEnd > uiPrevPageNo) && (uiTargetPageNo < uiPrevPageEnd))
            {
                //abnormal case
                uiAbnormalCase = 1u;
            }
        }
    }


    //erase
    if(uiAbnormalCase == 1u)
    {
        uiRet = EFLASH_Erase((SWL_STARTOFFSET&0xFFFFFF00u),SWL_TOTALSIZE, FCONT_TYPE_DFLASH);
        if(uiRet == (uint32)FCONT_RET_OK)
        {
            guiLastIdx    = 0xFFFFFFFFu;
            guiWriteIdx   = 0u;
            guiWriteCnt ++;
        }
    }
    else
    {
        for(uiI = 0; uiI < SWL_REGION_PAGE_TOTAL ; uiI++)
        {
            if((uiI > uiPrevPageEnd) || (uiI < uiPrevPageNo))
            {
                uiTargerAddr =  ((SWL_STARTOFFSET + (uiI * SWL_ERASE_PAGE)) & 0xFFFFFF00u);

                uiRet = EFLASH_Erase(uiTargerAddr, SWL_ERASE_PAGE, FCONT_TYPE_DFLASH);
                if(uiRet != (uint32)FCONT_RET_OK)
                {
                    uiRet = (uint32)SWL_WRITE_ERR;
                    break;
                }
            }

        }
    }
#else

    uint32 uiI;

    uint32 uiPrevPageNo     = 0u;
    uint32 uiPrevPageEnd     = 0u;

    uint32 uiTargerAddr;
    uint8 uiEraseValue[256] = {0xFF,};
    int32 iRet = 0;    
        
    SAL_MemSet(uiEraseValue, 0xFFu, 256u);
    if(guiLastIdx < SWL_REGION_NO)
    {
        uiPrevPageNo = (guiLastIdx * SWL_REGION_SIZE)/(SWL_ERASE_PAGE);

        uiPrevPageEnd = ((((guiLastIdx + 1u) * SWL_REGION_SIZE) - 1u)/(SWL_ERASE_PAGE));

        for(uiI = 0; uiI < SWL_REGION_PAGE_TOTAL ; uiI++)
        {
            if((uiI > uiPrevPageEnd) || (uiI < uiPrevPageNo))
            {                
                
                uiTargerAddr =  ((SWL_STARTOFFSET + (uiI * SWL_ERASE_PAGE)) & 0xFFFFFF00u);
                SAL_MemCmp((void*)(uiTargerAddr+ SWL_READOFFSET), (void*)uiEraseValue, 256u, &iRet);
                if(iRet == 0)
                {                    
                    continue;
                }

                uiRet = EFLASH_Erase(uiTargerAddr, SWL_ERASE_PAGE, FCONT_TYPE_DFLASH);
                if(uiRet != (uint32)FCONT_RET_OK)
                {
                    uiRet = (uint32)SWL_WRITE_ERR;
                    break;
                }
                
            }

        }
    }

#endif

#endif
    return uiRet;
}

static void SWL_LoadData
(
    void * lastSavedData
)
{
#if (SWL_TOTALSIZE != 0u)
    uint32 uiTargerAddr;

    if(guiLastIdx >= SWL_REGION_NO) /*  0xFFFFFFFFu)   */
    {
        (void)SAL_MemSet(lastSavedData, 0u, SWL_DATA_SIZE);
    }
    else
    {
        uiTargerAddr = SWL_READOFFSET + SWL_STARTOFFSET + (guiLastIdx * SWL_REGION_SIZE);
        (void)SAL_MemCopy(lastSavedData, (void *)(uiTargerAddr + SWL_DATA_HEADER_SIZE), SWL_DATA_SIZE);
    }
#endif
}


static uint32 SWL_RelocateData
(
    const void * lastSavedData
)
{
    uint32 uiRet          = 0u;
#if (SWL_TOTALSIZE != 0u)
    uint32 uiAddr         = SWL_STARTOFFSET;
    SWLTypeHeader wHeader = {SWL_MARKER, guiWriteCnt};

    uiRet = EFLASH_Erase((uiAddr&0xFFFFFF00u), SWL_TOTALSIZE, FCONT_TYPE_DFLASH);

    if(uiRet == (uint32)FCONT_RET_OK)
    {
        if(SWL_REGION_NO > 1u ) /* for test*/
        {
            uiRet = EFLASH_DFLASH_WriteByte(uiAddr ,&wHeader, SWL_DATA_HEADER_SIZE);
            if(uiRet == (uint32)FCONT_RET_OK)
            {
                uiRet = EFLASH_DFLASH_WriteByte((uiAddr+SWL_DATA_HEADER_SIZE) ,lastSavedData, SWL_DATA_SIZE);
                guiWriteIdx = 1u;
            }
        }
        else
        {
            guiWriteIdx = 0u;
        }
    }
#endif
    return uiRet;


}


/*
***************************************************************************************************
*                                       SWL_Init
*
* Init SAV drv
*   read last saved data and erase unnecessary data, and check index
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/

uint32 SWL_Init
(
    void * lastSavedData
)
{
    uint32 uiRet = 0u;    
    
#if (SWL_TOTALSIZE != 0u)
    
    uiRet = SWL_SearchLastIdx();
    
    if(uiRet == (uint32)SWL_NOSAVED_DATA)
    {
        SWL_LoadData(lastSavedData);
    }
    else if((uiRet == (uint32)SWL_CASENORMAL)||(uiRet == (uint32)SWL_CASEIDXGAP))
    {
        SWL_LoadData(lastSavedData);        
        if(SWL_EraseUnnessary() == (uint32)SWL_WRITE_ERR)
        {
            uiRet = (uint32)SWL_WRITE_ERR;
        }        
        
    }
    else if(uiRet == (uint32)SWL_DATASPACENOTREADY)
    {
        SWL_LoadData(lastSavedData);        
        if(SWL_REGION_NO == 2)
        {
            if(SWL_EraseUnnessary() == (uint32)SWL_WRITE_ERR)
            {
                uiRet = (uint32)SWL_WRITE_ERR;
            }
        }
        else if(SWL_RelocateData(lastSavedData) == (uint32)SWL_WRITE_ERR)
        {
            uiRet = (uint32)SWL_WRITE_ERR;
        }
    }
    else
    {
        ;
    }
#else
    uiRet = (uint32)SWL_ERRUNEXPECTED;
#endif

    return uiRet;
}


/*
***************************************************************************************************
*                                       SWL_Write
* The design concept is one write call at a time boot.
*
*
* @param    none
*
* @return   none
*
* Notes
*
***************************************************************************************************
*/
uint32 SWL_Write
(
    const void * SrcData
)
{
    uint32 uiRet          = 0u;
#if (SWL_TOTALSIZE != 0u)
    SWLTypeHeader wHeader = {SWL_MARKER, guiWriteCnt};
    uint32 uiAddr = SWL_STARTOFFSET + (guiWriteIdx * SWL_REGION_SIZE);
    const SWLTypeHeader *searched;

    searched = (const SWLTypeHeader *)(SWL_READOFFSET + uiAddr);
    if((searched->Marker != 0xFFFFFFFFu ) || (searched->Index != 0xFFFFFFFFu ))
    {
        //exist data.
#ifdef WHEN_OVERWRITE_ERASEALL
        //erase all of the area
        uiRet = EFLASH_Erase((SWL_STARTOFFSET&0xFFFFFF00u),SWL_TOTALSIZE, FCONT_TYPE_DFLASH);
        if(uiRet == FCONT_RET_OK)
        {
            guiLastIdx    = 0xFFFFFFFFu;
            guiWriteIdx   = 0u;
            guiWriteCnt ++;
            uiAddr        = SWL_STARTOFFSET + (guiWriteIdx * SWL_REGION_SIZE);
            wHeader.Index = guiWriteCnt;
            uiRet = EFLASH_DFLASH_WriteByte(uiAddr ,&wHeader, SWL_DATA_HEADER_SIZE);
            if(uiRet == FCONT_RET_OK)
            {
                uiRet = EFLASH_DFLASH_WriteByte((uiAddr+SWL_DATA_HEADER_SIZE) ,SrcData, SWL_DATA_SIZE);
                guiWriteIdx++;
                if(guiWriteIdx == SWL_REGION_NO)
                {
                    guiWriteIdx = 0u;
                    guiWriteCnt ++;
                }
            }

        }

        if(uiRet != FCONT_RET_OK)
        {
            uiRet = SWL_WRITE_ERR;
        }
#else
        uiRet = (uint32)SWL_DATASPACENOTREADY;
#endif



    }
    else if(guiWriteIdx < SWL_REGION_NO)
    {
        uiAddr = SWL_STARTOFFSET + (guiWriteIdx * SWL_REGION_SIZE);
        uiRet  = EFLASH_DFLASH_WriteByte(uiAddr ,&wHeader, SWL_DATA_HEADER_SIZE);

        if(uiRet == (uint32)FCONT_RET_OK)
        {
            uiRet = EFLASH_DFLASH_WriteByte((uiAddr+SWL_DATA_HEADER_SIZE) ,SrcData, SWL_DATA_SIZE);
            guiWriteIdx++;
            if(guiWriteIdx == SWL_REGION_NO)
            {
                guiWriteIdx = 0u;
                if(guiWriteCnt != 0xFFFFFFFFu)
                {
                    guiWriteCnt ++;
                }
            }
        }

        if(uiRet != (uint32)FCONT_RET_OK)
        {
            uiRet = (uint32)SWL_WRITE_ERR;
        }
        else
        {
            if(guiWriteCnt > SWL_FLASH_ENDURANCE)
            {/* for aging test */
                uiRet = (uint32)SWL_ENDURANCE_END;
            }
        }
    }
    else
    {
        uiRet = (uint32)SWL_ERRUNEXPECTED;
    }
#else
    uiRet = (uint32)SWL_ERRUNEXPECTED;
#endif
    return uiRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_SWL == 1 )

