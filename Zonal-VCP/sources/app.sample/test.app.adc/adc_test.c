// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : adc_test.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_TEST_APP_ADC  == 1 )

#include <adc_test.h>

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    #include "fmu.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

#include <gic.h>
#include <debug.h>
#include <bsp.h>
#include <app_cfg.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

//----------------------------------------------------------------------------------------------------------------------------
// ADC controller register structure
//----------------------------------------------------------------------------------------------------------------------------
// MC_ADC_CMD_U
#define ADC_DMA_TEST_REG(N)             ((volatile ADCDmaxPort_t  *)(0xA0800000UL + (0x10000 * N)))
#define ADC_TEST_REG(N)                 ((volatile ADCTest_t      *)((0xA0080000UL) + (N * 0x10000UL)))
#define ADC_TEST_REG_SM(N)              ((volatile ADCTestAdcSM_t *)((0xA00A0000UL) + (N * 0x10000UL)))

#define HwHCLK_MASK0                    (0xA0F20000UL)
#define HwHCLK_MASK1                    (0xA0F20004UL)
#define HwSW_RESET0                     (0xA0F2000CUL)
#define HwSW_RESET1                     (0xA0F20010UL)

#define ADC_WriteReg(a,v)               (SAL_WriteReg(v,a))
#define ADC_ReadReg(a)                  (SAL_ReadReg(a))

#define ADC_MAX_MODULE                  (2)

#define ADC_BASE_ADDR(N)                ((0xA0080000UL) + (N * 0x10000UL))
#define ADC_SMP_CMD_STS                 (0x000UL)
#define ADC_IRQ_DREQ_STS                (0x004UL)
#define ADC_CTRL_CTG                    (0x008UL)
#define ADC_TIMING_CFG                  (0x040UL)
#define ADC_SD_OUT0                     (0x080UL)
#define ADC_SD_OUT1                     (0x084UL)
#define ADC_SD_OUT2                     (0x088UL)
#define ADC_SD_OUT3                     (0x08CUL)
#define ADC_SD_OUT4                     (0x090UL)
#define ADC_SD_OUT5                     (0x094UL)
#define ADC_SD_OUT6                     (0x098UL)
#define ADC_SD_OUT7                     (0x09CUL)
#define ADC_SD_OUT8                     (0x0A0UL)
#define ADC_SD_OUT9                     (0x0A4UL)
#define ADC_SD_OUT10                    (0x0A8UL)
#define ADC_SD_OUT11                    (0x0ACUL)

#define AMM_BASE_ADDR(N)                ((0xA00A0000UL) + (N * 0x10000UL))
#define AMM_SMP_CMD                     (0x000UL)
#define AMM_CTRL                        (0x010UL)
#define AMM_TO_VAL                      (0x014UL)
#define AMM_ACK_TO_VAL                  (0x018UL)

#define ADC_DMA_RSRVD_CHANNELS          (((0x0FCUL - 0x038UL) / 4UL) + 1UL)
#define ADC_DMA_RSRVD_PERIPHERAL_ID     (((0xFDCUL - 0x140UL) / 4UL) + 1UL)

#define ADC_MAX_CHANNEL                 (16U)
#define ADC_HALF_CHANNEL                (8UL)

#define ADC_SWRESET_REG                 (HwSW_RESET1)



/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

uint32 dma_test = 0;
uint32 DMABUF = 0;;

typedef struct ADCCmd
{
    uint32                              acCmd               : 16;   // [15:00], R/W
    uint32                              acNofAin            :  4;   // [19:16], RO
    uint32                              acCnvCnt            :  5;   // [24:20], RO
    uint32                                                  :  6;   // [30:25], Reserved
    uint32                              acCnvDone           :  1;   // [   31], RO
} ADCCmd_t;

typedef union ADCCmdUnion
{
    uint32                              cuNreg;
    ADCCmd_t                            cuBreg;
} ADCCmdUnion_t;

// MC_ADC_CLR_U
typedef struct ADCTestAdcClr
{
    uint32                              acClrIrq            :  1;   // [   00], R/W
    uint32                              acClrReq            :  1;   // [   01], R/W
    uint32                              acDmaAck            :  1;   // [   02], RO
    uint32                                                  : 29;   // [31:03], Reserved
} ADCTestAdcClr_t;

typedef union ADCTestAdcClrU
{
    uint32                              nReg;
    ADCTestAdcClr_t                     bReg;
} ADCTestAdcClrU_t;

// MC_ADC_CLK_U
typedef struct ADCTestAdcclk
{
    uint32                              clDiv               :  8;   // [07:00], R/W
    uint32                              clIrqEn             :  1;   // [   08], R/W
    uint32                              clReqEn             :  1;   // [   09], R/W
    uint32                                                  : 22;   // [31:10], Reserved
} ADCTestAdcclk_t;

typedef union ADCTestAdcclkU
{
    uint32                              nReg;
    ADCTestAdcclk_t                     bReg;
} ADCTestAdcclkU_t;

// MC_ADC_TIME_U
typedef struct ADCTestAdcTime
{
    uint32                              atPrePwrOn          :  4;   // [03:00], R/W
    uint32                              atSoc               :  4;   // [07:04], R/W
    uint32                              atPstPwrOff         :  4;   // [11:08], R/W
    uint32                              atCvt               :  4;   // [15:12], R/W
    uint32                              atPreSel            :  4;   // [19:16], R/W
    uint32                              atCaptureMode       :  1;   // [   20], R/W
    uint32                                                  : 11;   // [31:21], Reserved
} ADCTestAdcTime_t;

typedef union ADCTestAdcTimeU
{
    uint32                              nReg;
    ADCTestAdcTime_t                    bReg;
} ADCTestAdcTimeU_t;


typedef struct ADCTest
{
    ADCCmdUnion_t                       atAdcCmd;
    ADCTestAdcClrU_t                    atAdcClr;
    ADCTestAdcclkU_t                    atAdcClk;
    uint32                              atReserved0x00c[13];

    ADCTestAdcTimeU_t                   atAdcTime;
    uint32                              atReserved_0x044[15];

    uint32                              atAdcAin00;
    uint32                              atAdcAin01;
    uint32                              atAdcAin02;
    uint32                              atAdcAin03;
    uint32                              atAdcAin04;
    uint32                              atAdcAin05;
    uint32                              atAdcAin06;
    uint32                              atAdcAin07;
    uint32                              atAdcAin08;
    uint32                              atAdcAin09;
    uint32                              atAdcAin10;
    uint32                              atAdcAin11;
    uint32                              atAdcAin12;
    uint32                              atAdcAin13;
    uint32                              atAdcAin14;
    uint32                              atAdcAin15;
} ADCTest_t;



