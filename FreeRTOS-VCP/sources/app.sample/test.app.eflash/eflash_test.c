// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : eflash_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

#include <eflash_test.h>
#include <eflash.h>
#include <bsp.h>
#include <gic.h>
#include <fmu.h>
#include <eflash.h>
#include <reg_phys.h>
#include <fmu.h>
#include <debug.h>
#include <snor_mio.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#ifdef __GNU_C__

#define TEST_EF_SETUSER()                    __asm volatile ( "CPS 0x10" )
#define TEST_EF_SETSVC()                     __asm volatile ( "SWI 0" )
#define TEST_EFLASH_CACHE_FLUSH()       \
    asm("DSB");                         \
    asm("MCR p15, 0, r0, c7, c5, 0");   \
    asm("MCR p15, 0, r0, c15, c5, 0");  \
    asm("ISB")
#else
__asm volatile void TEST_EF_SETUSER()
{
    CPS 0x10;
}

__asm volatile void TEST_EF_SETSVC()
{
    SWI 0;
}

__asm volatile void TEST_EFLASH_CACHE_FLUSH()
{
    DSB;
    MCR p15, 0, r0, c7, c5, 0;
    MCR p15, 0, r0, c15, c5, 0;
    ISB;
}
#endif
#define TEST_EF_PRG_LDT0                (0xA1011000u)
#define TEST_EF_CNT_PRG_LDT0            (0xFFFD1000u)

#define PFLASH_PW_ADDRESS               (0xA1008060u)
#define TEST_EFLASH_PASSWORD            (0x2939F437u)


#define PFLASH_INTEn_ADDRESS            (0xA1020000u)
#define PFLASH_INTEn_ERASE_FAIL         ((uint32) 1u << 0u)
#define PFLASH_INTEn_READ_FAIL          ((uint32) 1u << 1u)
#define PFLASH_INTEn_PROGADDR_FAIL      ((uint32) 1u << 2u)
#define PFLASH_INTEn_PROGDATA_FAIL      ((uint32) 1u << 3u)
#define PFLASH_INTEn_ECC_FAIL           ((uint32) 1u << 4u)
#define PFLASH_INTEn_CMDREADY           ((uint32) 1u << 5u)
#define PFLASH_INTEn_PGM_FIN            ((uint32) 1u << 6u)
#define PFLASH_INTEn_RESET_FIN          ((uint32) 1u << 7u)
#define PFLASH_INTEn_GLOBAL_INT         ((uint32) 1u << 8u)
#define PFLASH_INTEn_WRITE_PROT         ((uint32) 1u << 12u)

#define PFLASH_INTSTAT_ADDRESS          (0xA1020004u)

#define PFLASH_FAULTEn_ADDRESS          (0xA1020008u)
#define PFLASH_FAULTEn_ERASE_FAIL       (   1u << 0u)
#define PFLASH_FAULTEn_READ_FAIL        (   1u << 1u)
#define PFLASH_FAULTEn_PROGADDR_FAIL    (   1u << 2u)
#define PFLASH_FAULTEn_PROGDATA_FAIL    (   1u << 3u)
#define PFLASH_FAULTEn_ECC_FAIL         (   1u << 4u)

#define PFLASH_FAULTSTAT_ADDRESS        (0xA102000Cu)



#define DFLASH_INTEn_ADDRESS                   (0xA10A0000u)
#define DFLASH_INTEn_ECC_FAIL                  ((uint32)1u << 0u)
#define DFLASH_INTEn_CMDREADY                  ((uint32)1u << 1u)
#define DFLASH_INTEn_PGM_FIN                   ((uint32)1u << 2u)
#define DFLASH_INTEn_RESET_FIN                 ((uint32)1u << 3u)
#define DFLASH_Privilege_access_FAIL           ((uint32)1u << 4u)
#define DFLASH_Paddr_range_permission_error    ((uint32)1u << 5u)
#define DFLASH_HSM_SFR_access_permission_error ((uint32)1u << 6u)
#define DFLASH_CR5_SFR_access_permission_error ((uint32)1u << 7u)
#define DFLASH_Sema_timeout_release_error      ((uint32)1u << 8u)

#define DFLASH_INTEn_GLOBAL_INT                ((uint32)   1u << 9u)
#define DFLASH_INTEn_WRITE_PROT                ((uint32)   1u << 12u)

#define DFLASH_INTSTAT_ADDRESS                 (0xA10A0004u)

#define DFLASH_FAULTEn_ADDRESS                         (0xA10A0008u)
#define DFLASH_FAULTEn_ECC_FAIL                        ((uint32) 1u << 0u)
#define DFLASH_FAULTEn_Privilege_access_FAIL           ((uint32) 1u << 1u)
#define DFLASH_FAULTEn_Paddr_range_permission_error    ((uint32) 1u << 2u)
#define DFLASH_FAULTEn_HSM_SFR_access_permission_error ((uint32) 1u << 3u)
#define DFLASH_FAULTEn_CR5_SFR_access_permission_error ((uint32) 1u << 4u)
#define DFLASH_FAULTEn_Sema_timeout_release_error      ((uint32) 1u << 5u)





#define DFLASH_FAULTSTAT_ADDRESS        (0xA10A000Cu)

/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/



static uint32                           guiEflashWaitIrq = 0UL;
static uint32                           guiFmuIsrFlag =  1u;
static uint32                           guiIsrFlag = 0u;

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static inline uint32 FLASHTEST_READREG(uint32 uiAddr)
{
    uint32 uiRet = 0u;
    uiRet = SAL_ReadReg(uiAddr);
    return uiRet;
}

static inline void FLASHTEST_WRITEREG(uint32 uiValue, uint32 uiAddr)
{
    SAL_WriteReg(uiValue, uiAddr);
    return;
}

static void TEST_EFLASH_ISR
(
    void *                              pArg
);





