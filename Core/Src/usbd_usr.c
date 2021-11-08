#include "usbd_usr.h"
#include "usart.h"
#include "usb_dcd_int.h"
#include <stdio.h>

vu8 bDeviceState = 0;

extern USB_OTG_CORE_HANDLE USB_OTG_dev;

void OTG_FS_IRQHandler(void)
{
    USBD_OTG_ISR_Handler(&USB_OTG_dev);
}
USBD_Usr_cb_TypeDef USR_cb = {
    USBD_USR_Init,          USBD_USR_DeviceReset,     USBD_USR_DeviceConfigured,   USBD_USR_DeviceSuspended,
    USBD_USR_DeviceResumed, USBD_USR_DeviceConnected, USBD_USR_DeviceDisconnected,
};
void USBD_USR_Init(void)
{
    // PRINT("USBD_USR_Init\r\n");
}
void USBD_USR_DeviceReset(uint8_t speed)
{
    switch (speed)
    {
    case USB_OTG_SPEED_HIGH:
        PRINT("USB Device Library v1.1.0  [HS]\r\n");
        break;
    case USB_OTG_SPEED_FULL:
        PRINT("USB Device Library v1.1.0  [FS]\r\n");
        break;
    default:
        PRINT("USB Device Library v1.1.0  [??]\r\n");
        break;
    }
}
void USBD_USR_DeviceConfigured(void)
{
    PRINT("CDC Interface started.\r\n");
}
void USBD_USR_DeviceSuspended(void)
{
    PRINT("Device In suspend mode.\r\n");
}
void USBD_USR_DeviceResumed(void)
{
    PRINT("Device Resumed\r\n");
}
void USBD_USR_DeviceConnected(void)
{
    bDeviceState = 1;
    PRINT("USB Device Connected.\r\n");
}
void USBD_USR_DeviceDisconnected(void)
{
    bDeviceState = 0;
    PRINT("USB Device Disconnected.\r\n");
}