typedef struct ADCSmCmd
{
    uint32                              scCmd               : 16;   // [15:00], R/W
    uint32                                                  : 15;   // [30:16], Reserved
    uint32                              scCnvDone           :  1;   // [     31], RO
} ADCSmCmd_t;

typedef union ADCSmCmdU
{
    uint32                              nReg;
    ADCSmCmd_t                          bReg;
} ADCSmCmdU_t;

// ADCTestAdcSM_t_CTRL_U
typedef struct ADCSmCtrl
{
    uint32                              smAmmEn             :  1;   // [     00], R/W
    uint32                                                  :  3;   // [03:00], Reserved
    uint32                              smErrClr            :  1;   // [     04], R/W
    uint32                                                  : 27;   // [31:05], Reserved
} ADCSmCtrl_t;

typedef union ADCSmCtrlU
{
    uint32                              nReg;
    ADCSmCtrl_t                         bReg;
} ADCSmCtrlU_t;

// MC_ADC_SM_TO_VAL_U
typedef struct ADCSmToVal
{
    uint32                              tvToVal             :  32;   // [31:00], R/W
} ADCSmToVal_t;

typedef union ADCSmToValU
{
    uint32                              nReg;
    ADCSmToVal_t                        bReg;
} ADCSmToValU_t;

// MC_ADC_SM_ACK_TO_VAL_U
typedef struct ADCSmAckToVal
{
    uint32                              atAckToVal          :  16;   // [15:00], R/W
    uint32                                                  :  16;   // [31:16], Reserved
} ADCSmAckToVal_t;

typedef union ADCSmAckToValU
{
    uint32                              nReg;
    ADCSmAckToVal_t                     bReg;
} ADCSmAckToValU_t;


typedef struct ADCTestAdcSM
{
    ADCSmCmdU_t                         smAdcSmCmd;
    uint32                              smReserved0x004[3];
    ADCSmCtrlU_t                        smAdcSmCtrl;
    ADCSmToValU_t                       smAdcSmToVal;
    ADCSmAckToValU_t                    smAdcSmAckToVal;

} ADCTestAdcSM_t;


typedef struct  ADCDmaSch
{
    uint32                              scCh0               : 1;
    uint32                              scCh1               : 1;
    uint32                              scCh2               : 1;
    uint32                              scCh3               : 1;
    uint32                              scCh4               : 1;
    uint32                              scCh5               : 1;
    uint32                              scCh6               : 1;
    uint32                              scCh7               : 1;
    uint32                              scReserved          : 24;
} ADCDmaSch_t;

typedef union ADCDmaUch
{
    uint32                              duNreg;
    ADCDmaSch_t                         duBreg;
} ADCDmaUch_t;



typedef struct  ADCDmaReq
{
    uint32                              adReq0              : 1;
    uint32                              adReq1              : 1;
    uint32                              adReq2              : 1;
    uint32                              adReq3              : 1;
    uint32                              adReq4              : 1;
    uint32                              adReq5              : 1;
    uint32                              adReq6              : 1;
    uint32                              adReq7              : 1;
    uint32                              adReq8              : 1;
    uint32                              adReq9              : 1;
    uint32                              adReq10             : 1;
    uint32                              adReq11             : 1;
    uint32                              adReq12             : 1;
    uint32                              adReq13             : 1;
    uint32                              adReq14             : 1;
    uint32                              adReq15             : 1;
    uint32                              adReserved          : 16;
} ADCDmaReq_t;

typedef union ADCDmaUreq
{
    uint32                              drNreg;
    ADCDmaReq_t                         drBreg;
} ADCDmaUreq_t;

// Configuration register structure
typedef struct  ADCDmaScfg
{
    uint32                              dcCtrlEnable        : 1;
    uint32                              dcAhb1Endian        : 1;
    uint32                              dcAhb2Endian        : 1;
    uint32                              dcreserved          : (32-1-1-1);
} ADCDmaScfg_t;

typedef union ADCDmaUcfg
{
    uint32                              dcNreg;
    ADCDmaScfg_t                        dcBreg;
} ADCDmaUcfg_t;

// Channel linked list item register structure
typedef struct ADCDmaSchLli_
{
    uint32                              slNextLliBus        : 1;
    uint32                              slreserved          : 1;
    uint32                              slNextLli           : 30;
} ADCDmaSchLli_t;

typedef union ADCDmaUlli
{
    uint32                              dlNreg;
    ADCDmaSchLli_t                      dlBreg;
} ADCDmaUlli_t;

// Channel control register structure
typedef struct  ADCDmaSchCtrl
{
    uint32                              scTransferSize      : 12;
    uint32                              scSrcBurstSize      : 3;
    uint32                              scDestBurstSize     : 3;
    uint32                              scSrcWidth          : 3;
    uint32                              scDestWidth         : 3;
    uint32                              scSrcBus            : 1;
    uint32                              scDestBus           : 1;
    uint32                              scSrcIncrement      : 1;
    uint32                              scDestIncrement     : 1;
    uint32                              scProtection        : 3;
    uint32                              scInterruptEnable   : 1;
} ADCDmaSchCtrl_t;


typedef union ADCUchCtrol
{
    uint32                              rlNreg;
    ADCDmaSchCtrl_t                     rlBreg;
} ADCUchCtrol_t;

// Channel configuration register structure
typedef struct  ADCDmaSchCfg
{
    uint32                              cfChEnable          : 1;
    uint32                              cfSrcPeri           : 5;
    uint32                              cfDstPeri           : 5;
    uint32                              cfFlowCtrl          : 3;
    uint32                              cfErrIntMask        : 1;
    uint32                              cfIntMask           : 1;
    uint32                              cfBusLock           : 1;
    uint32                              cfActive            : 1;
    uint32                              cfHalt              : 1;
    uint32                              cfReserved          : 13;
} ADCDmaSchCfg_t;

typedef union ADCUchCfg
{
    uint32                              cfNreg;
    ADCDmaSchCfg_t                      cfBreg;
} ADCUchCfg_t;

// DMA Controller - DMA Channel Registers - template structure
typedef volatile struct ADCDmaxChannel
{
    uint32                              xcSrcAddrR;
    uint32                              xcDestAddr;
    ADCDmaUlli_t                        xcLli;
    ADCUchCtrol_t                       xcCtrl;
    ADCUchCfg_t                         xcCfg;
    uint32                              xcReserved0;
    uint32                              xcReserved1;
    uint32                              xcReserved2;

} ADCDmaxChannel_t;


