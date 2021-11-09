#include "usb_vcp.h"
#include "usbd_usr.h"
#include <stdarg.h>

/* Private */
uint8_t USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];
uint32_t usb_vcp_buf_in, usb_vcp_buf_out, usb_vcp_buf_num;
extern USB_VCP_Result USB_VCP_INT_Status;
extern LINE_CODING linecoding;
uint8_t USB_VCP_INT_Init = 0;
USB_OTG_CORE_HANDLE USB_OTG_dev;

/* USB VCP Internal receive buffer */
extern uint8_t USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];

USB_VCP_Result USB_VCP_Init(void)
{
    /* Initialize USB */
    USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_FS
              USB_OTG_FS_CORE_ID,
#else
              USB_OTG_HS_CORE_ID,
#endif
              &USR_desc, &USBD_CDC_cb, &USR_cb);

    /* Reset buffer counters */
    usb_vcp_buf_in = 0;
    usb_vcp_buf_out = 0;
    usb_vcp_buf_num = 0;

    /* Initialized */
    USB_VCP_INT_Init = 1;

    /* Return OK */
    return USB_VCP_OK;
}

uint8_t USB_VCP_BufferEmpty(void)
{
    return (usb_vcp_buf_num == 0);
}

uint8_t USB_VCP_BufferFull(void)
{
    return (usb_vcp_buf_num == USB_VCP_RECEIVE_BUFFER_LENGTH);
}

uint8_t USB_VCP_FindCharacter(volatile char c)
{
    uint16_t num, out;

    /* Temp variables */
    num = usb_vcp_buf_num;
    out = usb_vcp_buf_out;

    while (num > 0)
    {
        /* Check overflow */
        if (out == USB_VCP_RECEIVE_BUFFER_LENGTH)
        {
            out = 0;
        }
        if (USB_VCP_ReceiveBuffer[out] == c)
        {
            /* Character found */
            return 1;
        }
        out++;
        num--;
    }

    /* Character is not in buffer */
    return 0;
}

USB_VCP_Result USB_VCP_Getc(uint8_t *c)
{
    /* Any data in buffer */
    if (usb_vcp_buf_num > 0)
    {
        /* Check overflow */
        if (usb_vcp_buf_out >= USB_VCP_RECEIVE_BUFFER_LENGTH)
        {
            usb_vcp_buf_out = 0;
        }
        *c = USB_VCP_ReceiveBuffer[usb_vcp_buf_out];
        USB_VCP_ReceiveBuffer[usb_vcp_buf_out] = 0;

        /* Set counters */
        usb_vcp_buf_out++;
        usb_vcp_buf_num--;

        /* Data OK */
        return USB_VCP_DATA_OK;
    }
    *c = 0;
    /* Data not ready */
    return USB_VCP_DATA_EMPTY;
}

