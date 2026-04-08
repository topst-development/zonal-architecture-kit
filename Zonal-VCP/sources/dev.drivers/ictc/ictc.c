// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : ictc.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

#include "reg_phys.h"
#include "ictc.h"
#include "clock.h"
#include "clock_dev.h"
#include "gic.h"

/***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************/

static uint32 timestamp_rising_edge[ICTC_TOTAL_CHANNEL_NUM][ICTC_TIMESTAMP_RECORD_MAXCNT];
static uint32 timestamp_falling_edge[ICTC_TOTAL_CHANNEL_NUM][ICTC_TIMESTAMP_RECORD_MAXCNT];
static uint32 r_tstmp_idx[ICTC_TOTAL_CHANNEL_NUM];
static uint32 f_tstmp_idx[ICTC_TOTAL_CHANNEL_NUM];

static ICTCCallBack_t user_priv[ICTC_TOTAL_CHANNEL_NUM];

#ifdef ICTC_USER_CUS_FUNCTION
static ICTCIsrResult_t isr_result;
#endif

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


#ifdef ICTC_USER_CUS_FUNCTION
/*
***************************************************************************************************
*                                   ICTC_GetIsrResult
*
* Function to check the interrupt result (Can be changed by user)
* @param    channel number 0 ~ 2.
* Notes
*
***************************************************************************************************
*/
void ICTC_GetIsrResult
(
    ICTCIsrResult_t *                   psIsrResult
)
{
    SAL_MemCopy(psIsrResult, &isr_result, sizeof(ICTCIsrResult_t));
}

/*
***************************************************************************************************
*                                          ICTC_ClearIsrResult
*
* Function to check the interrupt result (Can be changed by user)
* @param
* Notes
*
***************************************************************************************************
*/
void ICTC_ClearIsrResult
(
    void
)
{
    ICTC_D("ICTC_IRQ_NOISEFLT_FULL : %d\n", isr_result.irNoisefltFullCnt);
    ICTC_D("ICTC_IRQ_TOCNT_FULL : %d\n", isr_result.irTocntFullCnt);
    ICTC_D("ICTC_IRQ_ECNT_FULL : %d\n", isr_result.irEcntFullCnt);
    ICTC_D("ICTC_IRQ_DT_ERR : %d\n", isr_result.irDtErrCnt);
    ICTC_D("ICTC_IRQ_PRD_ERR : %d\n", isr_result.irPrdErrCnt);
    ICTC_D("ICTC_IRQ_DTPRDCMP_FULL : %d\n", isr_result.irDtPrdCmpFullCnt);
    ICTC_D("ICTC_IRQ_REDGE_NOTI : %d\n", isr_result.irRedgeNotiCnt);
    ICTC_D("ICTC_IRQ_FEDGE_NOTI : %d\n", isr_result.irFedgeNotiCnt);

    SAL_MemSet(&isr_result, 0UL, sizeof(ICTCIsrResult_t));
}

#endif /* End of ICTC_USER_CUS_FUNCTION */

/*
***************************************************************************************************
*                                          ICTC_Init
*
* Initialize register map
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

void ICTC_Init
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC0, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC0);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC1, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC1);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC2, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC2);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC3, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC3);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC4, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC4);
    (void)CLOCK_SetPeriRate((sint32)CLOCK_PERI_ICTC5, ICTC_PERI_CLOCK);
    (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ICTC5);

    ICTC_D("\n ICTC ch0-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC0));
    ICTC_D("\n ICTC ch1-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC1));
    ICTC_D("\n ICTC ch2-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC2));
    ICTC_D("\n ICTC ch3-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC3));
    ICTC_D("\n ICTC ch4-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC4));
    ICTC_D("\n ICTC ch5-peri clk >> %dHz \n", CLOCK_GetPeriRate((sint32)CLOCK_PERI_ICTC5));
}

/*
***************************************************************************************************
*                                          ICTC_Deinit
*
* De-Initialize register map
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

void ICTC_Deinit
(
    void
)
{
    /*set peri clock*/
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC0);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC1);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC2);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC3);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC4);
    (void)CLOCK_DisablePeri((sint32)CLOCK_PERI_ICTC5);
}