typedef volatile struct ADCDmaxPort
{
    ADCDmaUch_t                         poIrqStatus;                                // 0x000
    ADCDmaUch_t                         poIrqItcStatus;                             // 0x004
    ADCDmaUch_t                         poIrqItcClear;                              // 0x008
    ADCDmaUch_t                         poIrqErrStatus;                             // 0x00C
    ADCDmaUch_t                         poIrqErrClear;                              // 0x010
    ADCDmaUch_t                         poRawIrqItcStatus;                          // 0x014
    ADCDmaUch_t                         poRawIrqErrStatus;                          // 0x018
    ADCDmaUch_t                         poEnabledChannel;                           // 0x01C
    ADCDmaUreq_t                        poSwBurstReq;                               // 0x020
    ADCDmaUreq_t                        poSwSingleReq;                              // 0x024
    ADCDmaUreq_t                        poSwLastBUrstReq;                           // 0x028
    ADCDmaUreq_t                        poSwLastSingleReq;                          // 0x02C
    ADCDmaUcfg_t                        poConfig;                                   // 0x030
    ADCDmaUreq_t                        poSync;                                     // 0x034

    uint32                              poReserved0[ADC_DMA_RSRVD_CHANNELS];        // 0x038 ~ 0x0FC

    ADCDmaxChannel_t                    poDmaChannel[2];                            // 0x100 ~ 0x13C

    uint32                              poReserved1[ADC_DMA_RSRVD_PERIPHERAL_ID];   // 0x140 ~ 0xFDC

    uint32                              poPeriId0;                                  // 0xFE0
    uint32                              poPeriId1;                                  // 0xFE4
    uint32                              poPeriId2;                                  // 0xFE8
    uint32                              poPeriId3;                                  // 0xFEC
    uint32                              poCellId0;                                  // 0xFF0
    uint32                              poCellId1;                                  // 0xFF4
    uint32                              poCellId2;                                  // 0xFF8
    uint32                              poCellId3;                                  // 0xFFC
} ADCDmaxPort_t;

static uint8                            dma_irq_check = 0;
static uint8                            dma_irq_check_for = 0;

static uint8                            adc_isr_done = 0;
static uint8                            fmu_isr_done = 0;
static uint32                           sampling_clock[3] = {4,9,24};

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void wait_adc_irq_done
(
    void
);


static void wait_dma_irq
(
    void
);


#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
static void wait_fmu_irq_done
(
    void
);

static void mc_fmu_fault_isr
(
    void *arg
);
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

static void dma_adc_cfg
(
    uint32                              idx
);

// Wait adc intrrupt
static void wait_adc_irq_done(void)
{
    while(1) {
        if(adc_isr_done == (uint8)1)
            break;
    }
}



static void wait_dma_irq(void)
{
    while(1) {
        if(dma_irq_check == (uint8)0)
            break;
    }
}

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
// wait fmu isr
static void wait_fmu_irq_done(void)
{
    while(1) {
        //mcu_printf("$");
        if(fmu_isr_done == (uint8)1)
            break;
    }
}
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

static SALRetCode_t ADC_reset_set (uint32 idx)
{
    uint32 temp_data;
    /* Enable ADC SW Reset */
    temp_data = ADC_ReadReg(HwSW_RESET1);
    temp_data = temp_data & ~((uint32) 0x1 << ((uint32) 10 + idx));
    ADC_WriteReg(HwSW_RESET1 , temp_data);

    /* Disable ADC SW Reset */
    temp_data = ADC_ReadReg(HwSW_RESET1);
    temp_data = temp_data | ((uint32) 0x1 << ((uint32) 10 + idx));
    ADC_WriteReg(HwSW_RESET1 , temp_data);

    return SAL_RET_SUCCESS;
}


#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
// FMU Interrupt Handler
static void adc0_fmu_fault_isr (void *arg)
{

    //Unused Parameter
    (void)arg;

    //clr_fmu_irq(FMU_ID_ADC0);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_ADC0);

    mcu_printf("FMU IRQ For ADC0 Occurs\n");

//#ifdef DEBUG_EN
//    mc_dbg_mcu_printf("Soft fault check success for MC_CFG[%d]", fault_index);
//#endif
    fmu_isr_done = 1;
}

static void adc1_fmu_fault_isr (void *arg)
{

    //Unused Parameter
    (void)arg;

    //clr_fmu_irq(FMU_ID_ADC0);
    (void)FMU_IsrClr((FMUFaultid_t)FMU_ID_ADC1);

    mcu_printf("FMU IRQ For ADC1 Occurs\n");

//#ifdef DEBUG_EN
//    mc_dbg_mcu_printf("Soft fault check success for MC_CFG[%d]", fault_index);
//#endif
    fmu_isr_done = 1;
}
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )

static uint32 full_channel_ret (uint32 id)
{
    uint32 ret;
    if(id < ADC_MAX_CHANNEL)
    {
        ret = 1;
    }
    else
    {
        ret =0;
    }
    return ret;
}

static void check_adc0_irq(void *arg)
{
    uint32      index;
    uint32      nof_sample;
    uint32      adc_data;

    //Unused Parameter
    (void)arg;

    // Clear interrupt
    ADC_TEST_REG(0)->atAdcClr.bReg.acClrIrq = 0x1;

        // Read number of sample
    nof_sample = (uint32)(ADC_TEST_REG(0)->atAdcCmd.cuBreg.acCnvCnt);

    // ADC command display
    mcu_printf("ADC command[0] : %x nof : %d\n", ADC_TEST_REG(0)->atAdcCmd.cuBreg.acCmd,nof_sample);

    for(index=0; index<nof_sample; index++) {
        adc_data    = *(&(ADC_TEST_REG(0)->atAdcAin00) + index);
        adc_data    &= 0xffffUL;
        mcu_printf("ADC[0] Sampling = 0x%08X [dec : %d]\n", adc_data,adc_data);
    }

    adc_isr_done = 1;
}

