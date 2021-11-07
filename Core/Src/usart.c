#include "usart.h"
#include "stdarg.h"
#include "sys.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#define DEBUG_UART USART1

static char *itoa(int value, char *string, int radix);

u8 USART_RX_BUF[USART_REC_LEN];

u16 USART_RX_STA = 0;

/*****************************************************************************
 * @name       :void Debug_Init(u32 bound)
 * @date       :2020-05-08
 * @function   :Initialize Debug
 * @parameters :bound:baudrate
 * @retvalue   :None
 ******************************************************************************/
void Debug_Init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
/* Method 1 using usart send */
void Debug_Send(const char *str)
{
    while (*str)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
        USART_SendData(DEBUG_UART, *str++);
    }
}
/* Method 2 using itoa */
void Debug_Printf(char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, Data);

    while (*Data != 0) // Determine whether the end of string is reached
    {
        if (*Data == 0x5c) //'\'
        {
            switch (*++Data)
            {
            case 'r': // carriage return
                USART_SendData(DEBUG_UART, 0x0d);
                Data++;
                break;

            case 'n': // line break
                USART_SendData(DEBUG_UART, 0x0a);
                Data++;
                break;

            default:
                Data++;
                break;
            }
        }

        else if (*Data == '%')
        { //
            switch (*++Data)
            {
            case 's': // string
                s = va_arg(ap, const char *);

                for (; *s; s++)
                {
                    USART_SendData(DEBUG_UART, *s);
                    while (USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET)
                        ;
                }

                Data++;
                break;
            case 'd':
                // Decimal
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    USART_SendData(DEBUG_UART, *s);
                    while (USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET)
                        ;
                }
                Data++;
                break;
            default:
                Data++;
                break;
            }
        }
        else
            USART_SendData(DEBUG_UART, *Data++);
        while (USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET)
            ;
    }
}

/*
 * Function name: itoa
 * Description: Convert integer data to character string
 * Input: -radix =10 means decimal, other results are 0
 * -value Integer to be converted
 * -buf converted string
 *         -radix = 10
 * Output: None
 * Return: None
 * Call: Called by Debug_printf()
 */
static char *itoa(int value, char *string, int radix)
{
    int i, d;
    int flag = 0;
    char *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;
}

/*****************************************************************************
 * @name       :void Debug_Init(u32 bound)
 * @date       :2020-05-08
 * @function   :Debug interrupt service program
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void USART1_IRQHandler(void)
{
    u8 Res;
    if (USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(DEBUG_UART);
        if ((USART_RX_STA & 0x8000) == 0)
        {
            if (USART_RX_STA & 0x4000)
            {
                if (Res != 0x0a)
                {
                    USART_RX_STA = 0;
                }
                else
                {
                    USART_RX_STA |= 0x8000;
                }
            }
            else
            {
                if (Res == 0x0d)
                {
                    USART_RX_STA |= 0x4000;
                }
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
                    USART_RX_STA++;
                    if (USART_RX_STA > (USART_REC_LEN - 1))
                    {
                        USART_RX_STA = 0;
                    }
                }
            }
        }
    }
}
