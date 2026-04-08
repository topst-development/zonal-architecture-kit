// SPDX-License-Identifier: Apache-2.0

/*
***************************************************************************************************
*
*   FileName : debug.c
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*/


#if ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )
    #include "uart.h"
#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )
#include "debug.h"

uint32                                  gDebugOption = 0xC0000001UL;

DbgTag_t                                dbgTags[DBG_MAX_TAGS] =
{
    {"SAL\0\0\0"},
    {"MPU\0\0\0"},
    {"GIC\0\0\0"},
    {"GMAC\0\0"},
    {"MIDF\0\0"},  //5
    {"CAN\0\0\0"},
    {"GPSB\0\0"},
    {"UART\0\0"},
    {"I2C\0\0\0"},
    {"PDM\0\0\0"},
    {"ICTC\0\0"}, //10
    {"GDMA\0\0"},
    {"ADC\0\0\0"},
    {"MBOX\0\0"},
    {"TIMER\0"},
    {"WDT\0\0\0"}, //15
    {"CKC\0\0\0"},
    {"GPIO\0\0"},
    {"PMIO\0\0"},
    {"DSE\0\0\0"},
    {"SDM\0\0\0"}, //20
    {"FMU\0\0\0"},
    {"FWUD\0\0"},
    {"LIN\0\0\0"},
    {"I2S\0\0\0"},
    {"EFLASH"}, //25
    {"SSM\0\0\0"},
};

#if DEBUG_ENABLE
#define is_digit(c)                     (((c) >= (sint8)'0') && ((c) <= (sint8)'9'))
#define to_upper(c)                     ((c) & ~0x20)

typedef void                            (fPUTC)(sint8 c);

static sint32 DBG_Print8
(
    uint32                              num,
    sint8 *                             str
);

static sint32 DBG_Print10
(
    uint32                              num,
    sint8 *                             str
);

static sint32 DBG_Print16L
(
    uint32                              num,
    sint8 *                             str
); //Lowercase

static sint32 DBG_Print16U
(
    uint32                              num,
    sint8 *                             str
); //Uppercase

sint32 DBG_Putc
(
    sint8                               putChar
);

sint32 DBG_Printfi
(
    fPUTC *                             fPutC,
    sint8 *                             fmt_in,
    sint32 *                            args_in
);

/*------- external -----------*/
sint32 DBG_Putc
(
    sint8                               putChar
) /* QAC - MISRA-C:2004 Rule 8.8 - This function is used in ASM */
{
#if ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )
    if (putChar == (sint8)'\n')
    {
        (void)UART_PutChar((uint32)UART_DEBUG_CH, (uint8)'\r');
    }

    (void)UART_PutChar((uint32)UART_DEBUG_CH, (uint8)putChar);

    return 0;
#else
    return -1;
#endif  // ( MCU_BSP_SUPPORT_DRIVER_UART == 1 )
}

/*
 * print character
 */
static sint32 DBG_Print8
(
    uint32                              num,
    sint8 *                             str
)
{
    sint32  len;
    sint8   temp[20];
    sint8   *p  = temp;
    sint8   *destStr;
    uint32  numChar;
    uint32  calcTemp;

    numChar = num;
    *p      = 0;
    len     = 0;

    if(str != NULL_PTR)
    {
        destStr = str;

        do
        {
            ++p;
            calcTemp    = numChar%8UL; //orig numChar & 0x7
            *p          = ((sint8)((sint8)calcTemp) + (sint8)'0');
            numChar     = numChar/8UL;//orig numChar     = numChar >> 3U;
            len++;
        } while (numChar!=0UL);

        while(1)
        {
            *destStr    = *p;
            destStr++;

            if((*p) == 0)
            {
                break;
            }

            p--;
         }
    }

    return  len;
}

/*
 * print decimal
 */
static sint32 DBG_Print10
(
    uint32                              num,
    sint8 *                             str
)
{
    sint32  len;
    sint8   temp[20];
    sint8   *p  = temp;
    sint8   *destStr;
    uint32 numChar;
    uint32 calcTemp;

    numChar = num;
    *p      = 0;
    len     = 0;

    if(str != NULL_PTR)
    {
        destStr = str;

        do
        {
            ++p;
            calcTemp    = numChar % 10UL;
            *p          = (sint8)((sint8)calcTemp + (sint8)'0');
            numChar     = numChar / 10UL;
            len++;
        } while (numChar != 0UL);

        while(1)
        {
            *destStr    = *p;
            destStr++;

            if((*p) == 0)
            {
                break;
            }

            p--;
        }
    }

    return  len;
}

/*
 * print hexa-decimal, lower case
 */
