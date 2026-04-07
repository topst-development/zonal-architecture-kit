// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : gpio.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/

#if ( MCU_BSP_SUPPORT_DRIVER_GPIO == 1 )

#include <gpio.h>
#include <reg_phys.h>
#include <bsp.h>
#include "debug.h"
#include <gic.h>

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/

#ifndef ENABLE
#   define GPIO_ENABLE                  (1)
#endif

#ifndef DISABLE
#   define GPIO_DISABLE                 (0)
#endif

#define GPIO_PMGPIO_BASE                (MCU_BSP_PMIO_BASE)

#define GPIO_REG_BASE(x)                (MCU_BSP_GPIO_BASE + ((((x) & GPIO_PORT_MASK) >> (uint32)GPIO_PORT_SHIFT) * 0x40UL))


#define GPIO_IS_GPIOK(x)                (boolean)((((x) & GPIO_PORT_MASK) == GPIO_PORT_K) ? 1 : 0)

#define GPIO_REG_DATA(x)                (GPIO_REG_BASE(x) + 0x00UL)
#define GPIO_REG_OUTEN(x)               (GPIO_REG_BASE(x) + 0x04UL)
#define GPIO_REG_DATA_OR(x)             (GPIO_REG_BASE(x) + 0x08UL)
#define GPIO_REG_DATA_BIC(x)            (GPIO_REG_BASE(x) + 0x0CUL)
#define GPIO_REG_PULLEN(x)              (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x10UL) : (GPIO_REG_BASE(x) + 0x1CUL))
#define GPIO_REG_PULLSEL(x)             (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x14UL) : (GPIO_REG_BASE(x) + 0x20UL))
#define GPIO_REG_CD(x,pin)              ((GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x18UL) : (GPIO_REG_BASE(x) + 0x14UL)) + (0x4UL * ((pin) / (uint32)16)))
#define GPIO_REG_IEN(x)                 (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x0CUL) :( GPIO_REG_BASE(x) + 0x24UL))
//#define GPIO_REG_IS(x)                  (GPIO_REG_BASE(x) + 0x28UL)
//#define GPIO_REG_SR(x)                  (GPIO_REG_BASE(x) + 0x2CUL)
#define GPIO_REG_FN(x,pin)              ((GPIO_REG_BASE(x) + 0x30UL) + (0x4UL * ((pin)/(uint32)8)))
#define GPIO_MFIO_CFG                   (MCU_BSP_GPIO_BASE + (0x2B4UL))
#define GPIO_PERICH_SEL                 (MCU_BSP_GPIO_BASE + (0x2B8UL))

#define GPIO_PMGPIO_SEL                 (GPIO_PMGPIO_BASE + 0x8UL)

#define GPIO_LIST_NUM                   (6)

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/

int mfio_ch_cfg_flag[3] = { 0, };

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/


static void GPIO_SetRegister(uint32 addr, uint32 bit, uint32 enable)
{
    uint32 base_val;
    uint32 set_val;

    base_val    = SAL_ReadReg(addr);
    set_val     = 0UL;

    if(enable == 1UL)
    {
        set_val     = (base_val | bit);
    }
    else if(enable == 0UL)
    {
        set_val     = (base_val & ~bit);
    }
    else
    {
        // Do nothing.
    }

    SAL_WriteReg(set_val, addr);
}

