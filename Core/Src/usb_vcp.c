#include "usb_vcp.h"
#include "usbd_usr.h"

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

USB_VCP_Result USB_VCP_Puts(char *str)
{
    while (*str)
    {
        USB_VCP_Putc(*str++);
    }

    /* Return OK */
    return USB_VCP_OK;
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

    /* Check for any data on USART */
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