USB_VCP_Result USB_VCP_Putc(volatile char c)
{
    uint8_t ce = (uint8_t)c;

    /* Send data over USB */
    CDC_DataTx(&ce, 1);

    /* Return OK */
    return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_Puts(const char *str)
{
    while (*str)
    {
        USB_VCP_Putc(*str++);
    }

    /* Return OK */
    return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_itoa(long val, int radix, int len)
{
    uint8_t r, sgn = 0, pad = ' ';
    uint8_t s[20], i = 0;
    uint32_t v;

    if (radix < 0)
    {
        radix = -radix;
        if (val < 0)
        {
            val = -val;
            sgn = '-';
        }
    }
    v = val;
    r = radix;
    if (len < 0)
    {
        len = -len;
        pad = '0';
    }
    if (len > 20)
        return USB_VCP_ERROR;
    do
    {
        uint8_t c = (uint8_t)(v % r);
        if (c >= 10)
            c += 7;
        c += '0';
        s[i++] = c;
        v /= r;
    } while (v);
    if (sgn)
        s[i++] = sgn;
    while (i < len)
        s[i++] = pad;
    do
        USB_VCP_Putc(s[--i]);
    while (i);
    return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_Printf(const char *str, ...)
{
    va_list arp;
    int d, r, w, s, l, i;

    // Check if only string
    for (i = 0;; i++)
    {
        if (str[i] == '%')
            break;
        if (str[i] == 0)
        {
            USB_VCP_Puts(str);
            return USB_VCP_OK;
        }
    }

    va_start(arp, str);

    while ((d = *str++) != 0)
    {
        if (d != '%')
        {
            USB_VCP_Putc(d);
            continue;
        }
        d = *str++;
        w = r = s = l = 0;
        if (d == '0')
        {
            d = *str++;
            s = 1;
        }
        while ((d >= '0') && (d <= '9'))
        {
            w += w * 10 + (d - '0');
            d = *str++;
        }
        if (s)
            w = -w;
        if (d == 'l')
        {
            l = 1;
            d = *str++;
        }
        if (!d)
            break;
        if (d == 's')
        {
            USB_VCP_Puts(va_arg(arp, char *));
            continue;
        }
        if (d == 'c')
        {
            USB_VCP_Putc((char)va_arg(arp, int));
            continue;
        }
        if (d == 'u')
            r = 10;
        if (d == 'd')
            r = -10;
        if (d == 'X' || d == 'x')
            r = 16;
        if (d == 'b')
            r = 2;
        if (!r)
            break;
        if (l)
        {
            USB_VCP_itoa((long)va_arg(arp, long), r, w);
        }
        else
        {
            if (r > 0)
                USB_VCP_itoa((unsigned long)va_arg(arp, int), r, w);
            else
                USB_VCP_itoa((long)va_arg(arp, int), r, w);
        }
    }

    va_end(arp);
}

USB_VCP_Result USB_VCP_Send(uint8_t *DataArray, uint32_t Length)
{
    /* Send array */
    CDC_DataTx(DataArray, Length);

    /* Return OK */
    return USB_VCP_OK;
}

uint16_t USB_VCP_Gets(char *buffer, uint16_t bufsize)
{
    uint16_t i = 0;
    uint8_t c;

    if (USB_VCP_BufferEmpty() || (!USB_VCP_FindCharacter('\n') && !USB_VCP_BufferFull()))
    {
        return 0;
    }

    /* If available buffer size is more than 0 characters */
    while (i < (bufsize - 1))
    {
        /* We have available data */
        while (USB_VCP_Getc(&c) != USB_VCP_DATA_OK)
            ;
        /* Save new data */
        buffer[i] = (char)c;
        /* Check for end of string */
        if (buffer[i] == '\n')
        {
            i++;
            /* Done */
            break;
        }
        else
        {
            i++;
        }
    }

    /* Add zero to the end of string */
    buffer[i] = 0;

    /* Return number of characters in string */
    return i;
}

USB_VCP_Result USB_VCP_AddReceived(uint8_t c)
{
    /* Still available data in buffer */
    if (usb_vcp_buf_num < USB_VCP_RECEIVE_BUFFER_LENGTH)
    {
        /* Check for overflow */
        if (usb_vcp_buf_in >= USB_VCP_RECEIVE_BUFFER_LENGTH)
        {
            usb_vcp_buf_in = 0;
        }
        /* Add character to buffer */
        USB_VCP_ReceiveBuffer[usb_vcp_buf_in] = c;
        /* Increase counters */
        usb_vcp_buf_in++;
        usb_vcp_buf_num++;

        /* Return OK */
        return USB_VCP_OK;
    }

    /* Return Buffer full */
    return USB_VCP_RECEIVE_BUFFER_FULL;
}

USB_VCP_Result USB_VCP_GetStatus(void)
{
    if (USB_VCP_INT_Init)
    {
        return USB_VCP_INT_Status;
    }
    return USB_VCP_ERROR;
}

USB_VCP_Result USB_VCP_GetSettings(USB_VCP_Settings_t *Settings)
{
    /* Fill data */
    Settings->Baudrate = linecoding.bitrate;
    Settings->DataBits = linecoding.datatype;
    Settings->Parity = linecoding.paritytype;
    Settings->Stopbits = linecoding.format;
    Settings->Changed = linecoding.changed;

    /* Clear changed flag */
    linecoding.changed = 0;

    /* Return OK */
    return USB_VCP_OK;
}