/*
***************************************************************************************************
*                                          GPIO_Config
*
* @param    [In] uiPort     :   Gpio port index, GPIO_GPX(X)
* @param    [In] uiConfig   :   Gpio configuration options
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPIO_Config
(
    uint32                              uiPort,
    uint32                              uiConfig
)
{
    uint32 pin;
    uint32 bit;
    uint32 func;
    uint32 pull;
    uint32 ds;
    uint32 ien;
    uint32 base_val;
    uint32 comp_val;
    uint32 set_val;
    uint32 reg_fn;
    uint32 pullen_addr;
    uint32 pullsel_addr;
    uint32 cd_addr;
    uint32 outen_addr;
    uint32 ien_addr;
	SALRetCode_t ret;

    ret     = SAL_RET_SUCCESS;
    pin     = uiPort & (uint32)GPIO_PIN_MASK;
    bit     = (uint32)1 << pin;
    func    = uiConfig & (uint32)GPIO_FUNC_MASK;
    pull    = uiConfig & ((uint32)GPIO_PULL_MASK << (uint32)GPIO_PULL_SHIFT);
    ds      = uiConfig & ((uint32)GPIO_DS_MASK << (uint32)GPIO_DS_SHIFT);
    ien     = uiConfig & ((uint32)GPIO_INPUTBUF_MASK << (uint32)GPIO_INPUTBUF_SHIFT);


    /* function */
    reg_fn      = GPIO_REG_FN(uiPort,pin);
    base_val    = SAL_ReadReg(reg_fn) & (~((uint32)0xF<<((pin%(uint32)8)*(uint32)4)));
    set_val     = base_val | (func<<((pin%(uint32)8)*(uint32)4));
    SAL_WriteReg(set_val, reg_fn);
    /* configuration check */
    comp_val    = SAL_ReadReg(reg_fn);

    if(comp_val != set_val)
    {
        ret = SAL_RET_FAILED;
    }
    else
    {
	    /* pull-up/down */
	    if (pull == GPIO_PULLUP)
	    {
	        if(GPIO_IS_GPIOK(uiPort))
	        {
	            pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
	        }
	        else
	        {
	            pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
	        }

	        GPIO_SetRegister(pullen_addr, bit, (uint32)TRUE);

	        if(GPIO_IS_GPIOK(uiPort))
	        {
	            pullsel_addr = (GPIO_PMGPIO_BASE + 0x14UL);
	        }
	        else
	        {
	            pullsel_addr = (GPIO_REG_BASE(uiPort) + 0x20UL);
	        }

            GPIO_SetRegister(pullsel_addr, bit, (uint32)TRUE);
        }
        else if (pull == GPIO_PULLDN)
        {
	        if(GPIO_IS_GPIOK(uiPort))
            {
                pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
            }
            else
            {
                pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
            }

            GPIO_SetRegister(pullen_addr, bit, (uint32)TRUE);

	        if(GPIO_IS_GPIOK(uiPort))
	        {
	            pullsel_addr = (GPIO_PMGPIO_BASE + 0x14UL);
	        }
	        else
	        {
	            pullsel_addr = (GPIO_REG_BASE(uiPort) + 0x20UL);
	        }

            GPIO_SetRegister(pullsel_addr, bit, (uint32)FALSE);
        }
        else
        {
        	if(GPIO_IS_GPIOK(uiPort))
            {
                pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
            }
            else
            {
                pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
            }

            GPIO_SetRegister(pullen_addr, bit, (uint32)FALSE);
        }

	    /* drive strength */
	    if (ds != 0UL)
	    {
	        if(GPIO_IS_GPIOK(uiPort))
	        {
	            cd_addr = (GPIO_PMGPIO_BASE + 0x18UL) + (0x4UL * ((pin) / (uint32)16));
	        }
	        else
	        {
	            cd_addr = (GPIO_REG_BASE(uiPort) + 0x14UL) + (0x4UL * ((pin) / (uint32)16));
	        }

	        ds          = ds >> (uint32)GPIO_DS_SHIFT;
	        base_val    = SAL_ReadReg(cd_addr) & ~((uint32)3 << ((pin % (uint32)16) * (uint32)2));
	        set_val     = base_val | ((ds & (uint32)0x3) << ((pin % (uint32)16) * (uint32)2));
            SAL_WriteReg(set_val, cd_addr);
        }

        /* direction */
        if ((uiConfig&GPIO_OUTPUT) != 0UL)
        {
            outen_addr  = GPIO_REG_OUTEN(uiPort);

            GPIO_SetRegister(outen_addr, bit, (uint32)TRUE);
        }
        else
        {
            outen_addr  = GPIO_REG_OUTEN(uiPort);

            GPIO_SetRegister(outen_addr, bit, (uint32)FALSE);
        }

        /* input buffer enable */
        if (ien == GPIO_INPUTBUF_EN)
        {
    	    if(GPIO_IS_GPIOK(uiPort))
            {
                ien_addr = (GPIO_PMGPIO_BASE + 0x0CUL);
            }
            else
            {
                ien_addr = (GPIO_REG_BASE(uiPort) + 0x24UL);
            }

            GPIO_SetRegister(ien_addr, bit, (uint32)TRUE);
        }
        else if (ien == GPIO_INPUTBUF_DIS)
        {
	        if(GPIO_IS_GPIOK(uiPort))
            {
                ien_addr = (GPIO_PMGPIO_BASE + 0x0CUL);
            }
            else
            {
                ien_addr = (GPIO_REG_BASE(uiPort) + 0x24UL);
            }

            GPIO_SetRegister(ien_addr, bit, (uint32)FALSE);
        }
        else //QAC
        {
            ; // no statement
        }

    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPIO_Get
*
* @param    [In] uiPort     :   Gpio port index, GPIO_GPX(X)
* @return   Gpio data value (0:Low or 1:High)
*
* Notes
*
***************************************************************************************************
*/

uint8 GPIO_Get
(
    uint32                              uiPort
)
{
    uint32 reg_data;
    uint32 value;
    uint8  ret;

    reg_data    = GPIO_REG_DATA(uiPort);
    value       = (SAL_ReadReg(reg_data) & ((uint32)1 << (uiPort & GPIO_PIN_MASK)));

    if (value != 0UL)
    {
        ret = (uint8)1;
    }
    else
    {
        ret = (uint8)0;
    }

    return ret;
}


/*
***************************************************************************************************
*                                          GPIO_Set
* @param    [In] uiPort     :   Gpio port index, GPIO_GPX(X)
* @param    [In] uiData     :   Gpio data value, (0 or 1)
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPIO_Set
(
    uint32                              uiPort,
    uint32                              uiData
)
{
    uint32 bit;
    uint32 data_or;
    uint32 data_bic;
    SALRetCode_t ret;

    ret = SAL_RET_SUCCESS;

    bit = (uint32)1 << (uiPort & GPIO_PIN_MASK);

    if (uiData > 1UL)
    {
        ret = SAL_RET_FAILED;
    }
    else
    {
        /* set data */
        if (uiData!=0UL)
        {
            data_or = GPIO_REG_DATA_OR(uiPort);
            SAL_WriteReg(bit, data_or);
        }
        else
        {
            data_bic = GPIO_REG_DATA_BIC(uiPort);
            SAL_WriteReg(bit, data_bic);
        }
    }

    return ret;
}


