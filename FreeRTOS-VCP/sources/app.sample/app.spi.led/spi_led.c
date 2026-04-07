// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : spi_led.c
*
*   Copyright (c) Telechips Inc.
*
*   Description : Demo application for controlling of "TLC5925 LED Sink Driver" using GPSB driver on the tcc70xx evb.
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

#include <bsp.h>

#include "gpsb.h"
#include "gpio.h"
#include "spi_led.h"
#include "debug.h"
#include "app_cfg.h"
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

//#define SPI_LED_DEBUG
#ifdef SPI_LED_DEBUG
#define SPI_LED_DBG(fmt, args...)        {LOGD(DBG_TAG_GPSB, fmt, ## args)}
#else
#define SPI_LED_DBG(fmt, args...)        {}
#endif
#define SPI_LED_ERR(fmt, args...)        {LOGE(DBG_TAG_GPSB, fmt, ## args)}

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/
static uint32 ledStatus = 0;


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static void SPILED_Task
(
    void * pArg
);

static void SPILED_Init
(
    void
);

static void SPILED_On
(
   uint16 *pattern
);

static void SPILED_Off
(
    void
);


static void SPILED_Task(void * pArg)
{
    uint16 led_pattern = 0xAAAAU;

    SPILED_Init();
    for(;;)
    {
        if ( ledStatus == 1 )
        {
            led_pattern ^= 0xFFFFU;
            SPILED_On(&led_pattern);
            (void)SAL_TaskSleep(500);
        }
        else if ( ledStatus == 2 )
        {
            SPILED_Off();
            ledStatus = 0;
        }

        (void)SAL_TaskSleep(10);
    }
}

static void SPILED_Init
(
    void
)
{
    uint8                               ucMstCh = 1;
    uint32                              cs_le;
    uint32                              led_oe;
    uint32                              led_pctl;
    uint8                               bpw;
    uint32                              speed;

    GPSBOpenParam_t                     OpenParam;

    OpenParam.uiSdo         = GPIO_GPC(14UL);
    OpenParam.uiSdi         = GPIO_GPC(15UL);
    OpenParam.uiSclk        = GPIO_GPC(12UL);
    OpenParam.pDmaAddrTx    = NULL_PTR;
    OpenParam.pDmaAddrRx    = NULL_PTR;
    OpenParam.uiDmaBufSize  = 0;
    OpenParam.fbCallback    = NULL_PTR;
    OpenParam.pArg          = NULL_PTR;
    OpenParam.uiIsSlave     = GPSB_MASTER_MODE;

    cs_le       = GPIO_GPC(13UL);
    led_oe      = GPIO_GPA(24UL);
    led_pctl    = GPIO_GPC(11UL);

    /* Configure GPSB master */
    speed     = 30UL*1000UL*1000UL;
    bpw       = (uint8)16UL;

    //(void)GPSB_Open((uint32)ucMstCh, sdo, sdi, sclk, NULL, NULL, 0, NULL_PTR, NULL_PTR, 0);
    (void)GPSB_Open((uint32)ucMstCh, OpenParam);
    (void)GPSB_SetSpeed(ucMstCh, speed);
    (void)GPSB_SetBpw(ucMstCh, bpw);
    (void)GPSB_CsAlloc(ucMstCh, cs_le);

    // LED_PWR_CTL
    (void)GPIO_Config(led_pctl, (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    GPIO_Set(led_pctl, 1U);

    // OE
    (void)GPIO_Config(led_oe, (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    GPIO_Set(led_oe, 1U);
    //(void)GPSB_CsActivate((uint32)mst_ch, gpio_cs, cs_high);
    //(void)GPSB_CsDeactivate((uint32)mst_ch, gpio_cs, cs_high);
}

static void SPILED_On
(
    uint16 *pattern
)
{
    uint8                               ucMstCh = 1;
    uint32                              cs_high;
    uint32                              cs_le;

    uint32                              led_oe;
    uint32                                 xfer_mode;

    cs_high = 0;
    cs_le          = GPIO_GPC(13UL);
    led_oe     = GPIO_GPA(24UL);

    xfer_mode = GPSB_XFER_MODE_PIO | GPSB_XFER_MODE_WITHOUT_INTERRUPT;

    (void)GPSB_CsActivate((uint32)ucMstCh, cs_le, cs_high);
    GPIO_Set(led_oe, 1U);

    (void)GPSB_Xfer((uint32)ucMstCh, (const uint8 *)pattern, NULL_PTR, 2, (uint32)xfer_mode);

    (void)GPSB_CsDeactivate((uint32)ucMstCh, cs_le, cs_high);
    BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();
    (void)GPSB_CsActivate((uint32)ucMstCh, cs_le, cs_high);

    GPIO_Set(led_oe, 0U);
    BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();
    BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();BSP_NOP_DELAY();
}

static void SPILED_Off
(
    void
)
{
    uint32                              led_oe;

    led_oe     = GPIO_GPA(24UL);

    GPIO_Set(led_oe, 1U);
}

void SPILED_StatusOn(void)
{
    ledStatus = 1;
}
void SPILED_StatusOff(void)
{
    ledStatus = 2;
}

void SPILED_CreateAppTask(void)
{
    //int32 err; // CS :
    static uint32 SpiLedDemoTaskID;
    static uint32 SpiLedDemoTaskStk[ACFG_TASK_NORMAL_STK_SIZE];

    (void)SAL_TaskCreate(&SpiLedDemoTaskID,
                         (const uint8 *)"SPILED",
                         (SALTaskFunc)&SPILED_Task,
                         (void * const)&SpiLedDemoTaskStk[0],
                         ACFG_TASK_NORMAL_STK_SIZE,
                         SAL_PRIO_SPILED_DEMO, NULL_PTR);
}

#endif  // ( MCU_BSP_SUPPORT_APP_SPI_LED == 1 )