static void TEST_EFLASH_ISR(    void *pArg)
{

    const uint32* puiFmuCheckArg;
    uint32  uiRegtemp = 0u;

    if(pArg != NULL_PTR)
    {
        puiFmuCheckArg = (uint32 *)pArg;

        if(*puiFmuCheckArg == 1u)
        {
            mcu_printf("FMU IRQ\n");
            uiRegtemp = FLASHTEST_READREG(PFLASH_FAULTSTAT_ADDRESS);
            if((uiRegtemp & PFLASH_FAULTEn_ERASE_FAIL) == PFLASH_FAULTEn_ERASE_FAIL)
            {
                mcu_printf("PFLASH_FAULTEn_ERASE_FAIL \n");
            }
            if((uiRegtemp & PFLASH_FAULTEn_READ_FAIL) == PFLASH_FAULTEn_READ_FAIL)
            {
                mcu_printf("PFLASH_FAULTEn_READ_FAIL \n");
            }
            if((uiRegtemp & PFLASH_FAULTEn_PROGADDR_FAIL) == PFLASH_FAULTEn_PROGADDR_FAIL)
            {
                mcu_printf("PFLASH_FAULTEn_PROGADDR_FAIL \n");
            }
            if((uiRegtemp & PFLASH_FAULTEn_PROGDATA_FAIL) == PFLASH_FAULTEn_PROGDATA_FAIL)
            {
                mcu_printf("PFLASH_FAULTEn_PROGDATA_FAIL \n");
            }
            if((uiRegtemp & PFLASH_FAULTEn_ECC_FAIL) == PFLASH_FAULTEn_ECC_FAIL)
            {
                mcu_printf("PFLASH_FAULTEn_ECC_FAIL \n");
            }
            FLASHTEST_WRITEREG(uiRegtemp, PFLASH_FAULTSTAT_ADDRESS);
        }
        else
        {
            mcu_printf("GIC IRQ\n");
            uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
            if((uiRegtemp & PFLASH_INTEn_ERASE_FAIL) == PFLASH_INTEn_ERASE_FAIL)
            {
                mcu_printf("PFLASH_INTEn_ERASE_FAIL \n");
            }
            if((uiRegtemp & PFLASH_INTEn_READ_FAIL) == PFLASH_INTEn_READ_FAIL)
            {
                mcu_printf("PFLASH_INTEn_READ_FAIL \n");
            }
            if((uiRegtemp & PFLASH_INTEn_PROGADDR_FAIL) == PFLASH_INTEn_PROGADDR_FAIL)
            {
                mcu_printf("PFLASH_INTEn_PROGADDR_FAIL \n");
            }
            if((uiRegtemp & PFLASH_INTEn_PROGDATA_FAIL) == PFLASH_INTEn_PROGDATA_FAIL)
            {
                mcu_printf("PFLASH_INTEn_PROGDATA_FAIL \n");
            }
            if((uiRegtemp & PFLASH_INTEn_ECC_FAIL) == PFLASH_INTEn_ECC_FAIL)
            {
                mcu_printf("PFLASH_INTEn_ECC_FAIL \n");
            }
            if((uiRegtemp & PFLASH_INTEn_CMDREADY) == PFLASH_INTEn_CMDREADY)
            {
                mcu_printf("PFLASH_INTEn_CMDREADY \n");
            }
            if((uiRegtemp & PFLASH_INTEn_PGM_FIN) == PFLASH_INTEn_PGM_FIN)
            {
                mcu_printf("PFLASH_INTEn_PGM_FIN \n");
            }
            if((uiRegtemp & PFLASH_INTEn_RESET_FIN) == PFLASH_INTEn_RESET_FIN)
            {
                mcu_printf("PFLASH_INTEn_RESET_FIN \n");
            }
            FLASHTEST_WRITEREG(uiRegtemp, PFLASH_INTSTAT_ADDRESS);
        }






#if 1//def NEVER
        if(*puiFmuCheckArg == 1u)
        {
            (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_PFLASH);
            //(void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH, (FMUSeverityLevelType_t)FMU_SVL_LOW, NULL_PTR, NULL_PTR);
        }
        else
        {
#ifdef NEVER
            (void)GIC_IntVectSet(GIC_PFLASH,
                                 GIC_PRIORITY_NO_MEAN,
                                 GIC_INT_TYPE_LEVEL_HIGH,
                                 NULL_PTR,
                                 NULL_PTR);


            (void)GIC_IntSrcDis(GIC_PFLASH);
#endif /* NEVER */
        }
#endif /* NEVER */
        mcu_printf("please reset for another test\n");

   }


    return;
}



static void TEST_EFLASH_ISR2(    void *pArg)
{
    volatile uint32  uiRegtemp = 0u;
    (void)pArg;

    uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
    if((uiRegtemp & PFLASH_INTEn_ERASE_FAIL) == PFLASH_INTEn_ERASE_FAIL)
    {
        mcu_printf("PFLASH_INTEn_ERASE_FAIL \n");
    }
    if((uiRegtemp & PFLASH_INTEn_READ_FAIL) == PFLASH_INTEn_READ_FAIL)
    {
        mcu_printf("PFLASH_INTEn_READ_FAIL \n");
    }
    if((uiRegtemp & PFLASH_INTEn_PROGADDR_FAIL) == PFLASH_INTEn_PROGADDR_FAIL)
    {
        mcu_printf("PFLASH_INTEn_PROGADDR_FAIL \n");
    }
    if((uiRegtemp & PFLASH_INTEn_PROGDATA_FAIL) == PFLASH_INTEn_PROGDATA_FAIL)
    {
        mcu_printf("PFLASH_INTEn_PROGDATA_FAIL \n");
    }
    if((uiRegtemp & PFLASH_INTEn_ECC_FAIL) == PFLASH_INTEn_ECC_FAIL)
    {
        mcu_printf("PFLASH_INTEn_ECC_FAIL \n");
    }
    if((uiRegtemp & PFLASH_INTEn_CMDREADY) == PFLASH_INTEn_CMDREADY)
    {
        mcu_printf("PFLASH_INTEn_CMDREADY \n");
    }
    if((uiRegtemp & PFLASH_INTEn_PGM_FIN) == PFLASH_INTEn_PGM_FIN)
    {
        mcu_printf("PFLASH_INTEn_PGM_FIN \n");
    }
    if((uiRegtemp & PFLASH_INTEn_RESET_FIN) == PFLASH_INTEn_RESET_FIN)
    {
        mcu_printf("PFLASH_INTEn_RESET_FIN \n");
    }

    FLASHTEST_WRITEREG(uiRegtemp, PFLASH_INTSTAT_ADDRESS);  /* clear int status */
    guiEflashWaitIrq = 0u;



    return;
}


static void TEST_EFLASH_ISR3(    void *pArg)
{
    volatile uint32  uiRegtemp = 0u;
    (void)pArg;

    uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
    if((uiRegtemp & DFLASH_INTEn_ECC_FAIL) == DFLASH_INTEn_ECC_FAIL)
    {
        mcu_printf("DFLASH_INTEn_ECC_FAIL \n");
    }
    if((uiRegtemp & DFLASH_INTEn_CMDREADY) == DFLASH_INTEn_CMDREADY)
    {
        mcu_printf("DFLASH_INTEn_CMDREADY \n");
    }
    if((uiRegtemp & DFLASH_INTEn_PGM_FIN) == DFLASH_INTEn_PGM_FIN)
    {
        mcu_printf("DFLASH_INTEn_PGM_FIN \n");
    }
    if((uiRegtemp & DFLASH_INTEn_RESET_FIN) == DFLASH_INTEn_RESET_FIN)
    {
        mcu_printf("DFLASH_INTEn_RESET_FIN \n");
    }
    if((uiRegtemp & DFLASH_Privilege_access_FAIL) == DFLASH_Privilege_access_FAIL)
    {
        mcu_printf("DFLASH_Privilege_access_FAIL \n");
    }

    FLASHTEST_WRITEREG(uiRegtemp, DFLASH_INTSTAT_ADDRESS);  /* clear int status */
    guiEflashWaitIrq = 0u;



    return;
}

