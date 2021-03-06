/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "Utilities.h"
#include "stdlib.h"
extern System_t xSystem;
extern uint32_t RecordCount;
static int offset = 0;
static int take = 100;

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_HID_Init(void *pdev,
                             uint8_t cfgidx);

static uint8_t USBD_HID_DeInit(void *pdev,
                               uint8_t cfgidx);

static uint8_t USBD_HID_Setup(void *pdev,
                              USB_SETUP_REQ *req);

static uint8_t *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length);

static uint8_t USBD_HID_DataIn(void *pdev, uint8_t epnum);
static uint8_t USBD_HID_DataOut(void *pdev, uint8_t epnum);
uint8_t rxBuff[64];
uint8_t txBuff[64];
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_Class_cb_TypeDef USBD_HID_cb =
    {
        USBD_HID_Init,
        USBD_HID_DeInit,
        USBD_HID_Setup,
        NULL,             /*EP0_TxSent*/
        NULL,             /*EP0_RxReady*/
        USBD_HID_DataIn,  /*DataIn*/
        USBD_HID_DataOut, /*DataOut*/
        NULL,             /*SOF */
        NULL,
        NULL,
        USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE
        USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t USBD_HID_AltSet __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t USBD_HID_Protocol __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint32_t USBD_HID_IdleState __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
    {
        0x09,                              /* bLength: Configuration Descriptor size */
        USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
        USB_HID_CONFIG_DESC_SIZ,
        /* wTotalLength: Bytes returned */
        0x00,
        0x01, /*bNumInterfaces: 1 interface*/
        0x01, /*bConfigurationValue: Configuration value*/
        0x00, /*iConfiguration: Index of string descriptor describing
  the configuration*/
        0xC0, /*bmAttributes: bus powered and Support Remote Wake-up */
        0x32, /*MaxPower 100 mA: this current is used for detecting Vbus*/

        /************** Descriptor of Joystick Mouse interface ****************/
        /* 09 */
        0x09,                          /*bLength: Interface Descriptor size*/
        USB_INTERFACE_DESCRIPTOR_TYPE, /*bDescriptorType: Interface descriptor type*/
        0x00,                          /*bInterfaceNumber: Number of Interface*/
        0x00,                          /*bAlternateSetting: Alternate setting*/
        0x02,                          /*bNumEndpoints*/
        0x03,                          /*bInterfaceClass: HID*/
        0x00,                          /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
        0x00,                          /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
        0,                             /*iInterface: Index of string descriptor*/
        /******************** Descriptor of Joystick Mouse HID ********************/
        /* 18 */
        0x09,                /*bLength: HID Descriptor size*/
        HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
        0x10,                /*bcdHID: HID Class Spec release number*/
        0x01,
        0x00,                       /*bCountryCode: Hardware target country*/
        0x01,                       /*bNumDescriptors: Number of HID class descriptors to follow*/
        0x22,                       /*bDescriptorType*/
        HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
        0x00,
        /******************** Descriptor of Mouse endpoint ********************/
        /* 27 */
        0x07,                         /*bLength: Endpoint Descriptor size*/
        USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/

        0x81, /*bEndpointAddress: Endpoint Address (IN)*/
        0x03, /*bmAttributes: Interrupt endpoint*/
        0x40, /*wMaxPacketSize: 64 Byte max */
        0x00,
        0x0A,                         /*bInterval: Polling Interval (10 ms)*/
        0x07,                         /* bLength: Endpoint Descriptor size */
        USB_ENDPOINT_DESCRIPTOR_TYPE, /* bDescriptorType: */
        /*  Endpoint descriptor type */
        0x01, /* bEndpointAddress: */
        /*  Endpoint Address (OUT) */
        0x03, /* bmAttributes: Interrupt endpoint */
        0x40, /* wMaxPacketSize: 64 Bytes max  */
        0x00,
        0x0A, /* bInterval: Polling Interval (20 ms) */
              /* 41 */
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] __ALIGN_END =
    {
        /* 18 */
        0x09,                /*bLength: HID Descriptor size*/
        HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
        0x11,                /*bcdHID: HID Class Spec release number*/
        0x01,
        0x00,                       /*bCountryCode: Hardware target country*/
        0x01,                       /*bNumDescriptors: Number of HID class descriptors to follow*/
        0x22,                       /*bDescriptorType*/
        HID_MOUSE_REPORT_DESC_SIZE, /*wItemLength: Total length of Report descriptor*/
        0x00,
};
#endif

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined(__ICCARM__) /*!< IAR Compiler */
#pragma data_alignment = 4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
    {
        0x06, 0x00, 0xFF, // Usage Page = 0xFF00 (Vendor Defined Page 1)
        0x09, 0x01,       // Usage (Vendor Usage 1)
        0xA1, 0x01,       // Collection (Application)
        0x19, 0x01,       //      Usage Minimum
        0x29, 0x40,       //      Usage Maximum     //64 input usages total (0x01 to 0x40)
        0x15, 0x01,       //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
        0x25, 0x40,       //      Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
        0x75, 0x08,       //      Report Size: 8-bit field size
        0x95, 0x40,       //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
        0x81, 0x00,       //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
        0x19, 0x01,       //      Usage Minimum
        0x29, 0x40,       //      Usage Maximum     //64 output usages total (0x01 to 0x40)
        0x91, 0x00,       //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
        0xC0};

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_Init(void *pdev,
                             uint8_t cfgidx)
{

  /* Open EP IN */
  DCD_EP_Open(pdev,
              0x81,
              64,
              USB_OTG_EP_INT);

  /* Open EP OUT */
  DCD_EP_Open(pdev,
              0x01,
              64,
              USB_OTG_EP_INT);

  DCD_EP_PrepareRx(pdev, 0x01, rxBuff, 64);
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_DeInit(void *pdev,
                               uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close(pdev, 0x81);
  DCD_EP_Close(pdev, 0x01);

  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_HID_Setup(void *pdev,
                              USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t *pbuf = NULL;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS:
    switch (req->bRequest)
    {

    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;

    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData(pdev,
                       (uint8_t *)&USBD_HID_Protocol,
                       1);
      break;

    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;

    case HID_REQ_GET_IDLE:
      USBD_CtlSendData(pdev,
                       (uint8_t *)&USBD_HID_IdleState,
                       1);
      break;

    default:
      USBD_CtlError(pdev, req);
      return USBD_FAIL;
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      if (req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
        pbuf = USBD_HID_Desc;
#else
        pbuf = USBD_HID_CfgDesc + 0x12;
#endif
        len = MIN(USB_HID_DESC_SIZ, req->wLength);
      }

      USBD_CtlSendData(pdev,
                       pbuf,
                       len);

      break;

    case USB_REQ_GET_INTERFACE:
      USBD_CtlSendData(pdev,
                       (uint8_t *)&USBD_HID_AltSet,
                       1);
      break;

    case USB_REQ_SET_INTERFACE:
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport(USB_OTG_CORE_HANDLE *pdev,
                            uint8_t *report,
                            uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED)
  {
    DCD_EP_Tx(pdev, 0x81, report, len);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length)
{
  *length = sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */

static uint8_t USBD_HID_DataIn(void *pdev,
                               uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
        be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, 0x81);
  return USBD_OK;
}

uint8_t USBD_HID_DataOut(void *pdev,
                         uint8_t epnum)
{
  DCD_EP_PrepareRx(pdev, 0x81, rxBuff, 64);

  return USBD_OK;
}

uint8_t checkCommand(USB_OTG_CORE_HANDLE *pdev)
{
  char *tokens[6];
  uint8_t i = 0;
  uint8_t command[64];
  sprintf((char *)command, "%s", "@AB,1234567890,888888,record");
  if (strncmp((char *)rxBuff, (char *)command, 28) == 0)
  {
    char *token = strtok((char *)rxBuff, ",");
    while (token != NULL)
    {
      tokens[i++] = token;
      token = strtok(NULL, ",");
    }
    offset = atoi(tokens[4]);
    take = atoi(tokens[5]);
    return 1;
  }
  else
  {
    return 0;
  }
}

void sendFlashData(USB_OTG_CORE_HANDLE *pdev)
{
  /* TODO: rewrite
  xSystem.GLStatus.DoneSend = 0;
  uint8_t data_send[128];
  uint8_t Buff[10];
  uint8_t command[64];
  int32_t index_store = GetIndexStore();
  uint32_t index_view = 0;
  uint32_t j;

  index_view = (RecordCount % 1000) - 1;

  //DCD_EP_PrepareRx(pdev, 0x81, rxBuff, 64);
  //sprintf((char *)data_send, "%s", "@123456789@123456789@123456789@123456789@123456789@123456789@123456789@123456789@123456789@123456789aaaaa!!!\r\n");
  sprintf((char *)command, "%s", "@AB,1234567890,888888,record,all");
  DEBUG_PRINT("dataSend %s\r\n", data_send);
  DEBUG_PRINT("rxBuff: %s\r\n", (char *)(rxBuff));
  DEBUG_PRINT("record index: %lu\r\n", GetIndexStore());
  DEBUG_PRINT("offset: %d\r\n", offset);
  DEBUG_PRINT("take: %d\r\n", take);
  if (index_view == 0)
    return;
  if (index_view == -1)
  {
    index_view = (RecordCount % 1000) - 1;
  }
  if (offset + take <= index_view)
  {
    for (j = offset; j < offset + take; j++)
    {
      memset(data_send, 0, sizeof(data_send));
      Record_Read(index_view - j);
      snprintf((char *)data_send, sizeof(data_send), "%d,%s,%s,%s,%d:%d:%d-%d/%d/%d,%s,%s,%s,%s,%s,%s\n", xSystem.Record.Name.IndexStore, (char *)xSystem.Record.Name.NguoiThucHien, (char *)xSystem.Record.Name.DonViCongTac, (char *)xSystem.Record.Name.NguoiLamChung, xSystem.Record.Name.Time.Hour, xSystem.Record.Name.Time.Minute, xSystem.Record.Name.Time.Second, xSystem.Record.Name.Time.Day, xSystem.Record.Name.Time.Month, xSystem.Record.Name.Time.Year, (char *)xSystem.Record.Name.DiaDiemKiemTra, ConvertFloatToString(xSystem.Record.Name.KinhDo.value, (char *)Buff, sizeof(Buff), 4), ConvertFloatToString(xSystem.Record.Name.ViDo.value, (char *)Buff, sizeof(Buff), 4), (char *)xSystem.Record.Name.TenLaiXe, (char *)xSystem.Record.Name.BienSo, ConvertFloatToString(xSystem.Record.Name.KetQua.value, (char *)Buff, sizeof(Buff), 2));
      USBD_HID_SendReport(pdev, data_send, 128);
      // while (USBD_HID_SendReport(pdev, data_send, 128) != USBD_OK)
      // {
      // }
    }
  }

  xSystem.GLStatus.DoneSend = 1;
  // Flush bo dem RX
  rxBuff[0] = 0x00;
  DEBUG_PRINT("\rSend data successful!!!!!\r\n");
  // DCD_EP_Flush(pdev, 0x81);
  */
}
/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