static void check_adc1_irq(void *arg)
{
    uint32      index;
    uint32      nof_sample;
    uint32      adc_data;

    //Unused Parameter
    (void)arg;

    // Clear interrupt
    ADC_TEST_REG(1)->atAdcClr.bReg.acClrIrq = 0x1;

    // Read number of sample
    nof_sample = (uint32)(ADC_TEST_REG(1)->atAdcCmd.cuBreg.acCnvCnt);

    // ADC command display
    mcu_printf("ADC[1] command : %x nof : %d\n", ADC_TEST_REG(1)->atAdcCmd.cuBreg.acCmd,nof_sample);

    for(index=0; index<nof_sample; index++) {
        adc_data    = *(&(ADC_TEST_REG(1)->atAdcAin00) + index);
        adc_data    &= 0xffffUL;
        mcu_printf("ADC[1] Sampling = 0x%08X [dec : %d]\n", adc_data,adc_data);
    }

    adc_isr_done = 1;
}


// ADC test with DMA
static void dma_adc_cfg(uint32 idx)
{
    ADCDmaSchCtrl_t    dma_ctrl;
    ADCDmaSchCfg_t     dma_cfg;
    //uint32 addr;

    // DMA controller enable
    // AHB master-1 endianness configuration : Little-endian mode
    // AHB master-2 endianness configuration : Little-endian mode
    ADC_DMA_TEST_REG(idx)->poConfig.dcNreg = (((uint32)0<<1) | ((uint32)1<<0));

    // DMA request synchronization enable
    ADC_DMA_TEST_REG(idx)->poSync.drNreg  = (uint32)0xffff;

    mcu_printf("[%d] xcSrcAddrR[0x%08X]\n", idx, &ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcSrcAddrR);
    ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcSrcAddrR   = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
    //addr = SRAM0_2_BASE;
    mcu_printf("[%d] src addr[0x%08X] dest addr[0x%08X]\n", idx, &ADC_TEST_REG(idx)->atAdcAin00, &DMABUF);
    ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcDestAddr  = (uint32) &DMABUF;
    ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcLli.dlNreg   = (uint32)0;

    dma_ctrl.scTransferSize             = (uint32)0;
    dma_ctrl.scSrcBurstSize             = (uint32)4;//dma_BURST_32;
    dma_ctrl.scDestBurstSize            = (uint32)4;//dma_BURST_32;
    dma_ctrl.scSrcWidth                 = (uint32)2;//dma_WIDTH_32_BIT;
    dma_ctrl.scDestWidth                = (uint32)2;//dma_WIDTH_32_BIT;
    dma_ctrl.scSrcBus                   = (uint32)0;
    dma_ctrl.scDestBus                  = (uint32)1;
    dma_ctrl.scSrcIncrement             = (uint32)1;
    dma_ctrl.scDestIncrement            = (uint32)1;
    //dma_ctrl.PROTECTION               = ((uint32)dma_PROT_NON_CACHEABLE | (uint32)dma_PROT_NON_BUFFERABLE | (uint32)dma_PROT_USER);
    dma_ctrl.scProtection               = ((uint32)0x00 | (uint32)0x00 | (uint32)0x00);
    dma_ctrl.scInterruptEnable          = (uint32)1;

    dma_cfg.cfChEnable                  = (uint32)0;
    dma_cfg.cfSrcPeri                   = (uint32)8 + idx;
    //dma_cfg.SRC_PERIPHERAL            = (uint32)dma_REQUEST_INDEX_12;     //tcc803x
    dma_cfg.cfDstPeri                   = (uint32)0;
    dma_cfg.cfFlowCtrl                  = (uint32)2;//dma_PERIPHERAL_TO_MEM_DMA_CTRL;
    dma_cfg.cfErrIntMask                = (uint32)0;
    dma_cfg.cfIntMask                   = (uint32)1;
    dma_cfg.cfBusLock                   = (uint32)0;
    dma_cfg.cfActive                    = (uint32)0;
    dma_cfg.cfHalt                      = (uint32)0;
    dma_cfg.cfReserved                  = (uint32)0;

    ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCtrl.rlBreg  = dma_ctrl;
    ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCfg.cfBreg   = dma_cfg;
}


//----------------------------------------------------------------------------------------------------------------------------
// Sub Function
//----------------------------------------------------------------------------------------------------------------------------
// ADC interrupt handler
static void check_dma0_irq(void *arg)
{
    uint32      index;
    uint32      nof_sample;
    //uint32      adc_data;
    //uint32      read_addr;
    //uint32      wr_addr;

    (void)arg;

//will be fixed
    // Clear interrupt
    ADC_DMA_TEST_REG(0)->poIrqItcClear.duNreg = ADC_DMA_TEST_REG(0)->poIrqItcStatus.duNreg;

    // Read number of sample
    nof_sample = ADC_TEST_REG(0)->atAdcCmd.cuBreg.acCnvCnt;

    // ADC command display
    //mcu_printf("DMA ADC command : %x", ADC_TEST_REG(idx)->atAdcCmd.cuBreg.acCmd);

    for(index=0; index<nof_sample; index++) {
        //read_addr   = SRAM0_2_BASE + (index<<(uint32)2);
        //adc_data    = ADC_ReadReg(read_addr);

        //adc_data    &= (uint32)0xffff;


        mcu_printf("ADC[0] DMA Sampling = 0x%08X [dec : %d]\n", DMABUF & 0xFFFF, DMABUF & 0xFFFF);
        DMABUF = 0;

        //wr_addr = (SRAM0_2_BASE + (index<<(uint32)2));
        //ADC_WriteReg(wr_addr, 0xffffffffUL);
    }

    dma_irq_check_for--;
    dma_irq_check = 0;
}

static void check_dma1_irq(void *arg)
{
    uint32      index;
    uint32      nof_sample;
    //uint32      adc_data;
    //uint32      read_addr;
    //uint32      wr_addr;

    (void)arg;

//will be fixed
    // Clear interrupt
    ADC_DMA_TEST_REG(1)->poIrqItcClear.duNreg = ADC_DMA_TEST_REG(1)->poIrqItcStatus.duNreg;

    // Read number of sample
    nof_sample = ADC_TEST_REG(1)->atAdcCmd.cuBreg.acCnvCnt;

    // ADC command display
    //mcu_printf("DMA ADC command : %x", ADC_TEST_REG(idx)->atAdcCmd.cuBreg.acCmd);

    for(index=0; index<nof_sample; index++) {
        //read_addr   = SRAM0_2_BASE + (index<<(uint32)2);
        //adc_data    = ADC_ReadReg(read_addr);

        //adc_data    &= (uint32)0xffff;


        mcu_printf("ADC[1] DMA Sampling = 0x%08X [dec : %d]\n", DMABUF & 0xFFFF ,DMABUF & 0xFFFF);
        DMABUF = 0;


        //wr_addr = (SRAM0_2_BASE + (index<<(uint32)2));
        //ADC_WriteReg(wr_addr, 0xffffffffUL);
    }

    dma_irq_check_for--;
    dma_irq_check = 0;
}

