#ifndef USB_VCP_H
#define USB_VCP_H 120

#include "misc.h"
#include "stm32f4xx.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
/* Parts of USB device */
#include "usb_cdc.h"
#include "usb_conf.h"
#include "usbd_cdc_core.h"
#include "usbd_desc.h"

/**
 * @defgroup USB_VCP_Macros
 * @brief    Library defines
 * @{
 */

/**
 * @brief  Default buffer length
 * @note   Increase this value if you need more memory for VCP receive data
 */
#ifndef USB_VCP_RECEIVE_BUFFER_LENGTH
#define USB_VCP_RECEIVE_BUFFER_LENGTH 256
#endif

/**
 * @}
 */

/**
 * @defgroup USB_VCP_Typedefs
 * @brief    Library Typedefs
 * @{
 */

/**
 * @brief VCP Result Enumerations
 */
typedef enum
{
    USB_VCP_OK,                  /*!< Everything ok */
    USB_VCP_ERROR,               /*!< An error occurred */
    USB_VCP_RECEIVE_BUFFER_FULL, /*!< Receive buffer is full */
    USB_VCP_DATA_OK,             /*!< Data OK */
    USB_VCP_DATA_EMPTY,          /*!< Data empty */
    USB_VCP_NOT_CONNECTED,       /*!< Not connected to PC */
    USB_VCP_CONNECTED,           /*!< Connected to PC */
    USB_VCP_DEVICE_SUSPENDED,    /*!< Device is suspended */
    USB_VCP_DEVICE_RESUMED       /*!< Device is resumed */
} USB_VCP_Result;

/**
 * @brief  Structure for USART if you are working USB/UART converter with STM32F4xx
 */
typedef struct
{
    uint32_t Baudrate; /*!< Baudrate, which is set by user on terminal.
                            Value is number of bits per second, for example: 115200 */
    uint8_t Stopbits;  /*!< Stop bits, which is set by user on terminal.
                            Possible values:
                               - 0: 1 stop bit
                               - 1: 1.5 stop bits
                               - 2: 2 stop bits */
    uint8_t DataBits;  /*!< Data bits, which is set by user on terminal.
                            Possible values:
                               - 5: 5 data bits
                               - 6: 6 data bits
                               - 7: 7 data bits
                               - 8: 8 data bits
                               - 9: 9 data bits */
    uint8_t Parity;    /*!< Parity, which is set by user on terminal.
                            Possible values:
                               - 0: No parity
                               - 1: Odd parity
                               - 2: Even parity
                               - 3: Mark parity
                               - 4: Space parity */
    uint8_t Changed;   /*!< When you check for settings in my function,
                            this will be set to 1 if user has changed parameters,
                            so you can reinitialize USART peripheral if you need to. */
} USB_VCP_Settings_t;

/**
 * @}
 */

/**
 * @defgroup USB_VCP_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Initializes USB VCP
 * @param  None
 * @retval USB_VCP_OK
 */
USB_VCP_Result USB_VCP_Init(void);

/**
 * @brief  Reads settings from user
 * @note   These settings are set in terminal on PC
 * @param  *Settings: Pointer to USB_VCP_Settings_t structure where to save data
 * @retval USB_VCP_OK
 */
USB_VCP_Result USB_VCP_GetSettings(USB_VCP_Settings_t *Settings);

/**
 * @brief  Gets received character from internal buffer
 * @param  *c: pointer to store new character to
 * @retval Character status:
 *            - USB_VCP_DATA_OK: Character is valid inside *c_str
 *            - USB_VCP_DATA_EMPTY: No character in *c
 */
USB_VCP_Result USB_VCP_Getc(uint8_t *c);

/**
 * @brief  Puts character to USB VCP
 * @param  c: character to send over USB
 * @retval USB_VCP_OK
 */
USB_VCP_Result USB_VCP_Putc(volatile char c);

/**
 * @brief  Gets string from VCP port
 *
 * @note   To use this method, you have to send \n (0x0D) at the end of your string,
 *         otherwise data can be lost and you will fall in infinite loop.
 * @param  *buffer: Pointer to buffer variable where to save string
 * @param  bufsize: Maximum buffer size
 * @retval Number of characters in buffer:
 *            - 0: String not valid
 *            - > 0: String valid, number of characters inside string
 */
uint16_t USB_VCP_Gets(char *buffer, uint16_t bufsize);

/**
 * @brief  Puts string to USB VCP
 * @param  *str: Pointer to string variable
 * @retval USB_VCP_OK
 */
USB_VCP_Result USB_VCP_Puts(const char *str);

/**
 * @brief  Sends array of data to USB VCP
 * @param  *DataArray: Pointer to 8-bit data array to be sent over USB
 * @param  Length: Number of elements to sent in units of bytes
 * @retval Sending status
 */
USB_VCP_Result USB_VCP_Send(uint8_t *DataArray, uint32_t Length);

/**
 * @brief  Gets VCP status
 * @param  None
 * @retval Device status:
 *            - USB_VCP_CONNECTED: Connected to computer
 *            - other: Not connected and not ready to communicate
 */
USB_VCP_Result USB_VCP_GetStatus(void);

/**
 * @brief  Checks if receive buffer is empty
 * @param  None
 * @retval Buffer status:
 *            - 0: Buffer is not empty
 *            - > 0: Buffer is empty
 */
uint8_t USB_VCP_BufferEmpty(void);

/**
 * @brief  Checks if receive buffer is full
 * @param  None
 * @retval Buffer status:
 *            - 0: Buffer is not full
 *            - > 0: Buffer is full
 */
uint8_t USB_VCP_BufferFull(void);

/**
 * @brief  Checks if character is in buffer
 * @param  c: Character to be checked if available in buffer
 * @retval Character status:
 *            - 0: Character is not in buffer
 *            - > 0: Character is in buffer
 */
uint8_t USB_VCP_FindCharacter(volatile char c);

USB_VCP_Result USB_VCP_itoa(long val, int radix, int len);

USB_VCP_Result USB_VCP_Printf(const char *str, ...);

/* Internal functions */
extern USB_VCP_Result USB_VCP_AddReceived(uint8_t c);

#define USB_PRINT(...) USB_VCP_Printf(__VA_ARGS__)

#endif