static void TEST_EFLASH_ISR4(    void *pArg)
{
    const uint32* puiFmuCheckArg;
    uint32  uiRegtemp = 0u;

    if(pArg != NULL_PTR)
    {
       puiFmuCheckArg = (const uint32 *)pArg;

        if(*puiFmuCheckArg == 1u)
        {
            mcu_printf("FMU IRQ\n");
            uiRegtemp = FLASHTEST_READREG(DFLASH_FAULTSTAT_ADDRESS);

            if((uiRegtemp & DFLASH_FAULTEn_ECC_FAIL) == DFLASH_FAULTEn_ECC_FAIL)
            {
                mcu_printf("DFLASH_FAULTEn_ECC_FAIL check ok\n");
            }
            if((uiRegtemp & DFLASH_FAULTEn_Privilege_access_FAIL) == DFLASH_FAULTEn_Privilege_access_FAIL)
            {
                mcu_printf("DFLASH_FAULTEn_Privilege_access_FAIL check ok\n");
            }
            if((uiRegtemp & DFLASH_FAULTEn_Paddr_range_permission_error) == DFLASH_FAULTEn_Paddr_range_permission_error)
            {
                mcu_printf("DFLASH_FAULTEn_Paddr_range_permission_error check ok\n");
            }
            if((uiRegtemp & DFLASH_FAULTEn_HSM_SFR_access_permission_error) == DFLASH_FAULTEn_HSM_SFR_access_permission_error)
            {
                mcu_printf("DFLASH_FAULTEn_HSM_SFR_access_permission_error check ok\n");
            }
            if((uiRegtemp & DFLASH_FAULTEn_CR5_SFR_access_permission_error) == DFLASH_FAULTEn_CR5_SFR_access_permission_error)
            {
                mcu_printf("DFLASH_FAULTEn_CR5_SFR_access_permission_error check ok\n");
            }
            if((uiRegtemp & DFLASH_FAULTEn_Sema_timeout_release_error) == DFLASH_FAULTEn_Sema_timeout_release_error)
            {
                mcu_printf("DFLASH_FAULTEn_Sema_timeout_release_error check ok\n");
            }
            FLASHTEST_WRITEREG(uiRegtemp, DFLASH_FAULTSTAT_ADDRESS);  /* clear int status */
        }
        else
        {
            mcu_printf("GIC IRQ\n");
            uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);

            if((uiRegtemp & DFLASH_INTEn_ECC_FAIL) == DFLASH_INTEn_ECC_FAIL)
            {
                mcu_printf("DFLASH_INTEn_ECC_FAIL check ok\n");
            }
            if((uiRegtemp & DFLASH_INTEn_CMDREADY) == DFLASH_INTEn_CMDREADY)
            {
                mcu_printf("DFLASH_INTEn_CMDREADY check ok\n");
            }
            if((uiRegtemp & DFLASH_INTEn_PGM_FIN) == DFLASH_INTEn_PGM_FIN)
            {
                mcu_printf("DFLASH_INTEn_PGM_FIN check ok\n");
            }
            if((uiRegtemp & DFLASH_INTEn_RESET_FIN) == DFLASH_INTEn_RESET_FIN)
            {
                mcu_printf("DFLASH_INTEn_RESET_FIN check ok\n");
            }
            if((uiRegtemp & DFLASH_Privilege_access_FAIL) == DFLASH_Privilege_access_FAIL)
            {
                mcu_printf("DFLASH_Privilege_access_FAIL check ok\n");
            }
            if((uiRegtemp & DFLASH_Paddr_range_permission_error) == DFLASH_Paddr_range_permission_error)
            {
                mcu_printf("DFLASH_Paddr_range_permission_error check ok\n");
            }

            if((uiRegtemp & DFLASH_HSM_SFR_access_permission_error) == DFLASH_HSM_SFR_access_permission_error)
            {
                mcu_printf("DFLASH_HSM_SFR_access_permission_error check ok\n");
            }
            if((uiRegtemp & DFLASH_CR5_SFR_access_permission_error) == DFLASH_CR5_SFR_access_permission_error)
            {
                mcu_printf("DFLASH_CR5_SFR_access_permission_error check ok\n");
            }
            if((uiRegtemp & DFLASH_Sema_timeout_release_error) == DFLASH_Sema_timeout_release_error)
            {
                mcu_printf("DFLASH_Sema_timeout_release_error check ok\n");
            }
            FLASHTEST_WRITEREG(uiRegtemp, DFLASH_INTSTAT_ADDRESS);  /* clear int status */
        }





#if 1//def NEVER
        if(*puiFmuCheckArg == 1u)
        {
//          (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH, (FMUSeverityLevelType_t)FMU_SVL_LOW, NULL_PTR, NULL_PTR);
            (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_DFLASH);
        }
        else
        {
            (void)GIC_IntVectSet(GIC_DFLASH,
                                 GIC_PRIORITY_NO_MEAN,
                                 GIC_INT_TYPE_LEVEL_HIGH,
                                 NULL_PTR,
                                 NULL_PTR);


            (void)GIC_IntSrcDis(GIC_DFLASH);
        }
#endif /* NEVER */

        mcu_printf("please reset for another test\n");



   }


    return;
}

#ifdef NEVER

uint32 EFLASH_ECC_Test(uint32 uiFAULTEn)
{
    uint32 ret = FCONT_RET_OK;
    volatile uint32 reg_temp = 0;
    uint32 uitempbuff[4] = {0x12345678u, 0xFFFF1234u, 0x1234FFFFu, 0xEEEEEEEEu};
    // 1. eflash 0 address
    EFLASH_EraseMode(0u, FLS_FRWCON_PAGE_ERASE, FCONT_TYPE_PFLASH);

    // 2. set ecc off
    //it is not neccessary  FLASHTEST_WRITEREG(TEST_EFLASH_PASSWORD, PFLASH_PW_ADDRESS);
    reg_temp = HwPflash_RegisterMap->TM_FSH_CON;
    reg_temp = reg_temp & 0x0FFFFFFFu;      //bit 28 ecc off
    HwPflash_RegisterMap->TM_FSH_CON = reg_temp;

    // 3. write 0x0~ 0xc
    EFLASH_WriteByte(0u, (void *)uitempbuff, 16u, FCONT_TYPE_PFLASH);

    // 4. ecc on
    reg_temp = HwPflash_RegisterMap->TM_FSH_CON;
    reg_temp = reg_temp | 0x10000000u;      //bit 28 ecc on
    HwPflash_RegisterMap->TM_FSH_CON = reg_temp;

    if(uiFAULTEn == 1)
    {
        reg_temp = FLASHTEST_READREG(PFLASH_FAULTEn_ADDRESS);
        reg_temp |= PFLASH_FAULTEn_ECC_FAIL;
        FLASHTEST_WRITEREG(reg_temp, PFLASH_FAULTEn_ADDRESS);
    }

    reg_temp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);

    if(uiFAULTEn == 2)
    {
        reg_temp |= (PFLASH_INTEn_ECC_FAIL);
    }
    else
    {
        reg_temp |= (PFLASH_INTEn_ECC_FAIL | PFLASH_INTEn_GLOBAL_INT);
    }
    FLASHTEST_WRITEREG(reg_temp, PFLASH_INTEn_ADDRESS);


    // 5. read data 0x0~0xc
    EFLASH_ReadByte(0, (void *)uitempbuff, 16u, FCONT_TYPE_PFLASH);


    return ret;
}
#endif /* NEVER */