/*
***************************************************************************************************
*                                          ICTC_GetIRQCtrlReg
*
* ICTC Interrupt Handler
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

static uint32 ICTC_GetIRQCtrlReg
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        val = SAL_ReadReg(reg);
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPrevPeriodCnt
*
* ICTC get previous period Count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetPrePeriodCnt
(
    uint32                              uiChannel
)
{
    uint32  val             = 0;
    uint32  reg             = 0;
    uint32  sub_reg         = 0;
    uint32  sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_CNT_PRE_PRD + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_CNT_PRE_PRD + (uiChannel*ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_PRD) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);
        }

#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPreDutyCnt
*
* ICTC get previous Duty Count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetPreDutyCnt
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_CNT_PRE_DUTY + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_CNT_PRE_DUTY + (uiChannel*ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CNT_PRE_DUTY) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetCurEdgeCnt
*
* ICTC get current edge count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetCurEdgeCnt
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_CUR_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_CUR_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(CUR_EDGE_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetPrvEdgeCnt
*
* ICTC get previous edge count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/

uint32 ICTC_GetPrvEdgeCnt
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_PRV_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_PRV_EDGE_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(PRV_EDGE_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetREdgeTstmpCnt
*
* ICTC get rising edge timestamp count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetREdgeTstmpCnt
(
    uint32                              uiChannel
)
{
    uint32          val             = ICTC_REG_MAX_VALUE;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_R_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_R_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(R_EDGE_TSTMP_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);
        }
        else if(r_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_rising_edge[uiChannel][r_tstmp_idx[uiChannel]] = val;
            r_tstmp_idx[uiChannel]++;

            if(r_tstmp_idx[uiChannel] >= ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                r_tstmp_idx[uiChannel] = 0UL;
            }
        }
        else
        {
            ICTC_D("%s : do nothing.\n", __func__);
        }
#else
        if(r_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_rising_edge[uiChannel][r_tstmp_idx[uiChannel]] = val;
            r_tstmp_idx[uiChannel]++;

            if(r_tstmp_idx[uiChannel] >= ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                r_tstmp_idx[uiChannel] = 0UL;
            }
        }
#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_GetFEdgeTstmpCnt
*
* ICTC get falling edge timestamp count value
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
uint32 ICTC_GetFEdgeTstmpCnt
(
    uint32                              uiChannel
)
{
    uint32          val             = ICTC_REG_MAX_VALUE;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          sub_val         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_F_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        val     = SAL_ReadReg(reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_F_EDGE_TSTMP_CNT + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_val = SAL_ReadReg(sub_reg);

        if((uint32)ICTC_DIFF_MARGIN < (uint32)ICTC_DiffABSValue(val, sub_val))
        {
            ICTC_Err("Redundancy ICTC(F_EDGE_TSTMP_CNT) is not same, main 0x%x redundancy 0x%x\n", val, sub_val);

        }
        else if(f_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_falling_edge[uiChannel][f_tstmp_idx[uiChannel]]   = val;
            f_tstmp_idx[uiChannel]++;

            if(f_tstmp_idx[uiChannel] >= (uint32)ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                f_tstmp_idx[uiChannel]  = 0UL;
            }
        }
        else
        {
            ICTC_D("%s : do nothing.\n", __func__);
        }
#else
        if(f_tstmp_idx[uiChannel] < ICTC_TIMESTAMP_RECORD_MAXCNT )
        {
            timestamp_falling_edge[uiChannel][f_tstmp_idx[uiChannel]]   = val;
            f_tstmp_idx[uiChannel]++;

            if(f_tstmp_idx[uiChannel] >= (uint32)ICTC_TIMESTAMP_RECORD_MAXCNT )
            {
                f_tstmp_idx[uiChannel]  = 0UL;
            }
        }
#endif
    }

    return val;
}

/*
***************************************************************************************************
*                                          ICTC_ClearOpEnCtrlCounter
*
* ICTC Clear OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
static void ICTC_ClearOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntValue
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        val = (SAL_ReadReg(reg) | uiCntValue);
        SAL_WriteReg(val, reg);

        val = (SAL_ReadReg(reg) & ~(uiCntValue));
        SAL_WriteReg(val, reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        val = (SAL_ReadReg(sub_reg) | uiCntValue);
        SAL_WriteReg(val, sub_reg);

        val = (SAL_ReadReg(sub_reg) & ~(uiCntValue));
        SAL_WriteReg(val, sub_reg);
#endif
    }
}

/*
***************************************************************************************************
*                                          ICTC_EnableCapture
*
* ICTC Enable Operation, Enable TCLK and ICTC EN
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
void ICTC_EnableCapture
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        /* enable TCLK_EN */
        val = (SAL_ReadReg(reg) | ICTC_OPEN_CTRL_TCLK_EN);
        SAL_WriteReg(val, reg);
        /* enable ictc */
        val = (SAL_ReadReg(reg) | ICTC_OPEN_CTRL_EN);
        SAL_WriteReg(val, reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        /* enable TCLK_EN */
        val = (SAL_ReadReg(sub_reg) | ICTC_OPEN_CTRL_TCLK_EN);
        SAL_WriteReg(val, sub_reg);
        /* enable sub-ictc */
        val = (SAL_ReadReg(sub_reg) | ICTC_OPEN_CTRL_EN);
        SAL_WriteReg(val, sub_reg);
#endif
    }
    else
    {
        ICTC_Err("ICTC invalid uiChannel value \n");
    }
}