static int32 ADC_IpBusClocking(void) /* based on SoC test bench */
{
    uint32 idx;
    uint32 temp_data;
    uint32 base_addr;
    mcu_printf("  IP_BusClocking Test Start\n");

    (void)SAL_TaskSleep(5);

    for (idx = 0; idx < ADC_MAX_MODULE; idx++)
    {
        base_addr =  ADC_BASE_ADDR(idx) + ADC_CTRL_CTG;

        ADC_WriteReg(base_addr, 0x55555555UL);
        mcu_printf("ADC[%d] CTRL_CFG reg write 0x%08X\n", idx, ADC_ReadReg(base_addr));

        /* Disable ADC bus clock */
        temp_data = ADC_ReadReg(HwHCLK_MASK1);
        temp_data = temp_data & ~((uint32) 0x1 << ((uint32) 10 + idx));
        ADC_WriteReg(HwHCLK_MASK1,temp_data);

        (void)SAL_TaskSleep(5);
        mcu_printf("ADC[%d] Disable bus clock \n", idx);

        ADC_WriteReg(base_addr, 0xffffffffUL);
        mcu_printf("ADC[%d] CTRL_CFG reg write 0xffffffff reg[0x%08X]\n", idx, ADC_ReadReg(base_addr));

        /* Enable ADC bus clock */
        temp_data = ADC_ReadReg(HwHCLK_MASK1);
        temp_data = temp_data | ((uint32) 0x1 << ((uint32) 10 + idx));
        ADC_WriteReg(HwHCLK_MASK1,temp_data);

        (void)SAL_TaskSleep(5);
        mcu_printf("ADC[%d] Enable bus clock \n", idx);
        ADC_WriteReg(base_addr, 0xffffffffUL);

        mcu_printf("ADC[%d] CTRL_CFG reg write 0xffffffff reg[0x%08X]\n", idx, ADC_ReadReg(base_addr));
    }

    mcu_printf("  IP_BusClocking Test End\n");

    return 0;

}