static uint32 EFLASH_ECC_Test(uint32 uiFAULTEn)
{
    uint32 ret = (uint32)FCONT_RET_OK;
    volatile uint32 reg_temp = 0u;

    // 1. eflash erase 0 address

    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = (reg_temp & 0x40u);
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0x40u);
    }

    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = (reg_temp & 0xF00u);
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0xF00u);
    }
    FLASHTEST_WRITEREG(0x90000u, 0xA1000200u);
    FLASHTEST_WRITEREG(1u, 0xA1000000u);
    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = (reg_temp & 0xf00u);
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0xf00u);
    }
    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = (reg_temp & 0x30u);
    while(reg_temp != 0x30u)
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0x30u);
    }




    // 2. set ecc off
    //it is not neccessary  FLASHTEST_WRITEREG(TEST_EFLASH_PASSWORD, PFLASH_PW_ADDRESS);
    reg_temp = FLASHTEST_READREG(0xA1000034u);
    reg_temp = reg_temp & 0x0FFFFFFFu;      //bit 28 ecc off
    FLASHTEST_WRITEREG(reg_temp, 0xA1000034u);

    // 3. write 0x0~ 0xc
    FLASHTEST_WRITEREG(0x90000u, 0xA1000200u);
    FLASHTEST_WRITEREG(0xFFFFFFEEu, 0xA1000204u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000210u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000214u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);

    FLASHTEST_WRITEREG(0x90010u, 0xA1000200u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000204u);
    FLASHTEST_WRITEREG(0xFFFFFFFEu, 0xA1000210u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000214u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);

    FLASHTEST_WRITEREG(0x90020u, 0xA1000200u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000204u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000210u);
    FLASHTEST_WRITEREG(0xFFFFFFFEu, 0xA1000214u);
    FLASHTEST_WRITEREG(0xFFFFFFFFu, 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);

    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = reg_temp & 0x30u;
    while(reg_temp != 0x30u)
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0x30u);
    }

    TEST_EFLASH_CACHE_FLUSH();

    // 4. ecc on
    reg_temp = FLASHTEST_READREG(0xA1000034u);
    reg_temp = reg_temp | 0x10000000u;      //bit 28 ecc on
    FLASHTEST_WRITEREG(reg_temp, 0xA1000034u);

    if(uiFAULTEn == 1u)
    {
        reg_temp = FLASHTEST_READREG(PFLASH_FAULTEn_ADDRESS);
        reg_temp |= PFLASH_FAULTEn_ECC_FAIL;
        FLASHTEST_WRITEREG(reg_temp, PFLASH_FAULTEn_ADDRESS);
    }

    reg_temp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);

    if(uiFAULTEn == 2u)
    {
        reg_temp |= (uint32)(PFLASH_INTEn_ECC_FAIL);
    }
    else
    {
        reg_temp |= ((uint32)PFLASH_INTEn_ECC_FAIL | (uint32)PFLASH_INTEn_GLOBAL_INT);
    }
    FLASHTEST_WRITEREG(reg_temp, PFLASH_INTEn_ADDRESS);


    // 5. read data 0x0~0xc
    mcu_printf("read reg x%x\n",FLASHTEST_READREG(0x20090000u));
    mcu_printf("read reg x%x\n",FLASHTEST_READREG(0x20090010u));
    mcu_printf("read reg x%x\n",FLASHTEST_READREG(0x20090020u));



    return ret;
}

/*
***************************************************************************************************
*                                       EFLASHTest_ECC
*
* EFLASH ECC test
*
* @param    none
*
* @return   none
*
* Notes
*           [SM-PV] 05. Data eFLASH and Controller - DataFlash.01, ECC (Error Correcting Code)
*           Flash memory must be able to detect 1-bit or 2-bit data errors occurring in flash
*           memory through ECC.
***************************************************************************************************
*/


void EFLASHTest_ECC(uint32 uiFlag)
{


    uint32 uiRegtemp = 0u;

    if(uiFlag == 0u)
    {

        mcu_printf("ECC int mode test\n");

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);


        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASH_ECC_Test(0u);
    }
    else if(uiFlag == 1u)
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_HIGH,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);

        mcu_printf("ECC int mode test\n");

        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASH_ECC_Test(1u);
    }
    else if(uiFlag == 2u)
    {

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);

        mcu_printf("ECC int mode test\n");

        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASH_ECC_Test(2u);
    }
    else
    {
        mcu_printf(" int not set, ECC test\n");
        uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASH_ECC_Test(0u);
        uiRegtemp = FLASHTEST_READREG(PFLASH_INTSTAT_ADDRESS);
        if((uiRegtemp & PFLASH_INTEn_ECC_FAIL) == PFLASH_INTEn_ECC_FAIL)
        {
            mcu_printf("PFLASH_INTEn_ECC_FAIL check ok\n");
        }
    }

    mcu_printf("=========== Done ===========\n");
    return;
}

/*
***************************************************************************************************
*                                       EFLASHTest_RESET
*
* EFLASH RESET test
*
* @param    none
*
* @return   uiFlag[in] - 1 - reset occur
*                        2 - after reset, check data recording is completed
*
* Notes
*           [SM-PV] 05. Data eFLASH and Controller - DataFlash.02, Reset
*           If a system reset occurs during programming in Flash, the reset is prevented
*           until the recording is completed to ensure the integrity of the recorded data.
***************************************************************************************************
*/