/*
***************************************************************************************************
*                                          ICTC_DisableCapture
*
* ICTC Diable Operation, Clear TCLK and ICTC EN
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
void ICTC_DisableCapture
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        ICTC_ClearOpEnCtrlCounter(uiChannel,
                                  (ICTC_OPEN_CTRL_TSCNT_CLR  |
                                  ICTC_OPEN_CTRL_FLTCNT_CLR  |
                                  ICTC_OPEN_CTRL_TOCNT_CLR   |
                                  ICTC_OPEN_CTRL_EDEGCNT_CLR |
                                  ICTC_OPEN_CTRL_PDCNT_CLR));

        /* enable TCLK_EN */
        val = SAL_ReadReg(reg) & ~(ICTC_OPEN_CTRL_TCLK_EN);
        SAL_WriteReg(val, reg);
        /* enable ictc */
        val = SAL_ReadReg(reg) & ~(ICTC_OPEN_CTRL_EN);
        SAL_WriteReg(val, reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        /* enable TCLK_EN */
        val = SAL_ReadReg(sub_reg) & ~(ICTC_OPEN_CTRL_TCLK_EN);
        SAL_WriteReg(val, sub_reg);
        /* enable sub-ictc */
        val = SAL_ReadReg(sub_reg) & ~(ICTC_OPEN_CTRL_EN);
        SAL_WriteReg(val, sub_reg);
#endif
    }
    else
    {
        ICTC_Err("ICTC invalid uiChannel value \n");
    }

}

/*
***************************************************************************************************
*                                          ICTC_SetOpEnCtrlCounter
*
* ICTC set OP_EN_CTRL Counter Register
* @param    channel number 0 ~ 5.
* @param    counter valuie
* Notes
*
***************************************************************************************************
*/
void ICTC_SetOpEnCtrlCounter
(
    uint32                              uiChannel,
    uint32                              uiCntEnValue
)
{
    uint32          reg             = 0;
    uint32          sub_reg         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        SAL_WriteReg(uiCntEnValue, reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_OP_EN_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        SAL_WriteReg(uiCntEnValue, sub_reg);
#endif
    }
    else
    {
        ICTC_Err("ICTC invalid uiChannel value \n");
    }

}

/*
***************************************************************************************************
*                                          ICTC_SetOpModeCtrlReg
*
* ICTC set OP_MODE_CTRL Register
* @param    channel number 0 ~ 5.
* @param    OP_MODE Value

* Notes
*
***************************************************************************************************
*/
void ICTC_SetOpModeCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiModeValue
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          f_in_sel_val    = 0;

    /* check f_in_sel range */
    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg             = ICTC_CHC0_BASE + ICTC_OP_MODE_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        f_in_sel_val    = (uiModeValue & ICTC_OPMODE_CTRL_F_IN_SEL_BIT_MASK);

        if((f_in_sel_val < 242UL) || (244UL <= f_in_sel_val))
        {
            SAL_WriteReg(ICTC_OPMODE_CTRL_RESET_VALUE, reg);
            val = SAL_ReadReg(reg) | uiModeValue;
            SAL_WriteReg(val, reg);

#ifdef ICTC_SUB_ICTC_ENABLED
            sub_reg         = ICTC_RED_CHC0_BASE + ICTC_OP_MODE_CTRL + (uiChannel*ICTC_CHANNEL_OFFSET);

            SAL_WriteReg(ICTC_OPMODE_CTRL_RESET_VALUE, sub_reg);
            SAL_WriteReg(uiModeValue, sub_reg);
#endif
        }
        else
        {
            ICTC_Err("\n ICTC invalid f_in_sel value \n");
        }
    }
    else
    {
        ICTC_Err("\n ICTC invalid uiChannel value \n");
    }
}