static sint32 DBG_Print16L
(
    uint32                              num,
    sint8 *                             str
)
{
    sint32  len;
    sint8   temp[20];
    sint8   *p  = temp;
    sint8   *destStr;
    uint32  numChar;
    uint32  calcTemp;

    numChar = num;
    *p      = 0;
    len     = 0;

    if(str != NULL_PTR)
    {
        destStr = str;

        do
        {
            ++p;
            calcTemp    = numChar % 16UL; //numChar & 0x0f
            *p          = (sint8)"0123456789abcdef"[calcTemp];
            numChar     = numChar / 16UL;
            len++;
        } while (numChar != 0UL);

        while(1)
        {
            *destStr    = *p;
            destStr++;

            if((*p) == 0)
            {
                break;
            }

            p--;
        }
    }

    return  len;
}

/*
 * print hexa-decimal, upper case
 */
static sint32 DBG_Print16U
(
    uint32                              num,
    sint8 *                             str
)
{
    sint32  len;
    sint8   temp[20];
    sint8   *p  = temp;
    sint8   *destStr;
    uint32  numChar;
    uint32  calcTemp;

    numChar = num;
    *p      = 0;
    len     = 0;

    if(str != NULL_PTR)
    {
        destStr = str;

        do
        {
            ++p;
            calcTemp    = numChar % 16UL; //numChar & 0x0f
            *p          = (sint8)"0123456789ABCDEF"[calcTemp];
            numChar     = numChar / 16UL;
            len++;
        } while (numChar != 0UL);

        while(1)
        {
            *destStr    = *p;
            destStr++;

            if((*p) == 0)
            {
                break;
            }

            p--;
        }
    }

    return  len;
}

/*
 *
 */