/*
***************************************************************************************************
*                                          GPIO_ToNum
*
* @param    [In] uiPort     :   Gpio port index, GPIO_GPX(X)
* @return
*
* Notes
*
***************************************************************************************************
*/
uint32 GPIO_ToNum
(
    uint32                              uiPort
)
{
    uint32 pin;
    uint32 ret;
    uint32 port;

    pin = uiPort & GPIO_PIN_NUM_MASK;
    ret = 0UL;
    port = uiPort & GPIO_PORT_MASK;

    switch (port)
    {
        case GPIO_PORT_A:
        {
            ret = pin;
            break;
        }

        case GPIO_PORT_B:
        {
            ret = (0x20UL + pin);
            break;
        }
        case GPIO_PORT_C:
        {
            ret = (0x3dUL + pin);
            break;
        }
        case GPIO_PORT_K:
        {
        	ret = (0x86UL + pin);

            break;
        }

        default:
        {
            GPIO_E("\n Can't find GPIO Port \n");
            break;
        }
    }

    return ret;
}

/*
***************************************************************************************************
*                                          GPIO_PerichSel
*
* @param    [In] uiPerichSel    : Gpio peri select index
* @param    [In] uiCh           : Gpio peri select channel index
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t GPIO_PerichSel
(
    uint32                              uiPerichSel,
    uint32                              uiCh
)
{
    uint32 peri_sel_addr;
    uint32 clear_bit;
    uint32 set_bit;
    uint32 base_val;
    uint32 comp_val;

    peri_sel_addr = GPIO_PERICH_SEL;
    base_val = SAL_ReadReg(peri_sel_addr);

    if(uiPerichSel < GPIO_PERICH_SEL_I2SSEL_0)
    {
        if(uiCh < 2)
        {
            //clear bit
            clear_bit = base_val & ~((0x1UL) << uiPerichSel);
            SAL_WriteReg(clear_bit, peri_sel_addr);
            //set bit
            base_val = SAL_ReadReg(peri_sel_addr);
            set_bit = base_val | ((uiCh & 0x1UL) << uiPerichSel);
            SAL_WriteReg(set_bit,peri_sel_addr);
            comp_val    = SAL_ReadReg(peri_sel_addr);

            if(comp_val != set_bit)
            {
                GPIO_E("GPIO PerichSel 1bit error \n");
                return SAL_RET_FAILED;
            }
        }
        else
        {
            GPIO_E("GPIO PerichSel ch 1bit error \n");
            return SAL_RET_FAILED;
        }
    }
    else
    {
        if(uiCh < 4)
        {
            //clear bit
            clear_bit = base_val & ~((0x3UL) << uiPerichSel);
            SAL_WriteReg(clear_bit, peri_sel_addr);
            //set bit
            base_val = SAL_ReadReg(peri_sel_addr);
            set_bit = base_val | ((uiCh & 0x3UL) << uiPerichSel);
            SAL_WriteReg(set_bit,peri_sel_addr);
            comp_val    = SAL_ReadReg(peri_sel_addr);

            if(comp_val != set_bit)
            {
                GPIO_E("GPIO PerichSel 2bit error \n");
                return SAL_RET_FAILED;
            }
        }
        else
        {
            GPIO_E("GPIO PerichSel ch 2bit error \n");
            return SAL_RET_FAILED;
        }
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPIO_MfioCfg
*
* @param    [In] uiPeriSel      :   MFIO peri select index
* @param    [In] uiPeriType     :   MFIO peri select type (Disable/GPSB/UART/I2C)
* @param    [In] uiChSel        :   MFIO channel select index
* @param    [In] uiChNum        :   MFIO channel select value
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
*
* Notes
*
***************************************************************************************************
*/
SALRetCode_t GPIO_MfioCfg
(
    uint32                              uiPeriSel,
    uint32                              uiPeriType,
    uint32                              uiChSel,
    uint32                              uiChNum
)
{
    uint32 base_val;
    uint32 set_val;
    uint32 clear_bit;
    uint32 comp_val;

    if(uiPeriSel == GPIO_MFIO_CFG_PERI_SEL0)
    {
        if(uiChSel == GPIO_MFIO_CFG_CH_SEL0)
        {
            if(mfio_ch_cfg_flag[0] == 0)
            {
                //clear bit
                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                clear_bit = base_val & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL0)
                                     & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL0);
                SAL_WriteReg(clear_bit, GPIO_MFIO_CFG);

                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                set_val = base_val  | ((uiChNum & 0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL0)
                                    | ((uiPeriType & 0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL0);
                SAL_WriteReg(set_val, GPIO_MFIO_CFG);
                comp_val    = SAL_ReadReg(GPIO_MFIO_CFG);

                if(comp_val != set_val)
                {
                    return SAL_RET_FAILED;
                }
                mfio_ch_cfg_flag[0] = 1;
            }
            else
            {
                GPIO_E("MFID 0 ch%d already set!!\n",uiChNum);
                return SAL_RET_FAILED;
            }

        }
        else
        {
            GPIO_E("match perisel0, chsel0 \n");
            return SAL_RET_FAILED;
        }
    }
    else if(uiPeriSel == GPIO_MFIO_CFG_PERI_SEL1)
    {
        if(uiChSel == GPIO_MFIO_CFG_CH_SEL1)
        {
            if(mfio_ch_cfg_flag[1] == 0)
            {
                //clear bit
                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                clear_bit = base_val & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL1)
                                     & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL1);
                SAL_WriteReg(clear_bit, GPIO_MFIO_CFG);

                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                set_val = base_val  | ((uiChNum & 0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL1)
                                    | ((uiPeriType & 0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL1);
                SAL_WriteReg(set_val, GPIO_MFIO_CFG);
                comp_val    = SAL_ReadReg(GPIO_MFIO_CFG);

                if(comp_val != set_val)
                {
                    return SAL_RET_FAILED;
                }
                mfio_ch_cfg_flag[1] = 1;
            }
            else
            {
                GPIO_E("MFID 01 ch%d already set!!\n",uiChNum);
                return SAL_RET_FAILED;
            }
        }
        else
        {
            GPIO_E("match perisel1, chsel1 \n");
            return SAL_RET_FAILED;
        }

    }
    else if(uiPeriSel == GPIO_MFIO_CFG_PERI_SEL2)
    {
        if(uiChSel == GPIO_MFIO_CFG_CH_SEL2)
        {
            if(mfio_ch_cfg_flag[2] == 0)
            {
                //clear bit
                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                clear_bit = base_val & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL2)
                                     & ~((0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL2);
                SAL_WriteReg(clear_bit, GPIO_MFIO_CFG);

                base_val = SAL_ReadReg(GPIO_MFIO_CFG);
                set_val = base_val  | ((uiChNum & 0x3UL) << (uint32)GPIO_MFIO_CFG_CH_SEL2)
                                    | ((uiPeriType & 0x3UL) << (uint32)GPIO_MFIO_CFG_PERI_SEL2);
                SAL_WriteReg(set_val, GPIO_MFIO_CFG);
                comp_val    = SAL_ReadReg(GPIO_MFIO_CFG);

                if(comp_val != set_val)
                {
                    return SAL_RET_FAILED;
                }
                mfio_ch_cfg_flag[2] = 1;
            }
            else
            {
                GPIO_E("MFID 02 ch%d already set!!\n",uiChNum);
                return SAL_RET_FAILED;
            }
        }
        else
        {
            GPIO_E("match perisel2, chsel2 \n");
            return SAL_RET_FAILED;
        }
    }
    else
    {
        GPIO_E("check perisel \n");
        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/**************************************************************************************************
*                                       GPIO_IntExtSet
*
* Configure external interrupt source select.
*
* @param    [In] uiIntId        : Index of Interrupt Source id.
* @param    [In] uiGpio         : Gpio port index, GPIO_GPX(X)
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
*
* Notes
*
**************************************************************************************************/
SALRetCode_t GPIO_IntExtSet
(
    uint32                              uiIntId,
    uint32                              uiGpio
)
{
    SALRetCode_t  ucRet;
    uint32        uiEintSel;
    uint32        uiBitField;
    uint32        uiExtIntGpioIdx;
    uint32        uiEIntSel;
    uint32        uiEIntSelMask;

    static const uint32 ExtIntr[] =
    {
        GPIO_GPA(0UL),   GPIO_GPA(1UL),   GPIO_GPA(2UL),   GPIO_GPA(3UL),   GPIO_GPA(4UL),   GPIO_GPA(5UL),   GPIO_GPA(6UL),   GPIO_GPA(7UL),
        GPIO_GPA(8UL),   GPIO_GPA(9UL),   GPIO_GPA(10UL),  GPIO_GPA(11UL),  GPIO_GPA(12UL),  GPIO_GPA(13UL),  GPIO_GPA(14UL),  GPIO_GPA(15UL),
        GPIO_GPA(16UL),  GPIO_GPA(17UL),  GPIO_GPA(18UL),  GPIO_GPA(18UL),  GPIO_GPA(20UL),  GPIO_GPA(21UL),  GPIO_GPA(22UL),  GPIO_GPA(23UL),
        GPIO_GPA(24UL),  GPIO_GPA(25UL),  GPIO_GPA(26UL),  GPIO_GPA(27UL),  GPIO_GPA(28UL),  GPIO_GPA(29UL),  GPIO_GPA(30UL),

        GPIO_GPB(0UL),   GPIO_GPB(1UL),   GPIO_GPB(2UL),   GPIO_GPB(3UL),   GPIO_GPB(4UL),   GPIO_GPB(5UL),   GPIO_GPB(6UL),   GPIO_GPB(7UL),
        GPIO_GPB(8UL),   GPIO_GPB(9UL),   GPIO_GPB(10UL),  GPIO_GPB(11UL),  GPIO_GPB(12UL),  GPIO_GPB(13UL),  GPIO_GPB(14UL),  GPIO_GPB(15UL),
        GPIO_GPB(16UL),  GPIO_GPB(17UL),  GPIO_GPB(18UL),  GPIO_GPB(19UL),  GPIO_GPB(20UL),  GPIO_GPB(21UL),  GPIO_GPB(22UL),  GPIO_GPB(23UL),
        GPIO_GPB(24UL),  GPIO_GPB(25UL),  GPIO_GPB(26UL),  GPIO_GPB(27UL),  GPIO_GPB(28UL),

        GPIO_GPC(0UL),   GPIO_GPC(1UL),   GPIO_GPC(2UL),   GPIO_GPC(3UL) ,  GPIO_GPC(4UL),   GPIO_GPC(5UL),   GPIO_GPC(6UL),   GPIO_GPC(7UL),
        GPIO_GPC(8UL),   GPIO_GPC(9UL),   GPIO_GPC(10UL),  GPIO_GPC(11UL),  GPIO_GPC(12UL),  GPIO_GPC(13UL),  GPIO_GPC(14UL),  GPIO_GPC(15UL),
        GPIO_GPC(16UL),  GPIO_GPC(17UL),  GPIO_GPC(18UL),  GPIO_GPC(19UL),  GPIO_GPC(20UL),  GPIO_GPC(21UL),  GPIO_GPC(22UL),  GPIO_GPC(23UL),
        GPIO_GPC(24UL),  GPIO_GPC(25UL),  GPIO_GPC(26UL),  GPIO_GPC(27UL),

        GPIO_GPK(0UL),   GPIO_GPK(1UL),   GPIO_GPK(2UL),   GPIO_GPK(3UL) ,  GPIO_GPK(4UL),   GPIO_GPK(5UL),   GPIO_GPK(6UL),   GPIO_GPK(7UL),
        GPIO_GPK(8UL),   GPIO_GPK(9UL),   GPIO_GPK(10UL),  GPIO_GPK(11UL),  GPIO_GPK(12UL),  GPIO_GPK(13UL),  GPIO_GPK(14UL),  GPIO_GPK(15UL),
        GPIO_GPK(16UL),  GPIO_GPK(17UL),
    };

    ucRet           = (SALRetCode_t)SAL_RET_SUCCESS;
    uiEintSel       = ((MCU_BSP_GPIO_BASE + 0x280UL) + (4UL*((uiIntId-(uint32)GIC_EXT0)/4UL))); /* EINT_SEL0,2,3 */
    uiBitField      = 0;
    uiExtIntGpioIdx     = 0;
    uiEIntSel       = 0;
    uiEIntSelMask   = 0;

    if ((uiIntId < (uint32)GIC_EINT_START_INT) || (uiIntId > (uint32)GIC_EINT_END_INT))
    {
        ucRet = (SALRetCode_t)SAL_RET_FAILED;
    }
    else
    {

        for ( ; uiExtIntGpioIdx < SAL_ArraySize(ExtIntr) ; uiExtIntGpioIdx++)
        {
            if (ExtIntr[uiExtIntGpioIdx] == uiGpio)
            {
                break;
            }
        }

        if (uiExtIntGpioIdx >= SAL_ArraySize(ExtIntr))
        {
            ucRet = (SALRetCode_t)SAL_RET_FAILED;
        }
        else
        {
            uiBitField      = (uint32)(8UL * ((uiIntId - (uint32)GIC_EXT0) % 4UL));
            uiEIntSelMask   = ((uint32)0x7FUL << uiBitField);

            uiEIntSel = (uint32)SAL_ReadReg(uiEintSel);
            uiEIntSel = (uint32)((uiEIntSel & ~uiEIntSelMask) | (uiExtIntGpioIdx << uiBitField));
            SAL_WriteReg(uiEIntSel, uiEintSel);
        }
    }

    return ucRet;
}

#endif  // ( MCU_BSP_SUPPORT_DRIVER_GPIO == 1 )