/*
***************************************************************************************************
*                                          ICTC_ClearIRQCTRLReg
*
* ICTC Clear IRQ_CTRL Register
* @param    channel number 0 ~ 5.
* Notes
*
***************************************************************************************************
*/
static void ICTC_ClearIRQCTRLReg
(
    uint32                              uiChannel
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;
    uint32          irq_en          = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        /*read IRQ_EN 31:24*/
        irq_en = ((uint32)SAL_ReadReg(reg) >> ICTC_IRQ_CTRL_IRQEN_OFFSET);

        val = SAL_ReadReg(reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET);
        SAL_WriteReg(val, reg);
        SAL_WriteReg(ICTC_REG_ZERO_VALUE, reg);
        SAL_WriteReg((irq_en << ICTC_IRQ_CTRL_IRQEN_OFFSET), reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);
        irq_en = ((uint32)SAL_ReadReg(sub_reg) >> ICTC_IRQ_CTRL_IRQEN_OFFSET);

        val = SAL_ReadReg(sub_reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET);
        SAL_WriteReg(val, sub_reg);
        SAL_WriteReg(ICTC_REG_ZERO_VALUE, sub_reg);
        SAL_WriteReg((irq_en << ICTC_IRQ_CTRL_IRQEN_OFFSET), sub_reg);
#endif
    }
}

/*
***************************************************************************************************
*                                          ICTC_IrqHandler
*
* ICTC Interrupt Handler
*  - User can change 'isr_result' parameter and sub_functions according to purpose of ICTC
* @param    user data argument.
* Notes
*
***************************************************************************************************
*/
static void ICTC_IrqHandler
(
    void *                              pArg
)
{
    ICTCCallBack_t *    pdata       = NULL_PTR;
    uint32              channel     = 0;
    uint32              val     = 0;
#ifdef ICTC_USER_CUS_FUNCTION
    uint32              period_val  = 0;
    uint32              duty_val    = 0;
#endif

    (void)SAL_MemCopy(&pdata, (const void *)&pArg, sizeof(ICTCCallBack_t *));

    if (pdata != NULL_PTR)
    {
        channel = pdata->cbChannel;
        val = ICTC_GetIRQCtrlReg(channel);

        if((val & ICTC_IRQ_CTRL_NOISEFLT_FULL_IRQ) == ICTC_IRQ_CTRL_NOISEFLT_FULL_IRQ)
        {
            ;
        }

#ifdef ICTC_USER_CUS_FUNCTION

        if((val & ICTC_IRQ_CTRL_TOCNT_FULL_IRQ) == ICTC_IRQ_CTRL_TOCNT_FULL_IRQ)
        {
            ;
        }

        if((val & ICTC_IRQ_CTRL_ECNT_FULL_IRQ) == ICTC_IRQ_CTRL_ECNT_FULL_IRQ)
        {
            if(isr_result.irEcntFullCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irEcntFullCnt++;
            }
        }

        if((val & ICTC_IRQ_CTRL_DTPRDCMP_FULL_IRQ) == ICTC_IRQ_CTRL_DTPRDCMP_FULL_IRQ)
        {
            if(isr_result.irDtPrdCmpFullCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irDtPrdCmpFullCnt++;
            }
        }

        if((val & ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ) == ICTC_IRQ_CTRL_FEDGE_NOTI_IRQ)
        {
            if(isr_result.irFedgeNotiCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irFedgeNotiCnt++;
            }

            (void)ICTC_GetFEdgeTstmpCnt(channel);
        }

        if((val & ICTC_IRQ_CTRL_REDGE_NOTI_IRQ) == ICTC_IRQ_CTRL_REDGE_NOTI_IRQ)
        {
            if(isr_result.irRedgeNotiCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irRedgeNotiCnt++;
            }

            (void)ICTC_GetREdgeTstmpCnt(channel);
        }

        if((val & ICTC_IRQ_CTRL_PRD_ERR_IRQ) == ICTC_IRQ_CTRL_PRD_ERR_IRQ)
        {
            if(isr_result.irPrdErrCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irPrdErrCnt++;
            }

            if (pdata->cbCallBackFunc != (ICTCCallback)NULL)
            {
                period_val = ICTC_GetPrePeriodCnt(channel);
                duty_val = ICTC_GetPreDutyCnt(channel);
                pdata->cbCallBackFunc(channel, period_val, duty_val);
            }
        }

        if((val & ICTC_IRQ_CTRL_DT_ERR_IRQ) == ICTC_IRQ_CTRL_DT_ERR_IRQ)
        {
            if(isr_result.irDtErrCnt < ICTC_REG_MAX_VALUE)
            {
                isr_result.irDtErrCnt++;
            }

            if (pdata->cbCallBackFunc != (ICTCCallback)NULL)
            {
                period_val = ICTC_GetPrePeriodCnt(channel);
                duty_val = ICTC_GetPreDutyCnt(channel);
                pdata->cbCallBackFunc(channel, period_val, duty_val);
            }
        }

        /* clear interrupt */
        ICTC_ClearIRQCTRLReg(channel);
#endif  /* End of ICTC_USER_CUS_FUNCTION */
    }
    else
    {
        ICTC_Err("ICTC Interrupt invalid argmument! (NULL_PTR)\n");
    }
}

