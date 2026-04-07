// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : fmu.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#include <fmu.h>
#include <gic.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/



/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

static FMUIntVectPtr_t FMU_IntVectTbl[FMU_FAULT_SOURCE];

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void FMU_IrqCallBack
(
    void *                              arg
);

static void FMU_FiqCallBack
(
    void *                              arg
);

static void FMU_SetExtFault
(
    uint8                               uiPolarity,
    uint32                              uiLowDuration,
    uint32                              uiHighDuration
);

static void FMU_SwReset
(
    void
);

static FMUErrTypes_t FMU_IsrMaskClear
(
    FMUFaultid_t                        uiId
);


static FMUErrTypes_t FMU_IsrClear
(
    FMUFaultid_t                        uiId
);

static FMUErrTypes_t FMU_DsCompare
(
    uint32                              uiValue,
    uint32                              uiReg
);

static FMUSts_t FMU_GetIrqStatus
(
    void
);

static FMUSts_t FMU_GetFiqStatus
(
    void
);

/*
***************************************************************************************************
*                                          FMU_IrqCallBack
*
* @param    void *arg [in]
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_IrqCallBack
(
    void *                              arg
)
{
    FMUSts_t        status;
    FMUIntFnctPtr   p_isr;
    void *          int_arg;

    FMU_D("[%s] IN!!\n", __func__);

    //Unused Parameter
    (void)arg;

    status = FMU_GetIrqStatus();

    //range check
    if((status.id) > FMU_ID_FMU_FAULT)
    {
        FMU_E("[%s]IRQ id(%d) is invalid\n",__func__, status.id);
    }
    else
    {
        p_isr   = FMU_IntVectTbl[status.id].func;                     /* Fetch ISR handler.                                   */
        int_arg = FMU_IntVectTbl[status.id].arg;

        if(p_isr != (void *) 0)
        {
            FMU_E("[%s] Call Safety Mechanism IRQ handler[%d]\n",__func__,status.id);
            (*p_isr)(int_arg);                                      /* Call ISR handler.                                    */
        }
    }
}

/*
***************************************************************************************************
*                                          FMU_FiqCallBack
*
* @param    void *arg [in]
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_FiqCallBack
(
    void *                              arg
)
{
    FMUSts_t        status;
    FMUIntFnctPtr   p_isr;
    void *          int_arg;

    FMU_D("[%s] IN!!\n", __func__);

    //Unused Parameter
    (void)arg;

    status = FMU_GetFiqStatus();

    //range check
    if((status.id) > FMU_ID_FMU_FAULT)
    {
        FMU_E("[%s]FIQ id(%d) is invalid\n",__func__, status.id);
    }
    else
    {
        p_isr   = FMU_IntVectTbl[status.id].func;                     /* Fetch ISR handler.                                   */
        int_arg = FMU_IntVectTbl[status.id].arg;

        if(p_isr != (void *) 0)
        {
            FMU_E("[%s] Call Safety Mechanism FIQ handler[%d]\n",__func__,status.id);
            (*p_isr)(int_arg);                                      /* Call ISR handler.                                    */
        }
    }
}

/*
***************************************************************************************************
*                                          FMU_SetExtFault
*
* @param    uint8 uiPolarity [in]
* @param    uint32 low_duration [in] : Activation iinterval = 2^(LOW_TIMING + 13) x Clock Period (LOW_TIMING = 0, then interval is mesaured by 28.8ms.
* @param    uint32 high_duration [in] : Activation iinterval = 2^(HIGH_TIMING + 7) x Clock Period
*
* Notes
*
***************************************************************************************************
*/
static void FMU_SetExtFault
(
    uint8                               uiPolarity,
    uint32                              uiLowDuration,
    uint32                              uiHighDuration
)
{
    FMU_D("[%s] IN!! polarity %d low_duration %d high_duration %d\n", __func__, uiPolarity, uiLowDuration, uiHighDuration);

    if (uiPolarity > (uint8)2)
    {
        FMU_E("Polarity type(%d) is invalid @ FMU_SetExtFault()\n", uiPolarity);

    }
    else if (uiLowDuration > (uint32)16)
    {
        FMU_E("Fault low duration(%d) is invalid @ FMU_SetExtFault()\n", uiLowDuration);

    }
    else if (uiHighDuration > (uint32)2)
    {
        FMU_E("Fault high duration(%d) is invalid @ FMU_SetExtFault()\n", uiHighDuration);

    }
    else
    {
        FMU_REG->mcCfgWrPw              = FMU_PW;
        FMU_REG->mcFaultTiming.ftNreg  = ((uiHighDuration << (uint32)8) | (uiLowDuration << (uint32)4) | ((uint32)uiPolarity << (uint32)0));
        FMU_REG->mcCfgWrPw              = FMU_PW;
        FMU_REG->mcClkSel.csBreg.csClkSel  = 1; //todo checking after confirmation with SoC

    }
}