sint32 DBG_Printfi
(
    fPUTC *                             fPutC,
    sint8 *                             fmt_in,
    sint32 *                            args_in
) /* QAC - MISRA-C:2004 Rule 8.8 - This function is used in ASM */
{
    sint32      i;
    sint32      f;
    sint8       c;
    sint32      length;
    sint32      leftjust;
    sint32      leading;
    sint32      fmaxString;
    sint32      fminString;
    sint8       sign;
    sint8       fill;
    sint8 *     str;
    sint8       string[64];
    sint8 *     fmt;
    sint32 *    args;

    length      = 0;
    leftjust    = 0;

    if ((fPutC != NULL_PTR) && (fmt_in != NULL_PTR) && (args_in != NULL_PTR))
    {
        fmt     = fmt_in;
        args    = args_in;

        for(;;)
        {
            /* Echo character until '%' or end of fmt string */
            while ((*fmt) != (sint8)'%')
            {
                c = *fmt;
                fmt++;

                if (c == (sint8)0)//if (!c)
                {
                    return (-1);
                }

                (*fPutC)(c);
            }

            fmt++;

            /* Echo " ... %%..." as '%'  */
            if (*fmt == (sint8)'%')
            {
                (*fPutC)(*fmt);
                fmt++;

                continue;
            }

            /* Check for "%-...",== Left-justified output */
            if (*fmt == (sint8)'-')//if ((*fmt == (sint8)'-') != 0)
            {
                leftjust = 1;
                fmt ++;
            }
            else
            {
                leftjust = 0;
            }

            /* Allow for zero-filled numeric outputs ("%0...") */
            if (*fmt ==(sint8)'0')
            {
                fill = *fmt;
                fmt++;
            }
            else
            {
                fill = (sint8)' ';
            }

            /* Allow for minimum field width specifier for %d %u x,c,o,s */
            /* Also  allow %* for variable width (%0* as well)   */
            fminString = 0;

            if (*fmt == (sint8)'*')
            {
                fminString = *args;
                args++;
                ++fmt;
            }
            else
            {
                while ((sint8)is_digit(*fmt) != 0)
                {
                    fminString = (((fminString * 10) + (*fmt)) - (sint8)'0') ;
                    fmt++;
                }
            }

            /* Allow for maximum string width for %s */
            fmaxString = 0;

            if (*fmt == (sint8)'.')
            {
                ++fmt;

                if (*fmt == (sint8)'*' )
                {
                    fmaxString = *args;
                    args++;
                    ++fmt;
                }
                else
                {
                    while ((sint8)is_digit(*fmt) != 0)
                    {
                        fmaxString = (((fmaxString * 10) + (*fmt)) - (sint8)'0');
                        fmt++;
                    }
                }
            }

            if ((*fmt) == 0)
            {
                f = *fmt;
                fmt++;
                (*fPutC)((sint8)'%');

                return (-1);
            }
            else
            {
                f = *fmt;
                fmt++;
            }

            sign = 0;

            switch (f)
            {
                case 0x63/*'c'*/ :
                {
                    string[0]   = (sint8)*args;
                    string[1]   = (sint8)'\0';
                    str         = string;
                    length      = 1;
                    fmaxString  = 0;
                    fill        = (sint8)' ';
                    break;
                }

                case 0x73/*'s'*/ :
                {
                    str = (sint8 *)*args;

                    for (length = 0; length < 2147483647; length ++)//change of for(;;)
                    {
                        if(*str == 0)
                        {
                            break;
                        }

                        str++;
                    }
                    str++;
                    str     = (sint8 *)*args;
                    fill    = (sint8)' ';
                    break;
                }

                case 0x64 /*'d'*/ :
                {
                    str = string;
                    i   = *(const sint32 *)args;

                    if (i < 0)
                    {
                        sign    = (sint8)'-';
                        i       = -i;
                    }

                    length      = DBG_Print10((uint32)i, str);
                    fmaxString  = 0;
                    break;
                }

                case 0x6f /*'o'*/ :
                {
                    str         = string;
                    length      = DBG_Print8((uint32)(*(const sint32 *)args), str);
                    fmaxString  = 0;
                    break;
                }

                case 0x78 /*'x'*/ :
                {
                    str         = string;
                    length      = DBG_Print16L((uint32)(*(const sint32 *)args), str);
                    fmaxString  = 0;
                    break;
                }

                case 0x58 /*'X'*/ :
                {
                    str         = string;
                    length      = DBG_Print16U((uint32)(*(const sint32 *)args), str);
                    fmaxString        = 0;
                    break;
                }

                default:
                {
                    /* format is not supported */
                    str     = (sint8 *) *args;
                    length  = 1;
                    break;
                }
            }

        //  if (fminString > uDBG_txbuf_len)
        //      fminString = 0;
        //  if (fmaxString > uDBG_txbuf_len)
        //      fmaxString = 0;

            leading = 0;

            if ((fmaxString != 0) || (fminString!=0))
            {
                if (fmaxString != 0)
                {
                    if (length > fmaxString)
                    {
                        length  = fmaxString;
                    }
                }

                if (fminString!=0)
                {
                    leading     = fminString - length;
                }

                if (sign == (sint8)'-')
                {
                    leading --;
                }
            }

            if ((sign == (sint8)'-') && (fill == (sint8)'0'))
            {
                (*fPutC)(sign);
            }

            if (leftjust==0)//if (!leftjust)
            {
                for (i = 0; i < leading; i ++)
                {
                    (*fPutC)(fill);
                }
            }

            if ((sign == (sint8)'-') && (fill == (sint8)' '))
            {
                (*fPutC)(sign);
            }

            for (i = 0; i < length; i ++)
            {
                (*fPutC)(*str);
                str++;
            }

            if (leftjust != 0)
            {
                for (i = 0; i < leading; i ++)
                {
                    (*fPutC)(fill);
                }
            }

            args++;
        }
    }

    return (-1);
}

__attribute__ ((naked)) void DBG_SerialPrintf
(
    const int8 *                        format,
    ...
) /* QAC - MISRA-C:2004 Rule 16.1 - Printf() needs variable parameters */
{
    /*The compiler only supports basic __asm statements in __attribute__((naked)) functions.
      Using extended assembly, parameter references or mixing C code with __asm statements might not work reliably.*/
    //(void)format;
#if defined(__GNUC__)
    __asm volatile (
            "stmdb  sp!, {r1-r3}    \n"
            "mov    r1, r0          \n"     /* r1 = format string */
            "mov    r2, sp          \n"     /* r2 = arguments */
            "ldr    r0, =DBG_Putc     \n"     /* r0 = function pointer which will put character */
            "str    lr, [sp, #-4]!  \n"     /* save r14 */
            "bl     DBG_Printfi       \n"
            "ldr    lr, [sp], #4    \n"     /* restore r14 */
            "add    sp, sp, #12     \n"     /* restore stack */
            "mov    pc, lr          \n"
            :::"r0", "r1", "r2", "r3" \
    );
#else //Green Hills case
    #pragma asm
    mov r1, r0
    mov r2, sp
    add r2, r2, #8
    ldr r0, =DBG_Putc
    str lr, [sp, #-4]!
    bl  DBG_Printfi
    ldr lr, [sp], #4
    add sp, sp, #20
    mov pc, lr
    #pragma endasm
#endif
}
#else
__attribute__ ((naked)) void DBG_SerialPrintf
(
    const int8 *                        format,
    ...
) /* QAC - MISRA-C:2004 Rule 16.1 - Printf() needs variable parameters */
{

}
#endif