/*
***************************************************************************************************
*                                          ICTC_SetIRQCtrlReg
*
* ICTC set IRQ_CTRL Register
* @param    channel number 0 ~ 5.
* @param    IRQ_EN
* Notes
*
***************************************************************************************************
*/
void ICTC_SetIRQCtrlReg
(
    uint32                              uiChannel,
    uint32                              uiIRQValue
)
{
    uint32          val             = 0;
    uint32          reg             = 0;
    uint32          sub_reg         = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel * ICTC_CHANNEL_OFFSET);

        /* clear interrupt flag */
        val = SAL_ReadReg(reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET);
        SAL_WriteReg(val, reg);

        /* IRQ_EN */
        SAL_WriteReg(ICTC_REG_ZERO_VALUE, reg); /* reset 0x00 */
        SAL_WriteReg( (uiIRQValue << ICTC_IRQ_CTRL_IRQEN_OFFSET), reg);
#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_IRQ_CTRL + (uiChannel*ICTC_CHANNEL_OFFSET);

        /* clear interrupt flag */
        val = SAL_ReadReg(sub_reg) | ((uint32)(0xFFUL) << ICTC_IRQ_CTRL_IRQCLR_OFFSET);
        SAL_WriteReg(val, sub_reg);

        /* IRQ_EN */
        SAL_WriteReg(ICTC_REG_ZERO_VALUE, sub_reg); /* reset 0x00 */
        SAL_WriteReg( (uiIRQValue << ICTC_IRQ_CTRL_IRQEN_OFFSET), sub_reg);
#endif
        switch(uiChannel)
        {
            case 0:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC0, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC0);
                break;
            }

            case 1:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC1, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC1);
                break;
            }

            case 2:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC2, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC2);
                break;
            }

            case 3:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC3, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC3);
                break;
            }

            case 4:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC4, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC4);
                break;
            }

            case 5:
            {
                (void)GIC_IntVectSet((uint32)GIC_ICTC5, (uint32)GIC_PRIORITY_NO_MEAN, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ICTC_IrqHandler, (void*)&user_priv[uiChannel]);
                (void)GIC_IntSrcEn((uint32)GIC_ICTC5);
                break;
            }

            default:
			{
                ICTC_Err("Wrong argument (uiChannel : %d)\n", uiChannel);
                break;
			}
        }

#ifdef ICTC_USER_CUS_FUNCTION
        SAL_MemSet(&isr_result, 0UL, sizeof(ICTCIsrResult_t));  // result values are initialized
#endif
    }
}

/*
***************************************************************************************************
*                                          ICTC_SetCallBackFunc
*
* ICTC set CallbackFunction
* @param    channel number 0 ~ 5.
* @param    CallBack Function
* Notes
*
***************************************************************************************************
*/
void ICTC_SetCallBackFunc
(
    uint32                              uiChannel,
    ICTCCallback                        pCallbackFunc
)
{
    if((uiChannel < ICTC_TOTAL_CHANNEL_NUM) && (pCallbackFunc != NULL))
    {
        user_priv[uiChannel].cbChannel      = uiChannel;
        user_priv[uiChannel].cbCallBackFunc = pCallbackFunc;
    }
	else
    {
        ICTC_Err("There is no CallbackFunc (NULL)\n");
    }
}

/*
***************************************************************************************************
*                                          ICTC_SetTimeoutValue
*
* ICTC set Time-Out Value Register
* @param    channel number 0 ~ 5.
* @param    timeout value.
* Notes
*
***************************************************************************************************
*/
void ICTC_SetTimeoutValue
(
    uint32                              uiChannel,
    uint32                              uiTimeoutValue
)
{
    uint32          reg             = 0;
    uint32          sub_reg         = 0;


    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        reg     = ICTC_CHC0_BASE + ICTC_TIME_OUT + (uiChannel * ICTC_CHANNEL_OFFSET);
        SAL_WriteReg((0x0FFFFFFFUL & uiTimeoutValue), reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_reg = ICTC_RED_CHC0_BASE + ICTC_TIME_OUT + (uiChannel*ICTC_CHANNEL_OFFSET);
        SAL_WriteReg((0x0FFFFFFFUL & uiTimeoutValue), sub_reg);
#endif
    }
}