/*
***************************************************************************************************
*                                          FMU_SwReset
*
*
* Notes
*
***************************************************************************************************
*/
static void FMU_SwReset
(
    void
)
{
    FMU_REG->mcCfgWrPw                  = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSwReset    = 0x1;

    FMU_REG->mcCfgWrPw                  = FMU_PW;
    FMU_REG->mcCtrl.cuBreg.ctSwReset    = 0x0;
}

/*
***************************************************************************************************
*                                          FMU_IsrMaskClear
*
* @param    uint8 id [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_IsrMaskClear
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          IRQ_reg_64[2];
    uint32          FIQ_reg_64[2];
    uint32          FOULT_reg_64[2];
    FMUErrTypes_t   ret;

    ret     = FMU_OK;
    nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;
    nth_bit = ((uint32)uiId) & (uint32)0x1f;

    if (FMU_IntVectTbl[uiId].severity_level == FMU_SVL_LOW)
    {
        // for Double storage test
        IRQ_reg_64[nth_reg] = FMU_REG->mcIrqMask.euNreg[nth_reg];

        // Clear IRQ mask enable for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqMask.euNreg[nth_reg]  = (FMU_REG->mcIrqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        IRQ_reg_64[nth_reg] = (IRQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(IRQ_reg_64[nth_reg], FMU_REG->mcIrqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_MID)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqMask.euNreg[nth_reg];

        // Clear FIQ mask enable for given FMU ID                                               // 0 ~ 31
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqMask.euNreg[nth_reg]  = (FMU_REG->mcFiqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_HIGH)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqMask.euNreg[nth_reg];

        // Clear FIQ mask enable for given FMU ID                                               // 0 ~ 31
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqMask.euNreg[nth_reg]  = (FMU_REG->mcFiqMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        // for Double storage test
        FOULT_reg_64[nth_reg] = FMU_REG->mcFaultMask.euNreg[nth_reg];

        // Clear Fault mask enable for given FMU ID                                         // 0 ~ 31
        FMU_REG->mcCfgWrPw                      = FMU_PW;
        FMU_REG->mcFaultMask.euNreg[nth_reg]    = (FMU_REG->mcFaultMask.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        // for Double storage test
        FOULT_reg_64[nth_reg] = (FOULT_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FOULT_reg_64[nth_reg], FMU_REG->mcFaultMask.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }
    }
    else
    {
        ret = FMU_INVALID_SEVERITY_ERR;
    }

    return ret;
 }

/*
***************************************************************************************************
*                                          FMU_IsrClear
*
* @param    uint32 id [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_IsrClear
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          IRQ_reg_64[2];
    uint32          FIQ_reg_64[2];
    uint32          FOULT_reg_64[2];
    FMUErrTypes_t   ret;

    ret = FMU_OK;

    FMU_D("[%s] IN!! id %d \n", __func__, uiId);

    nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;                                                    // 0 ~ 1
    nth_bit = ((uint32)uiId   ) & (uint32)0x1f;                                                 // 0 ~ 31

    if (FMU_IntVectTbl[uiId].severity_level == FMU_SVL_LOW)
    {
        // for Double storage test
        IRQ_reg_64[nth_reg] = FMU_REG->mcIrqClr.euNreg[nth_reg];

        // Clear IRQ clear for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        IRQ_reg_64[nth_reg] = (IRQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(IRQ_reg_64[nth_reg], FMU_REG->mcIrqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        // for Double storage test
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcIrqClr.euNreg[nth_reg]   = 0;
    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_MID)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqClr.euNreg[nth_reg];

        // Clear FIQ for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = 0;
    }
    else if(FMU_IntVectTbl[uiId].severity_level == FMU_SVL_HIGH)
    {
        // for Double storage test
        FIQ_reg_64[nth_reg] = FMU_REG->mcFiqClr.euNreg[nth_reg];

        // Clear FIQ for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FIQ_reg_64[nth_reg] = (FIQ_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FIQ_reg_64[nth_reg], FMU_REG->mcFiqClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFiqClr.euNreg[nth_reg]   = 0;

        // for Double storage test
        FOULT_reg_64[nth_reg] = FMU_REG->mcFaultClr.euNreg[nth_reg];

        // Clear Fault clear for given FMU ID
        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFaultClr.euNreg[nth_reg] = ((uint32)0x1 << nth_bit);

        // for Double storage test
        FOULT_reg_64[nth_reg] = (FOULT_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(FOULT_reg_64[nth_reg], FMU_REG->mcFaultClr.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        FMU_REG->mcCfgWrPw                  = FMU_PW;
        FMU_REG->mcFaultClr.euNreg[nth_reg] = 0;

    }
    else
    {
        ret = FMU_INVALID_SEVERITY_ERR;
    }

    return ret;
}

/*
***************************************************************************************************
*                                          DS_Compare
*
* @param    uint32 value [in]
* @param    uint32 reg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUErrTypes_t FMU_DsCompare
(
    uint32                              uiValue,
    uint32                              uiReg
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    if(uiValue != uiReg)
    {
        ret = FMU_DS_COMPARE_FAIL_ERR;
    }
    else
    {
        FMU_E("[%s]DS compare result sucess\n",__func__);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_GetIrqStatus
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUSts_t FMU_GetIrqStatus
(
    void
)
{
    uint8       id_cnt;
    uint32      reg0;
    uint32      reg1;
    uint32      id_status;
    FMUSts_t    status;
    uint32      check_out;

    status.id   = (FMUFaultid_t)63; //initial value
    status.cnt  = 0;
    check_out   = 0;
    reg0        = FMU_REG->mcIrqStatus.euNreg[0];
    reg1        = FMU_REG->mcIrqStatus.euNreg[1];

    for(id_cnt = 0 ; id_cnt < (uint8)64 ; id_cnt++)
    {
        id_status = reg0 & (uint32)0x1;

        if (id_status == (uint32)1)
        {
            if (check_out == (uint32)0)
            {
                status.id = (FMUFaultid_t)id_cnt;
                check_out = (uint32)1;
            }
            else
            {
                status.cnt++;
                FMU_D("[%s]status.cnt = %d\n",status.cnt);
            }
        }

        reg0 = (((reg1 & (uint32)0x1) << (uint32)31) | (reg0 >> (uint32)1));
        reg1 = (reg1 >> (uint32)1);
    }

    return status;
}


/*
***************************************************************************************************
*                                          FMU_GetFiqStatus
*
* @return
*
* Notes
*
***************************************************************************************************
*/
static FMUSts_t FMU_GetFiqStatus
(
    void
)
{
    uint8       id_cnt;
    uint32      reg0;
    uint32      reg1;
    uint32      id_status;
    FMUSts_t    status;
    uint32      check_out;

    status.id   = (FMUFaultid_t)63; //initial value
    status.cnt  = 0;
    check_out   = 0;
    reg0        = FMU_REG->mcFiqStatus.euNreg[0];
    reg1        = FMU_REG->mcFiqStatus.euNreg[1];

    for(id_cnt = 0 ; id_cnt < (uint8)64 ; id_cnt++)
    {
        id_status = reg0 & (uint32)0x1;

        if (id_status == (uint32)1)
        {
            if (check_out == (uint32)0)
            {
                status.id = (FMUFaultid_t)id_cnt;
                check_out = (uint32)1;
            }
            else
            {
                status.cnt++;
            }
        }

        reg0 = (((reg1 & (uint32)0x1) << (uint32)31) | (reg0 >> (uint32)1));
        reg1 = (reg1 >> (uint32)1);
    }

    return status;
}


