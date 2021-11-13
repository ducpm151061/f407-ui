#ifndef __USB_CDC_H
#define __USB_CDC_H

#include "usbd_cdc_core.h"
#include "usbd_conf.h"

uint16_t CDC_DataTx(uint8_t *Buf, uint32_t Len);
uint16_t CDC_DataRx(uint8_t *Buf, uint32_t Len);

typedef struct
{
    uint32_t bitrate;
    uint8_t format;
    uint8_t paritytype;
    uint8_t datatype;
    uint8_t changed;
} LINE_CODING;

#define DEFAULT_CONFIG 0
#define OTHER_CONFIG 1

#endif /* __USB_CDC_H */