/*
***************************************************************************************************
*                                          ICTC_SetEdgeMatchingValue
*
* ICTC set Edge Matching Value, R_EDGE, F_EDGE, EDGE_COUNTER
* @param    channel number 0 ~ 5.
* @param    rising edge matching value.
* @param    falling edge matching value.
* @param    edge counter matching value.
* @return
*
* Notes
*
***************************************************************************************************
*/
void ICTC_SetEdgeMatchingValue
(
    uint32                              uiChannel,
    uint32                              uiRisingEdgeMat,
    uint32                              uiFallingEdgeMat,
    uint32                              uiEdgeCounterMat
)
{
    uint32          r_edge_reg          = 0;
    uint32          f_edge_reg          = 0;
    uint32          edge_cnt_reg        = 0;
    uint32          sub_r_edge_reg      = 0;
    uint32          sub_f_edge_reg      = 0;
    uint32          sub_edge_cnt_reg    = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        r_edge_reg          = ICTC_CHC0_BASE + ICTC_R_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
        f_edge_reg          = ICTC_CHC0_BASE + ICTC_F_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
        edge_cnt_reg        = ICTC_CHC0_BASE + ICTC_EDGE_CNT_MAT + (uiChannel * ICTC_CHANNEL_OFFSET);

        SAL_WriteReg((0x0FFFFFFFUL & uiRisingEdgeMat), r_edge_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiFallingEdgeMat), f_edge_reg);
        SAL_WriteReg((0x0000FFFFUL & uiEdgeCounterMat), edge_cnt_reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_r_edge_reg = ICTC_RED_CHC0_BASE + ICTC_R_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_f_edge_reg = ICTC_RED_CHC0_BASE + ICTC_F_EDGE + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_edge_cnt_reg = ICTC_RED_CHC0_BASE + ICTC_EDGE_CNT_MAT + (uiChannel * ICTC_CHANNEL_OFFSET);

        SAL_WriteReg((0x0FFFFFFFUL & uiRisingEdgeMat), sub_r_edge_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiFallingEdgeMat), sub_f_edge_reg);
        SAL_WriteReg((0x0000FFFFUL & uiEdgeCounterMat), sub_edge_cnt_reg);
#endif
    }
}

/*
***************************************************************************************************
*                                          ICTC_SetCompareRoundValue
*
* ICTC set Period/Duty compare round value configuration register
* @param    channel number 0 ~ 5.
* @param    period compare round value.
* @param    duty compare round value.
* Notes
*
***************************************************************************************************
*/
void ICTC_SetCompareRoundValue
(
    uint32                              uiChannel,
    uint32                              uiPeriodCompareRound,
    uint32                              uiDutyCompareRound
)
{
    uint32          prd_cmp_reg         = 0;
    uint32          duty_cmp_reg        = 0;
    uint32          sub_prd_cmp_reg     = 0;
    uint32          sub_duty_cmp_reg    = 0;

    if(uiChannel < ICTC_TOTAL_CHANNEL_NUM)
    {
        prd_cmp_reg         = ICTC_CHC0_BASE + ICTC_PRD_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);
        duty_cmp_reg        = ICTC_CHC0_BASE + ICTC_DUTY_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);

        SAL_WriteReg((0x0FFFFFFFUL & uiPeriodCompareRound), prd_cmp_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiDutyCompareRound), duty_cmp_reg);

#ifdef ICTC_SUB_ICTC_ENABLED
        sub_prd_cmp_reg = ICTC_RED_CHC0_BASE + ICTC_PRD_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);
        sub_duty_cmp_reg = ICTC_RED_CHC0_BASE + ICTC_DUTY_CMP_RND + (uiChannel * ICTC_CHANNEL_OFFSET);

        SAL_WriteReg((0x0FFFFFFFUL & uiPeriodCompareRound), sub_prd_cmp_reg);
        SAL_WriteReg((0x0FFFFFFFUL & uiDutyCompareRound), sub_duty_cmp_reg);
#endif
    }
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_ICTC == 1 )