/*
***************************************************************************************************
*                                          FMU_init
*
* @return
*
* Notes
*
***************************************************************************************************
*/

FMUErrTypes_t FMU_Init
(
    void
)
{
    FMUErrTypes_t ret = FMU_OK;

    FMU_SwReset();
    FMU_SetExtFault((uint8)FMU_FAULT_ACTIVE_LOW, 10, 0);

    (void)GIC_IntVectSet(GIC_FMU_IRQ, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&FMU_IrqCallBack, NULL);
    (void)GIC_IntSrcEn(GIC_FMU_IRQ);

    (void)GIC_IntVectSet(GIC_FMU_FIQ, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&FMU_FiqCallBack, NULL);
    (void)GIC_IntSrcEn(GIC_FMU_FIQ);

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_IsrHandler
*
*
* @param    FMUFaultid_t id [in]
* @param    FMUSeverityLevelType_t severity_level [in]
* @param    FMUIntFnctPtr int_fnct [in]
* @param    void * arg [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrHandler
(
    FMUFaultid_t                        uiId,
    FMUSeverityLevelType_t              uiSeverityLevel,
    FMUIntFnctPtr                       fnIntFnct,
    void *                              pArg
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_FAULT)
    {
        ret =  FMU_INVALID_ID_ERR;
    }
    else if (uiSeverityLevel > FMU_SVL_HIGH)
    {
        ret =  FMU_INVALID_SEVERITY_ERR;
    }
    else if (fnIntFnct == ((void *)0))
    {
        ret =  FMU_ISR_HANDLER_ERR;
    }
    else
    {
        FMU_IntVectTbl[uiId].severity_level = uiSeverityLevel;
        FMU_IntVectTbl[uiId].func           = fnIntFnct;
        FMU_IntVectTbl[uiId].arg            = pArg;
    }

    return ret;
}



/*
***************************************************************************************************
*                                          FMU_Set
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Set
(
    FMUFaultid_t                        uiId
)
{
    uint32          nth_reg;
    uint32          nth_bit;
    uint32          ds_reg_128[4];
    uint32          ds_reg_64[2];
    FMUErrTypes_t   ret;
    //uint8   ds_ret;//unused variable

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_FAULT)
    {
        ret = FMU_INVALID_ID_ERR;
    }
    else
    {
        // Set FMU severity level for given FMU ID
        nth_reg = ((uint32)uiId >> (uint32)4) & (uint32)0x3;                                              // 0 ~ 3
        nth_bit = (((uint32)(uiId) & (uint32)0xf) << 1);                                         // (0 ~ 15) x 2

        // for double storage
        ds_reg_128[nth_reg] = FMU_REG->mcSeverityLevel.slNreg[nth_reg];

        FMU_REG->mcCfgWrPw                       = FMU_PW;
        FMU_REG->mcSeverityLevel.slNreg[nth_reg] = ((FMU_REG->mcSeverityLevel.slNreg[nth_reg] & ((uint32)0xFFFFFFFFU - ((uint32)0x3 << nth_bit))) | ((uint32)(FMU_IntVectTbl[uiId].severity_level) << nth_bit));

        ds_reg_128[nth_reg] = ((ds_reg_128[nth_reg] & (0xFFFFFFFFU - ((uint32)0x3 << nth_bit))) | ((uint32)(FMU_IntVectTbl[uiId].severity_level) << nth_bit));

        if(FMU_OK != FMU_DsCompare(ds_reg_128[nth_reg], FMU_REG->mcSeverityLevel.slNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }

        ret = FMU_IsrMaskClear(uiId);

        // Set FMU enable for given FMU ID
        nth_reg = ((uint32)uiId >> (uint32)5) & (uint32)0x1;                                                    // 0 ~ 1
        nth_bit = ((uint32)uiId   ) & (uint32)0x1f;                                                 // 0 ~ 31

        //for double storage
        ds_reg_64[nth_reg] = FMU_REG->mcEn.euNreg[nth_reg] ;

        FMU_REG->mcCfgWrPw              = FMU_PW;
        FMU_REG->mcEn.euNreg[nth_reg]   = (FMU_REG->mcEn.euNreg[nth_reg] | ((uint32)0x1 << nth_bit));

        ds_reg_64[nth_reg] = (ds_reg_64[nth_reg] | ((uint32)0x1 << nth_bit));

        if(FMU_OK != FMU_DsCompare(ds_reg_64[nth_reg], FMU_REG->mcEn.euNreg[nth_reg]))
        {
            ret = FMU_DS_COMPARE_FAIL_ERR;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_IsrClr
*
*
* @param    uiId [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_IsrClr
(
    FMUFaultid_t                        uiId
)
{
    FMUErrTypes_t   ret;

    ret = FMU_OK;

    if(uiId > FMU_ID_FMU_FAULT)
    {
        ret = FMU_INVALID_ID_ERR;
    }
    else
    {
        ret = FMU_IsrClear(uiId);
    }

    return ret;
}

/*
***************************************************************************************************
*                                          FMU_Deinit
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
FMUErrTypes_t FMU_Deinit
(
    void
)
{
    FMUErrTypes_t ret;

    ret = FMU_OK;

    (void)GIC_IntSrcDis(GIC_FMU_IRQ);
    (void)GIC_IntSrcDis(GIC_FMU_FIQ);

    return ret;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