static void ADC_IpSwReset(void) /* based on SoC test bench */
{
    uint32 idx;
    uint32 temp_data;
    mcu_printf("  IP_SWReset Test Start \n");

    for (idx = 0; idx < ADC_MAX_MODULE; idx++)
    {
        ADC_WriteReg(ADC_BASE_ADDR(idx) + ADC_SMP_CMD_STS, 0x00003000UL);
        ADC_WriteReg(ADC_BASE_ADDR(idx) + ADC_IRQ_DREQ_STS, 0xFFFFFFFFUL);
        ADC_WriteReg(ADC_BASE_ADDR(idx) + ADC_CTRL_CTG, 0xFFFFFFFFUL);
        ADC_WriteReg(ADC_BASE_ADDR(idx) + ADC_TIMING_CFG, 0x00000000UL);

        mcu_printf("ADC[%d] ADC_SMP_CMD_STS  [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_SMP_CMD_STS));
        mcu_printf("ADC[%d] ADC_IRQ_DREQ_STS [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_IRQ_DREQ_STS));
        mcu_printf("ADC[%d] ADC_CTRL_CTG     [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_CTRL_CTG));
        mcu_printf("ADC[%d] ADC_TIMING_CFG   [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_TIMING_CFG));

        (void)SAL_TaskSleep(5);

        /* Enable ADC SW Reset */
        temp_data = ADC_ReadReg(HwSW_RESET1);
        temp_data = temp_data & ~((uint32) 0x1 << ((uint32) 10 + idx));
        ADC_WriteReg(HwSW_RESET1 , temp_data);

        /* Disable ADC SW Reset */
        temp_data = ADC_ReadReg(HwSW_RESET1);
        temp_data = temp_data | ((uint32) 0x1 << ((uint32) 10 + idx));
        ADC_WriteReg(HwSW_RESET1 , temp_data);

        mcu_printf("ADC[%d]  ------------------- ADC RESET ---------------- \n", idx);

        mcu_printf("ADC[%d] ADC_SMP_CMD_STS  [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_SMP_CMD_STS));
        mcu_printf("ADC[%d] ADC_IRQ_DREQ_STS [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_IRQ_DREQ_STS));
        mcu_printf("ADC[%d] ADC_CTRL_CTG     [0x%08X] \n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_CTRL_CTG));
        mcu_printf("ADC[%d] ADC_TIMING_CFG   [0x%08X] \n\n", idx, ADC_ReadReg(ADC_BASE_ADDR(idx) + ADC_TIMING_CFG));
    }

    mcu_printf("  IP_SWReset Test End \n\n");

}

static void ADC_IpTestInit(void) /* based on SoC test bench */
{

    (void)GIC_IntVectSet(GIC_ADC0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_BOTH, (GICIsrFunc)&check_adc0_irq, (void *) 0);
    (void)GIC_IntSrcEn(GIC_ADC0);

    (void)GIC_IntVectSet(GIC_ADC1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_BOTH, (GICIsrFunc)&check_adc1_irq, (void *) 0);
    (void)GIC_IntSrcEn(GIC_ADC1);

    //uint8 bool_ret;
    (void)GIC_IntVectSet(GIC_DMA0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_BOTH, (GICIsrFunc)&check_dma0_irq, (void *) 0);
    (void)GIC_IntSrcEn(GIC_DMA0);

    (void)GIC_IntVectSet(GIC_DMA1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_BOTH, (GICIsrFunc)&check_dma1_irq, (void *) 0);
    (void)GIC_IntSrcEn(GIC_DMA1);

#if ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
    (void)FMU_IsrHandler(FMU_ID_ADC0, FMU_SVL_HIGH, (FMUIntFnctPtr)&adc0_fmu_fault_isr, NULL);
    (void)FMU_Set(FMU_ID_ADC0);

    (void)FMU_IsrHandler(FMU_ID_ADC1, FMU_SVL_HIGH, (FMUIntFnctPtr)&adc1_fmu_fault_isr, NULL);
    (void)FMU_Set(FMU_ID_ADC1);
#endif  // ( MCU_BSP_SUPPORT_DRIVER_FMU == 1 )
}

static void ADC_IpDmaRead(void) /* based on SoC test bench */
{
    uint32 idx;
    //sint32 i;
    uint32 index;
    uint32 uiK1;
    //uint32 uiK2;
    //uint32 uiK3;
    //uint32 uiK4;
    //uint32 uiK5;
    //uint32 uiK6;
    //uint32 uiK7;
    const uint32  uiDelaycnt = (5000000UL);

    dma_test = 1;
    //mcu_printf("  IP_Test6_DMARead  capture mode [%d]\n",cap_mode);

    //peri_clk_cfg(MC_PCLK_ADC, MC_PCLK_PLL0_FOUT, (6-1));

    for (idx = 0; idx < ADC_MAX_MODULE; idx++)
    {
        dma_adc_cfg(idx);
        // Set capture mode
        //ADC_TEST_REG(idx)->atAdcTime.nReg = ((cap_mode&0x1)<<20);   //hidden by SOC Team

        //----------------------------------------------------------------------------------------------------------------------------
        // 1. Fixed sampling clock test
        //----------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------
        // ADC controller interrupt enable
        //--------------------------------------------------------------
        // - ADC clock divider = 4
        //   * Periclock         = 200MHz
        //   * Sampling clock    =  20MHz
        //   * ADC clock divider = (200MHz/20)/2 - 1 = 4
        // - Interrupt enable
        //--------------------------------------------------------------
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv   = ((120/20)/2) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //--------------------------------------------------------------
        // 1.1. Sampling for each analog input
        //--------------------------------------------------------------
        dma_irq_check_for                                                   = ADC_MAX_CHANNEL;
        for(index=0; index < (uint32)16; index++) {
            ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcSrcAddrR                    = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
            ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcDestAddr                    = (uint32) &DMABUF;
            ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCtrl.rlBreg.scTransferSize  = (uint32)1;
            ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCfg.cfBreg.cfChEnable       = (uint32)1;
            ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = ((uint32)1 << index);

            for (uiK1=0 ; (uiK1 < uiDelaycnt) ; uiK1++)
            {
                if(dma_irq_check_for < (uint8)((ADC_MAX_CHANNEL - index) & 0xFFUL))
                {
                    break;
                }
                BSP_NOP_DELAY();
            }

            //wait_dma_irq();
        }

        //--------------------------------------------------------------
        // 1.2. Sampling for all analog inputs
        //--------------------------------------------------------------
        //if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //{
        //    dma_irq_check                                                   = 1;
        //}

        //ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcSrcAddrR                        = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcDestAddr                        = (uint32) &DMABUF;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCtrl.rlBreg.scTransferSize      = (uint32)16;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCfg.cfBreg.cfChEnable           = (uint32)1;
        //ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0xffff;

        //for (uiK2=0 ; (uiK2 < uiDelaycnt) ; uiK2++)
        //{
        //    BSP_NOP_DELAY();
        //    if(dma_irq_check == 0UL)
        //    {
        //        break;
        //    }

        //}
        //wait_dma_irq();

        //--------------------------------------------------------------
        // 1.3. Sampling for even analog inputs
        //--------------------------------------------------------------
        //if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //{
        //    dma_irq_check                                                   = 1;
        //}

        //ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcSrcAddrR                        = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcDestAddr                        = (uint32) &DMABUF;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCtrl.rlBreg.scTransferSize      = (uint32)8;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[idx].xcCfg.cfBreg.cfChEnable           = (uint32)1;
        //ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0x5555;
        //for (uiK3=0 ; (uiK3 < uiDelaycnt) ; uiK3++)
        //{
        //    BSP_NOP_DELAY();
        //    if(dma_irq_check == 0UL)
        //    {
        //        break;
        //    }

        //}
        //wait_dma_irq();

        //--------------------------------------------------------------
        // 1.4. Sampling for odd analog inputs
        //--------------------------------------------------------------
        //if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //{
        //    dma_irq_check                                                   = 1;
        //}

        //ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcSrcAddrR                        = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcDestAddr                        = (uint32) &DMABUF;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCtrl.rlBreg.scTransferSize      = (uint32)8;
        //ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCfg.cfBreg.cfChEnable           = (uint32)1;
        //ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0xaaaa;
        //for (uiK4=0 ; (uiK4 < uiDelaycnt) ; uiK4++)
        //{
        //    BSP_NOP_DELAY();
        //    if(dma_irq_check == 0UL)
        //    {
        //        break;
        //    }

        //}
        //wait_dma_irq();

        //----------------------------------------------------------------------------------------------------------------------------
        // 2. Variable sampling clock test
        //----------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------
        // ADC controller interrupt enable
        //--------------------------------------------------------------
        // - ADC clock divider = 4
        //   * Periclock                  = 200MHz
        //   * Sampling clock             =  10MHz, 5MHz, 2.5MHz
        //   * ADC clock divider(10.0MHz) = (200MHz/    10)/2 - 1 =  9
        //   * ADC clock divider( 5.0MHz) = (200MHz/     5)/2 - 1 = 19
        //   * ADC clock divider( 2.5MHz) = (200MHz/2.5Mhz)/2 - 1 = 39
        // - Interrupt enable
        //--------------------------------------------------------------
        //for(i=2; i>=0; i--) {

        //    mcu_printf("Clock DIV[%d]\n",(((uint32)40 >> (uint32)i)-(uint32)1));
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv   = (((uint32)40 >> (uint32)i)-(uint32)1);

        //    //--------------------------------------------------------------
        //    // 2.1. Sampling for all analog inputs
        //    //--------------------------------------------------------------
        //    if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //    {
        //        dma_irq_check                                                   = 1;
        //    }

        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcSrcAddrR                    = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcDestAddr                    = (uint32) &DMABUF;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCtrl.rlBreg.scTransferSize  = (uint32)16;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCfg.cfBreg.cfChEnable       = (uint32)1;
        //    ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0xffff;
        //    for (uiK5=0 ; (uiK5 < uiDelaycnt) ; uiK5++)
        //    {
        //        BSP_NOP_DELAY();
        //        if(dma_irq_check == 0UL)
        //        {
        //            break;
        //        }

        //    }
        //    //wait_dma_irq();

        //    //--------------------------------------------------------------
        //    // 2.2. Sampling for even analog inputs
        //    //--------------------------------------------------------------
        //    if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //    {
        //        dma_irq_check                                                   = 1;
        //    }

        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcSrcAddrR                    = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcDestAddr                    = (uint32) &DMABUF;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCtrl.rlBreg.scTransferSize  = (uint32)8;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCfg.cfBreg.cfChEnable       = (uint32)1;
        //    ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0x5555;
        //    for (uiK6=0 ; (uiK6 < uiDelaycnt) ; uiK6++)
        //    {
        //        BSP_NOP_DELAY();
        //        if(dma_irq_check == 0UL)
        //        {
        //            break;
        //        }

        //    }
        //    //wait_dma_irq();


        //    //--------------------------------------------------------------
        //    // 2.3. Sampling for odd analog inputs
        //    //--------------------------------------------------------------
        //    if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        //    {
        //        dma_irq_check                                                   = 1;
        //    }
        //
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((200/20)/2) - 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 1;
        //    ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcSrcAddrR                    = (uint32)(&ADC_TEST_REG(idx)->atAdcAin00);
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcDestAddr                    = (uint32) &DMABUF;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCtrl.rlBreg.scTransferSize  = (uint32)8;
        //    ADC_DMA_TEST_REG(idx)->poDmaChannel[0].xcCfg.cfBreg.cfChEnable       = (uint32)1;
        //    ADC_TEST_REG(idx)->atAdcCmd.cuNreg                                   = (uint32)0xaaaa;
        //    for (uiK7=0 ; (uiK7 < uiDelaycnt) ; uiK7++)
        //    {
        //        BSP_NOP_DELAY();
        //        if(dma_irq_check == 1UL)
        //        {
        //            break;
        //        }

        //    }
        //    //wait_dma_irq();

        //}

        ADC_TEST_REG(idx)->atAdcClk.bReg.clReqEn = 0;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 0;
    }
    dma_test = 0;
}

static void ADC_IpSingleMultiClock(void) /* based on SoC test bench */
{
    uint32 idx;
    uint32 index;
    int32 i;
    uint32 uiK1;
    uint32 uiK2;
    uint32 uiK3;
    uint32 uiK4;
    uint32 uiK5;
    uint32 uiK6;
    uint32 uiK7;
    const uint32  uiDelaycnt = (5000000UL);
    for (idx = 0; idx < ADC_MAX_MODULE; idx++)
    {
        //uint32 *dev_read_buf = NULL;
        //mcu_printf("  IP_Single_Multi_Clock  capture mode [%d]\n",cap_mode);
        //ADC_TEST_REG(idx)->atAdcTime.nReg = (((cap_mode&0x1)<<20) | (0x1<<16) | (0xb<<12) | (0x0<<8) | (0x5<<4) | (0x7<<0));
        //----------------------------------------------------------------------------------------------------------------------------
        // 1. Fixed sampling clock test
        //----------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------
        // ADC controller interrupt enable
        //--------------------------------------------------------------
        // - ADC clock divider = 4
        //   * Periclock         = 200MHz
        //   * Sampling clock    =  20MHz
        //   * ADC clock divider = (200MHz/20)/2 - 1 = 4
        // - Interrupt enable
        //--------------------------------------------------------------
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((120/20)/2) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        //--------------------------------------------------------------
        // 1.1. Sampling for each analog input
        //--------------------------------------------------------------
#if 1
        for(index=0; index<(uint32)16; index++) {
            for (uiK1=0 ; (uiK1 < uiDelaycnt) ; uiK1++)
            {
                BSP_NOP_DELAY();
                if(adc_isr_done == 1UL)
                {
                    break;
                }

            }
            //wait_adc_irq_done();


        }
#else
        dev_read_buf = mt_adc_auto_scan();

        for (i=0; i<12; i++)
        {
            mcu_printf("conversion value = %x[%d]\n",(dev_read_buf[i]&0xfff), i);
        }
        //Note. chage isr handler function in ADCTestApp.c
#endif



        //--------------------------------------------------------------
        // 1.2. Sampling for all analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
             adc_isr_done = 0;
        }
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((120/20)/2) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;

        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0xffff;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0xffff;
        for (uiK2=0 ; (uiK2 < uiDelaycnt) ; uiK2++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();



        //--------------------------------------------------------------
        // 1.3. Sampling for even analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
             adc_isr_done = 0;
        }
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((120/20)/2) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;
        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0x5555;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0x5555;
        for (uiK3=0 ; (uiK3 < uiDelaycnt) ; uiK3++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();



        //--------------------------------------------------------------
        // 1.4. Sampling for odd analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
             adc_isr_done = 0;
        }
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = ((120/20)/2) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 1;
        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0xaaaa;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0xaaaa;
        for (uiK4=0 ; (uiK4 < uiDelaycnt) ; uiK4++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();


        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 0;

        //----------------------------------------------------------------------------------------------------------------------------
        // 2. Variable sampling clock test
        //----------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------
        // ADC controller interrupt enable
        //--------------------------------------------------------------
        // - ADC clock divider = 4
        //   * Periclock                  = 200MHz
        //   * Sampling clock             =  10MHz, 5MHz, 2.5MHz
        //   * ADC clock divider(10.0MHz) = (200MHz/    10)/2 - 1 =  9
        //   * ADC clock divider( 5.0MHz) = (200MHz/     5)/2 - 1 = 19
        //   * ADC clock divider( 2.5MHz) = (200MHz/2.5Mhz)/2 - 1 = 39
        // - Interrupt enable
        //--------------------------------------------------------------
        for(i=2; i>=0; i--) {

            mcu_printf("ADC[%d] \n\n  !!!!!!! Clock [%d]\n\n",idx, sampling_clock[i]);

            if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
            {
                 adc_isr_done = 0;
            }
            ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = sampling_clock[i];
            ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;

            ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0xffff;
            ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0xffff;
            for (uiK5=0 ; (uiK5 < uiDelaycnt) ; uiK5++)
            {
                BSP_NOP_DELAY();
                if(adc_isr_done == 1UL)
                {
                    break;
                }

            }
            //wait_adc_irq_done();


            //--------------------------------------------------------------
            // 2.2. Sampling for even analog inputs
            //--------------------------------------------------------------
            if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
            {
                 adc_isr_done = 0;
            }

            ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = sampling_clock[i];
            ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;

            ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0x5555;
            ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0x5555;
            for (uiK6=0 ; (uiK6 < uiDelaycnt) ; uiK6++)
            {
                BSP_NOP_DELAY();
                if(adc_isr_done == 1UL)
                {
                    break;
                }

            }
            //wait_adc_irq_done();


            //--------------------------------------------------------------
            // 2.3. Sampling for odd analog inputs
            //--------------------------------------------------------------
            if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
            {
                 adc_isr_done = 0;
            }

            ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv    = sampling_clock[i];
            ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;

            ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = (uint32)0xaaaa;
            ADC_TEST_REG(idx)->atAdcCmd.cuNreg = (uint32)0xaaaa;
            for (uiK7=0 ; (uiK7 < uiDelaycnt) ; uiK7++)
            {
                BSP_NOP_DELAY();
                if(adc_isr_done == 1UL)
                {
                    break;
                }

            }
            //wait_adc_irq_done();


        }

        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;
    }
}

static void ADC_SmTest(void) /* based on SoC test bench */
{
    uint32 idx;
    uint32 index;
    uint32 rdata;
    //uint32  uiK1,uiK2,uiK3,uiK4,uiK5,uiK6,uiK7,uiK8;
    uint32 uiK1;
    uint32 uiK2;
    uint32 uiK3;
    uint32 uiK4;
    uint32 uiK5;
    uint32 uiK6;
    const uint32  uiDelaycnt = (5000000UL);
    mcu_printf("  SM_Test\n");

    for(idx=0; idx < ADC_MAX_MODULE; idx++) {
        // Set capture mode
        ADC_TEST_REG(idx)->atAdcTime.nReg = ((((uint32)1&(uint32)0x1)<<(uint32)20) | ((uint32)0x1<<(uint32)16) | ((uint32)0xb<<(uint32)12) | ((uint32)0x0<<(uint32)8) | ((uint32)0x5<<(uint32)4) | ((uint32)0x7<<(uint32)0));

        // ADC SM enable
        ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smAmmEn = (uint32)0x1;
        //----------------------------------------------------------------------------------------------------------------------------
        // 1. Fixed sampling clock test
        //----------------------------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------
        // ADC controller interrupt enable
        //--------------------------------------------------------------
        // - ADC clock divider = 4
        //   * Periclock         = 200MHz
        //   * Sampling clock    =  20MHz
        //   * ADC clock divider = (200MHz/20)/2 - 1 = 4
        // - Interrupt enable
        //--------------------------------------------------------------
        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv   = (((120/20)/2)) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;


        ADC_TEST_REG_SM(idx)->smAdcSmToVal.nReg = (uint32) 0x40;


        for(index=0; index< (uint32)16; index++) {
            //--------------------------------------------------------------
            // 1.1. Sampling for each analog input
            //--------------------------------------------------------------
        #if 1
            //if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
            //{
            //    adc_isr_done = 0;
            //    fmu_isr_done = 0;
            //}

            if( index < (uint32)15)
            {
                ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = ((uint32)1 << (index+(uint32)1));
                ADC_TEST_REG(idx)->atAdcCmd.cuNreg = ((uint32)1 << index);

                for (uiK1=0 ; (uiK1 < uiDelaycnt) ; uiK1++)
                {
                    BSP_NOP_DELAY();
                    if(adc_isr_done == 1UL)
                    {
                        break;
                    }

                }
                //wait_adc_irq_done();

                for (uiK2=0 ; (uiK2 < uiDelaycnt) ; uiK2++)
                {
                    BSP_NOP_DELAY();
                    if(fmu_isr_done == 1UL)
                    {
                        break;
                    }

                }
                //wait_fmu_irq_done();


                rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
                while (rdata == 0x10UL) {      // check error generated
                    rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
                }

                ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr = 0x1U;
            }
        }
        #endif

        //--------------------------------------------------------------
        // 1.2. Sampling for all analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
            adc_isr_done = 0;
            fmu_isr_done = 0;
        }

        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv   = (((120/20)/2)) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;

        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = 0x1111UL;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = 0xffffUL;
        for (uiK3=0 ; (uiK3 < uiDelaycnt) ; uiK3++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();

        for (uiK4=0 ; (uiK4 < uiDelaycnt) ; uiK4++)
        {
            BSP_NOP_DELAY();
            if(fmu_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_fmu_irq_done();



        rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        while (rdata == 0x10UL) {      // check error generated
            mcu_printf("!");
            rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        }
        ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr = 0x1U;

        //--------------------------------------------------------------
        // 1.3. Sampling for even analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
            adc_isr_done = 0;
            fmu_isr_done = 0;
        }

        ADC_TEST_REG(idx)->atAdcClk.bReg.clDiv   = (((120/20)/2)) - 1;
        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = (uint32)1;

        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = 0x5555UL;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = 0xaaaaUL;
        for (uiK5=0 ; (uiK5 < uiDelaycnt) ; uiK5++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();

        for (uiK6=0 ; (uiK6 < uiDelaycnt) ; uiK6++)
        {
            BSP_NOP_DELAY();
            if(fmu_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_fmu_irq_done();



        rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        while (rdata == 0x10UL) {      // check error generated
            mcu_printf("!");
            rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        }
        ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr = 0x1U;
#if     0
        //--------------------------------------------------------------
        // 1.4. Sampling for odd analog inputs
        //--------------------------------------------------------------
        if(ADC_reset_set(idx) == SAL_RET_SUCCESS)
        {
            adc_isr_done = 0;
            fmu_isr_done = 0;
        }

        ADC_TEST_REG_SM(idx)->smAdcSmCmd.nReg = 0xaaaaUL;
        ADC_TEST_REG(idx)->atAdcCmd.cuNreg = 0x5555UL;
        for (uiK7=0 ; (uiK7 < uiDelaycnt) ; uiK7++)
        {
            BSP_NOP_DELAY();
            if(adc_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_adc_irq_done();

        for (uiK8=0 ; (uiK8 < uiDelaycnt) ; uiK8++)
        {
            BSP_NOP_DELAY();
            if(fmu_isr_done == 1UL)
            {
                break;
            }

        }
        //wait_fmu_irq_done();



        rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        while (rdata == 0x10UL) {      // check error generated
            mcu_printf("!");
            rdata = ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr & 0x10UL;
        }
        ADC_TEST_REG_SM(idx)->smAdcSmCtrl.bReg.smErrClr = 0x1UL;
#endif

        ADC_TEST_REG(idx)->atAdcClk.bReg.clIrqEn = 0U;
    }
}

void ADC_StartAdcTest(int32 ucMode)
{

    ADC_IpTestInit();

    switch (ucMode)
    {

        case 1 :
        {
            (void)ADC_IpBusClocking();
            break;
        }

        case 2 :
        {
            ADC_IpSingleMultiClock();
            break;
        }


        case 3:
        {
            ADC_IpSwReset();
            break;
        }

        case 4:
        {
            ADC_IpDmaRead();
            break;
        }

        case 10:
        {
            ADC_SmTest();
            break;
        }
        default:
        {
            mcu_printf("invalid test mode param\n");
            break;
        }
    }
}

#endif  // ( MCU_BSP_SUPPORT_TEST_APP_ADC == 1 )