void EFLASHTest_RESET(uint32 uiFlag)
{
    const uint32 testdata = 0x12341234u;
    const uint32 pmupw    = 0x5AFEACE5u;
    uint32 uiTemp[4] = {0};

    if(uiFlag == 0u)
    {
        mcu_printf("=========== erase addr 0x2000 0000 ~ 0c  ===========\n");
        FLASHTEST_WRITEREG(0u, 0xA1000200u);
        FLASHTEST_WRITEREG(1u, 0xA1000000u);
        mcu_printf("=========== write addr 0x2000 0000 ~ 0c  & cold reset===========\n");

        FLASHTEST_WRITEREG(0u, 0xA1000200u);
        FLASHTEST_WRITEREG(testdata, 0xA1000204u);
        FLASHTEST_WRITEREG(testdata, 0xA1000210u);
        FLASHTEST_WRITEREG(testdata, 0xA1000214u);
        FLASHTEST_WRITEREG(testdata, 0xA1000218u);
        FLASHTEST_WRITEREG(2u, 0xA1000000u);

        FLASHTEST_WRITEREG(pmupw, 0xA0F283FCu);
        FLASHTEST_WRITEREG(1u, 0xA0F28010u);
    }
    else
    {
        uiTemp[0] = FLASHTEST_READREG(0x20000000u);
        uiTemp[1] = FLASHTEST_READREG(0x20000004u);
        uiTemp[2] = FLASHTEST_READREG(0x20000008u);
        uiTemp[3] = FLASHTEST_READREG(0x2000000cu);
        if((uiTemp[0] == testdata) &&
           (uiTemp[1] == testdata) &&
           (uiTemp[2] == testdata) &&
           (uiTemp[3] == testdata))
           {
               mcu_printf("=========== write ok ===========\n");
           }
      //read and check
    }


    return;
}



/*
***************************************************************************************************
*                                       EFLASHTest_Int_Test
*
* EFLASH RESET test
*
* @param    none
*
* @return   none
*
* Notes
*           [SM-PV] 05. Data eFLASH and Controller - DataFlash.02, Reset
*           If a system reset occurs during programming in Flash, the reset is prevented
*           until the recording is completed to ensure the integrity of the recorded data.
***************************************************************************************************
*/


uint32 EFLASHTest_Int_Test(void)
{
    uint32 ret = (uint32)FCONT_RET_OK;
    volatile uint32 reg_temp = 0u;
    const uint32 uitempbuff[4] = {0x12345678u, 0xFFFF1234u, 0x1234FFFFu, 0xEEEEEEEEu};


    FLASHTEST_WRITEREG(0x11FFu, PFLASH_INTEn_ADDRESS); //all int enable

    (void)GIC_IntVectSet(GIC_PFLASH,
                         GIC_PRIORITY_NO_MEAN,
                         GIC_INT_TYPE_LEVEL_HIGH,
                         (GICIsrFunc)&TEST_EFLASH_ISR2,
                         NULL_PTR);


    (void)GIC_IntSrcEn(GIC_PFLASH);

    mcu_printf("Starts erasing the ldt0 area... expected isr is PFLASH_INTEn_ERASE_FAIL\n");
    guiEflashWaitIrq = 1u;
    FLASHTEST_WRITEREG(TEST_EF_CNT_PRG_LDT0, 0xA1000200u);
    FLASHTEST_WRITEREG(1u, 0xA1000000u);

    while(guiEflashWaitIrq == 1u)
    {

    }
    mcu_printf("Starts reading the ldt0 area... expected isr is PFLASH_INTEn_READ_FAIL\n");

    guiEflashWaitIrq = 1u;

    reg_temp = FLASHTEST_READREG(TEST_EF_PRG_LDT0);

    while(guiEflashWaitIrq == 1u)
    {

    }

    mcu_printf("TEST_EF_PRG_LDT0 . readed data x5x\n",reg_temp);

    mcu_printf("Starts writing the ldt0 area... expected isr is PFLASH_INTEn_PROGADDR_FAIL\n");
    guiEflashWaitIrq = 1u;

    FLASHTEST_WRITEREG(TEST_EF_CNT_PRG_LDT0, 0xA1000200u);
    FLASHTEST_WRITEREG(uitempbuff[0], 0xA1000204u);
    FLASHTEST_WRITEREG(uitempbuff[1], 0xA1000210u);
    FLASHTEST_WRITEREG(uitempbuff[2], 0xA1000214u);
    FLASHTEST_WRITEREG(uitempbuff[3], 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);

    while(guiEflashWaitIrq == 1u)
    {

    }

    mcu_printf("Starts erasing address 0... expected isr is PFLASH_INTEn_CMDREADY, PFLASH_INTEn_PGM_FIN\n");
    guiEflashWaitIrq = 1u;

    FLASHTEST_WRITEREG(0u, 0xA1000200u);
    FLASHTEST_WRITEREG(1u, 0xA1000000u);
    while(guiEflashWaitIrq == 1u)
    {

    }

    mcu_printf("Starts writing address 0... expected isr is PFLASH_INTEn_CMDREADY, PFLASH_INTEn_PGM_FIN\n");

    guiEflashWaitIrq = 1u;
    FLASHTEST_WRITEREG(0u, 0xA1000200u);
    FLASHTEST_WRITEREG(uitempbuff[0], 0xA1000204u);
    FLASHTEST_WRITEREG(uitempbuff[1], 0xA1000210u);
    FLASHTEST_WRITEREG(uitempbuff[2], 0xA1000214u);
    FLASHTEST_WRITEREG(uitempbuff[3], 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);
    while(guiEflashWaitIrq == 1u)
    {

    }


    mcu_printf("Starts writing invalid area(not erased)... expected isr is PFLASH_INTEn_PROGDATA_FAIL\n");

    guiEflashWaitIrq = 1u;
    FLASHTEST_WRITEREG(0u, 0xA1000200u);
    FLASHTEST_WRITEREG(uitempbuff[0], 0xA1000204u);
    FLASHTEST_WRITEREG(uitempbuff[1], 0xA1000210u);
    FLASHTEST_WRITEREG(uitempbuff[2], 0xA1000214u);
    FLASHTEST_WRITEREG(uitempbuff[3], 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);
    while(guiEflashWaitIrq == 1u)
    {

    }


    mcu_printf("Starts reset eflash controller... expected isr is PFLASH_INTEn_RESET_FIN\n");
    guiEflashWaitIrq = 1u;

    FLASHTEST_WRITEREG(0x100u, 0xA1000000u);
    while(guiEflashWaitIrq == 1u)
    {

    }
    mcu_printf("---eflash int test complete......\n");
    return ret;
}


static uint32 EFLASHTest_AccCnt(uint32 uiFAULTEn)
{
    uint32 ret = (uint32)FCONT_RET_OK;
    volatile uint32 reg_temp = 0u;
    uint32 uiTempA = 0u;
    uint32 uiTempB = 0u;
  //  uint32 ret_cnt = 3u;

    if(uiFAULTEn == 1u)
    {
        reg_temp = FLASHTEST_READREG(PFLASH_FAULTEn_ADDRESS);
        reg_temp |= (PFLASH_FAULTEn_READ_FAIL | PFLASH_FAULTEn_PROGADDR_FAIL);
        FLASHTEST_WRITEREG(reg_temp, PFLASH_FAULTEn_ADDRESS);
    }

    reg_temp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);



    reg_temp |= 0x1FFu; //all enable
    FLASHTEST_WRITEREG(reg_temp, PFLASH_INTEn_ADDRESS);

    TEST_EF_SETUSER();

    // 5. read data 0x0~0xc
    if(uiFAULTEn == 2u)
    {
        uiTempA = FLASHTEST_READREG(0x20000000u);
        uiTempB = FLASHTEST_READREG(0xA1000004u);
        mcu_printf("attempt read 0x20000000 : x%x, FLASHTEST_READREG(0xA1000004u) status : x%x\n", uiTempA, uiTempB);
    }

    FLASHTEST_WRITEREG(0u, 0xA1000200u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1000204u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1000210u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1000214u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1000218u);
    FLASHTEST_WRITEREG(2u, 0xA1000000u);
    reg_temp = FLASHTEST_READREG(0xA1000004u);
    reg_temp = (reg_temp & 0x30u);
    while((reg_temp != 0x30u))// && (ret_cnt > 0u))
    {
        reg_temp = FLASHTEST_READREG(0xA1000004u);
        reg_temp = (reg_temp & 0x30u);
//      ret_cnt--;
    }

    TEST_EFLASH_CACHE_FLUSH();

    return ret;
}


void EFLASHTest_PflashAccessControl(uint32 uiFlag)
{

    uint32 uiRegtemp = 0u;

    if(uiFlag == 0u)
    {

        mcu_printf("ACC cnt int mode test\n");

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);


        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }


        (void)EFLASHTest_AccCnt(0u);

    }
    else if(uiFlag == 1u)
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);

        mcu_printf("ACC cnt  int mode test\n");

        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_AccCnt(1u);
    }
    else if(uiFlag == 2u)
    {

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_PFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_PFLASH);

        mcu_printf("ACC cnt  int mode test\n");

        (void)GIC_IntVectSet(GIC_PFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_PFLASH);




        uiRegtemp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_AccCnt(2u);
    }
    else
    {
        mcu_printf(" ACC cnt int not set, ECC test\n");
        uiRegtemp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_AccCnt(0u);
        uiRegtemp = FLASHTEST_READREG(PFLASH_INTEn_ADDRESS);
        if((uiRegtemp & DFLASH_INTEn_ECC_FAIL) == DFLASH_INTEn_ECC_FAIL)
        {
            mcu_printf("DFLASH_INTEn_ECC_FAIL check ok\n");
        }
    }

    mcu_printf("=========== Done ===========\n");
    return;
}

/////////////////////////////////////////////////////DFLASH
uint32 EFLASHTest_Int_DFLASHTest(void)
{
    uint32 ret = (uint32)FCONT_RET_OK;

    const uint32 uitempbuff[4] = {0x12345678u, 0xFFFF1234u, 0x1234FFFFu, 0xEEEEEEEEu};

    uint32 LockNSel = FLASHTEST_READREG(0xA10A0014U);//FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS);
    uint32 targetAddr = 0u;
    uint8 Lock = (uint8)((LockNSel & 0x40u) >> 6u);
    uint8 Sel  = (uint8)((LockNSel & 0x30u) >> 4u);
//    uint32 intFlag = (DFLASH_INTEn_GLOBAL_INT | DFLASH_INTEn_PGM_FIN)
    if(Lock == 1u)
    {
        targetAddr += (Sel * 0x10000u);
    }
    else
    {
        mcu_printf("dflash belonging to r5 does not exist\n");
    }


    FLASHTEST_WRITEREG(0x3FFu, DFLASH_INTEn_ADDRESS); //all int enable


    (void)GIC_IntVectSet(GIC_DFLASH,
                         GIC_PRIORITY_NO_MEAN,
                         GIC_INT_TYPE_LEVEL_HIGH,
                         (GICIsrFunc)&TEST_EFLASH_ISR3,
                         NULL_PTR);


    (void)GIC_IntSrcEn(GIC_DFLASH);




    mcu_printf("Starts erasing address 0... expected isr is DFLASH_INTEn_CMDREADY, DFLASH_INTEn_PGM_FIN\n");
    guiEflashWaitIrq = 1u;
    //0. hw sema set

    FLASHTEST_WRITEREG(1u, 0xA10A001CU);

    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(1u, 0xA1080000u);
    while(guiEflashWaitIrq == 1u)
    {

    }

    mcu_printf("Starts writing address 0... expected isr is DFLASH_INTEn_CMDREADY, DFLASH_INTEn_PGM_FIN\n");

    guiEflashWaitIrq = 1u;
    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(uitempbuff[0], 0xA1080204u);
    FLASHTEST_WRITEREG(uitempbuff[1], 0xA1080210u);
    FLASHTEST_WRITEREG(2u, 0xA1080000u);
    while(guiEflashWaitIrq == 1u)
    {

    }




    mcu_printf("Starts reset eflash controller... expected isr is PFLASH_INTEn_RESET_FIN\n");
    guiEflashWaitIrq = 1u;

    FLASHTEST_WRITEREG(0x100u, 0xA1080000u);
    while(guiEflashWaitIrq == 1u)
    {

    }
    FLASHTEST_WRITEREG(0u, 0xA10A001CU);
    mcu_printf("---eflash int test complete......\n");
    return ret;
}

static uint32 EFLASHTest_ECC_DFlash(uint32 uiFAULTEn)
{
    uint32 ret = (uint32)FCONT_RET_OK;
    volatile uint32 reg_temp = 0u;
    const uint32 uitempbuff[2] = {0x12345678u, 0xFFFF1234u};
    uint32 LockNSel = FLASHTEST_READREG(0xA10A0014U);//FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS);
    uint32 targetAddr = 0u;
    uint8 Lock = (uint8)((LockNSel & 0x40u) >> 6u);
    uint8 Sel  = (uint8)((LockNSel & 0x30u) >> 4u);

    if(Lock == 1u)
    {
        targetAddr += (Sel * 0x10000u);
    }
    else
    {
        mcu_printf("dflash belonging to r5 does not exist\n");
    }

    //0. hw sema set

    FLASHTEST_WRITEREG(1u, 0xA10A001CU);


    // 1. eflash targetAddr address


    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x40u;
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0x40u;
    }
    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(1u, 0xA1080000u);
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0xf00u;
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0xf00u;
    }
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x30u;
    while(reg_temp != 0x30u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0x30u;
    }


    // 2. set ecc off
    //it is not neccessary  FLASHTEST_WRITEREG(TEST_EFLASH_PASSWORD, PFLASH_PW_ADDRESS);

    reg_temp = FLASHTEST_READREG(0xA1080034u);
    reg_temp = reg_temp & 0x0FFFFFFFu;      //bit 28 ecc off
    FLASHTEST_WRITEREG(reg_temp, 0xA1080034u);

    // 3. write 0x0~ 0x4
    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(uitempbuff[0], 0xA1080204u);
    FLASHTEST_WRITEREG(uitempbuff[1], 0xA1080210u);
    FLASHTEST_WRITEREG(2u, 0xA1080000u);
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x30u;
    while(reg_temp != 0x30u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = (reg_temp & 0x30u);
    }

    TEST_EFLASH_CACHE_FLUSH();

    // 4. ecc on
    reg_temp = FLASHTEST_READREG(0xA1080034u);
    reg_temp = (reg_temp | 0x10000000u);      //bit 28 ecc on
    FLASHTEST_WRITEREG(reg_temp, 0xA1080034u);

    if(uiFAULTEn == 1u)
    {
        reg_temp = FLASHTEST_READREG(DFLASH_FAULTEn_ADDRESS);
        reg_temp |= (DFLASH_FAULTEn_ECC_FAIL | DFLASH_FAULTEn_Privilege_access_FAIL);
        FLASHTEST_WRITEREG(reg_temp, DFLASH_FAULTEn_ADDRESS);
    }

    reg_temp = FLASHTEST_READREG(DFLASH_INTEn_ADDRESS);

    reg_temp |= (uint32)(DFLASH_INTEn_ECC_FAIL | DFLASH_Privilege_access_FAIL | DFLASH_INTEn_GLOBAL_INT);

    FLASHTEST_WRITEREG(reg_temp, DFLASH_INTEn_ADDRESS);


    // 5. read data 0x0~0xc

    mcu_printf("read reg x%x\n", FLASHTEST_READREG( (0x30000000u+targetAddr)) );
    FLASHTEST_WRITEREG(0u, 0xA10A001CU);
    return ret;
}


static uint32 EFLASHTest_DAccCnt(uint32 uiFAULTEn)
{
    uint32 ret = (uint32)FCONT_RET_OK;
    volatile uint32 reg_temp = 0u;
    uint32 ret_cnt = 3u;
    uint32 LockNSel = FLASHTEST_READREG(0xA10A0014U);//FLS_HARDWARE_E_DFLASH_LOCK_SEL_ADDRESS);
    uint32 targetAddr = 0u;
    uint8 Lock = (uint8)((LockNSel & 0x40u) >> 6u);
    uint8 Sel  = (uint8)((LockNSel & 0x30u) >> 4u);

    if(Lock == 1u)
    {
        targetAddr += (Sel * 0x10000u);
    }
    else
    {
        mcu_printf("dflash belonging to r5 does not exist\n");
    }
    //0. hw sema set

    FLASHTEST_WRITEREG(1u, 0xA10A001CU);

//clear test area
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x40u;
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0x40u;
    }
    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(1u, 0xA1080000u);
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0xf00u;
    while(reg_temp != 0u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0xf00u;
    }
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x30u;
    while(reg_temp != 0x30u)
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = reg_temp & 0x30u;
    }


///


    if(uiFAULTEn == 1u)
    {
        reg_temp = FLASHTEST_READREG(DFLASH_FAULTEn_ADDRESS);
        reg_temp |= (DFLASH_FAULTEn_ECC_FAIL | DFLASH_FAULTEn_Privilege_access_FAIL);
        FLASHTEST_WRITEREG(reg_temp, DFLASH_FAULTEn_ADDRESS);
    }

    reg_temp = FLASHTEST_READREG(DFLASH_INTEn_ADDRESS);

    reg_temp |= (uint32)(DFLASH_INTEn_ECC_FAIL | DFLASH_Privilege_access_FAIL | DFLASH_INTEn_GLOBAL_INT);;

    FLASHTEST_WRITEREG(reg_temp, DFLASH_INTEn_ADDRESS);

    TEST_EF_SETUSER();

    // 5. read data 0x0~0xc

    if(uiFAULTEn == 2u)
    {
        mcu_printf("read reg x%x\n", FLASHTEST_READREG((0x30000000u+targetAddr)) );
    }

    FLASHTEST_WRITEREG(targetAddr, 0xA1080200u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1080204u);
    FLASHTEST_WRITEREG(0x12345678u, 0xA1080210u);
    FLASHTEST_WRITEREG(2u, 0xA1080000u);
    reg_temp = FLASHTEST_READREG(0xA1080004u);
    reg_temp = reg_temp & 0x30u;
    while((reg_temp != 0x30u) && (ret_cnt > 0u))
    {
        reg_temp = FLASHTEST_READREG(0xA1080004u);
        reg_temp = (reg_temp & 0x30u);
        ret_cnt--;
    }


    TEST_EF_SETSVC();


#ifdef NEVER
    TEST_EFLASH_CACHE_FLUSH();
#endif /* NEVER */
    FLASHTEST_WRITEREG(0u, 0xA10A001CU);

    return ret;
}


void EFLASHTest_DflashEcc(uint32 uiFlag)
{

    uint32 uiRegtemp = 0u;

    if(uiFlag == 0u)
    {

        mcu_printf("ECC int mode test\n");

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);


        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }



        (void)EFLASHTest_ECC_DFlash(0u);

    }
    else if(uiFlag == 1u)
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_HIGH,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);

        mcu_printf("ECC int mode test\n");

        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_ECC_DFlash(1u);
    }
    else if(uiFlag == 2u)
    {

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);

        mcu_printf("ECC int mode test\n");

        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_ECC_DFlash(2u);
    }
    else
    {
        mcu_printf(" int not set, ECC test\n");
        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_ECC_DFlash(0u);
        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if((uiRegtemp & DFLASH_INTEn_ECC_FAIL) == DFLASH_INTEn_ECC_FAIL)
        {
            mcu_printf("DFLASH_INTEn_ECC_FAIL check ok\n");
        }
    }

    mcu_printf("=========== Done ===========\n");
    return;
}

void EFLASHTest_DflashAccessControl(uint32 uiFlag)
{

    uint32 uiRegtemp = 0u;

    if(uiFlag == 0u)
    {

        mcu_printf("ACC cnt int mode test\n");

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);


        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }


        (void)EFLASHTest_DAccCnt(0u);

    }
    else if(uiFlag == 1u)
    {
        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);

        mcu_printf("ACC cnt  int mode test\n");

        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_DAccCnt(1u);
    }
    else if(uiFlag == 2u)
    {

        (void)FMU_IsrHandler((FMUFaultid_t)FMU_ID_DFLASH,
                            (FMUSeverityLevelType_t)FMU_SVL_LOW,
                            (FMUIntFnctPtr)&TEST_EFLASH_ISR4,
                            (void *)&guiFmuIsrFlag);

        (void)FMU_Set((FMUFaultid_t)FMU_ID_DFLASH);

        mcu_printf("ACC cnt  int mode test\n");

        (void)GIC_IntVectSet(GIC_DFLASH,
                             GIC_PRIORITY_NO_MEAN,
                             GIC_INT_TYPE_LEVEL_HIGH,
                             (GICIsrFunc)&TEST_EFLASH_ISR4,
                             &guiIsrFlag);


        (void)GIC_IntSrcEn(GIC_DFLASH);




        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_DAccCnt(2u);
    }
    else
    {
        mcu_printf(" ACC cnt int not set, ECC test\n");
        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if(uiRegtemp != 0u)
        {
            mcu_printf("Eflash Interrupt status register is not 0, It is not in a normal operating state. \n");
            mcu_printf("stop ecc test \n");
        }

        (void)EFLASHTest_DAccCnt(0u);
        uiRegtemp = FLASHTEST_READREG(DFLASH_INTSTAT_ADDRESS);
        if((uiRegtemp & DFLASH_INTEn_ECC_FAIL) == DFLASH_INTEn_ECC_FAIL)
        {
            mcu_printf("DFLASH_INTEn_ECC_FAIL check ok\n");
        }
    }

    mcu_printf("=========== Done ===========\n");
    return;
}

static uint8 bufftemp[2048];

void EFLASHTest_flashEraseWrite(uint32 type)
{

    uint32 uI;
    uint32 targetAddr   = 0u;
    uint32 targetSize   = 0x10000u;//64k//256k
    uint32 retTick      = 0u;
    uint32 retEraseTick = 0u;
    uint32 retWriteTick = 0u;
    uint32 retCmpTick = 0u;
    sint32 sRet            = 0;   /* to get the return of SAL_MemCmp */
    const uint32 DflashOffset = 0x30000u;  //R5 accessabile
    if(type == (uint32)FCONT_TYPE_PFLASH)
    {
        targetSize = 0x200000u;//2M
    }
    for(uI = 0u; uI <2048u; uI++)\
    {
        bufftemp[uI] =(uint8)(uI % 256u);
    }
    (void)SAL_GetTickCount(&retTick);

    //ease

    if(type == (uint32)FCONT_TYPE_PFLASH)
    {
        (void)EFLASH_Erase(0u,targetSize,FCONT_TYPE_PFLASH);
    }
    else
    {
        (void)EFLASH_Erase(DflashOffset,targetSize,FCONT_TYPE_DFLASH);
    }
    (void)SAL_GetTickCount(&retEraseTick);

    //write

    if(type == (uint32)FCONT_TYPE_PFLASH)
    {
        for(targetAddr = 0u; targetAddr < targetSize; targetAddr+= 2048u)
        {
            (void)EFLASH_WriteByte(targetAddr, bufftemp, 2048u,FCONT_TYPE_PFLASH);
        }
    }
    else
    {
        for(targetAddr = DflashOffset; targetAddr < (DflashOffset + targetSize); targetAddr+= 2048u)
        {
            (void)EFLASH_WriteByte(targetAddr, bufftemp, 2048u,FCONT_TYPE_DFLASH);
        }
    }
    (void)SAL_GetTickCount(&retWriteTick);

    if(type == (uint32)FCONT_TYPE_PFLASH)
    {
        for(targetAddr = 0u; targetAddr < targetSize; targetAddr+= 2048u)
        {
            SAL_MemCmp((const void *)bufftemp, (const void *)(targetAddr+0x20000000u), 2048u, &sRet);
            if (sRet != 0)
            {
                mcu_printf("targetAddr x%x cmpare fail \n",targetAddr);
            }
        }
    }
    else
    {
        for(targetAddr = DflashOffset; targetAddr < (DflashOffset + targetSize); targetAddr+= 2048u)
        {
            SAL_MemCmp((const void *)bufftemp, (const void *)(targetAddr+0x30000000u), 2048u, &sRet);
            if (sRet != 0)
            {
                mcu_printf("targetAddr x%x cmpare fail \n",targetAddr);
            }
        }
    }

    (void)SAL_GetTickCount(&retCmpTick);

    mcu_printf("type x%x \n",type);
    if(retEraseTick > retTick)
    {
        mcu_printf("tick erase %d\n", retEraseTick - retTick);
    }

    if(retWriteTick > retEraseTick)
    {
        mcu_printf("tick write %d\n",retWriteTick- retEraseTick);
    }

    if(retCmpTick > retWriteTick)
    {
        mcu_printf("tick cmp %d\n",retCmpTick- retWriteTick);
    }

    return;
}

void EFLASHTest_snorEraseWrite(uint32 type)
{

    uint32 uI;
    uint32 targetAddr   = 0u;
    uint32 targetSize   = 0x200000u;
    uint32 retTick      = 0u;
    uint32 retEraseTick = 0u;
    uint32 retWriteTick = 0u;
    uint32 retCmpTick   = 0u;
    sint32 sRet         = 0;   /* to get the return of SAL_MemCmp */
    sint32 ret          = 0;
    uint32 uiDrvidx     = (uint32)SFMC0;
    uint32 uiReadOffset = 0u;
    if(type == 1u)
    {
        uiDrvidx = SFMC1;
        uiReadOffset = 0x48000000u;
    }
    else
    {
        FLASHTEST_WRITEREG(0x5afeace5u, 0xA0F26048u);
        FLASHTEST_WRITEREG(0u, 0xA0F26154u);
        uiReadOffset = 0x40000000u;
    }

    if(uiDrvidx == (uint32)SFMC0)
    {
        FLASHTEST_WRITEREG(0xE0000005u, 0xA0F24028u); //100Mhz //for test
    }
    else
    {
        FLASHTEST_WRITEREG(0xE0000005u, 0xA0F24060u); //100Mhz //for test
    }

    ret = SNOR_MIO_Init(uiDrvidx);

    if (ret == 0)
    {
        mcu_printf("SFMC%d Init Done.\n",uiDrvidx);
    }


    for(uI = 0u; uI <2048u; uI++)\
    {
        bufftemp[uI] = (uint8)(uI % 256u);
    }


    (void)SAL_GetTickCount(&retTick);

    //ease

    (void)SNOR_MIO_Erase(uiDrvidx, 0u,targetSize);

    (void)SAL_GetTickCount(&retEraseTick);

    if(retEraseTick > retTick)
    {
        mcu_printf("tick erase %d\n", retEraseTick - retTick);
    }
    (void)SAL_GetTickCount(&retEraseTick);

    //write


    for(targetAddr = 0u; targetAddr < targetSize; targetAddr+= 2048u)
    {
        (void)SNOR_MIO_Write(uiDrvidx, targetAddr, bufftemp, 2048u);
    }

    (void)SAL_GetTickCount(&retWriteTick);

    if(retWriteTick > retEraseTick)
    {
        mcu_printf("tick write %d\n",retWriteTick- retEraseTick);
    }


    (void)SAL_GetTickCount(&retWriteTick);
    SNOR_MIO_AutoRun(uiDrvidx, 1u, 1u);


    for(targetAddr = 0u; targetAddr < targetSize; targetAddr+= 2048u)
    {
        SAL_MemCmp((const void *)bufftemp, (const void *)(targetAddr+uiReadOffset), 2048u, &sRet);
        if (sRet != 0)
        {
            mcu_printf("targetAddr x%x cmpare fail \n",targetAddr);
        }
    }


    (void)SAL_GetTickCount(&retCmpTick);

    mcu_printf("type x%x \n",type);

    if(retCmpTick > retWriteTick)
    {
        mcu_printf("tick cmp %d\n",retCmpTick- retWriteTick);
    }
   // SNOR_MIO_AutoRun(SFMC0,0u, 1u);

    return;
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_EFLASH == 1 )

