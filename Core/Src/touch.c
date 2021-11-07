
#include "touch.h"
#include "ctpiic.h"
#include "ft5426.h"
#include "gt911.h"
#include "rtp.h"

_m_tp_dev tp_dev = {GT911_Scan, 0, 0, 0, 0, 0, 0, 0};

/*****************************************************************************
 * @name       :u8 TP_Init(void)
 * @date       :2020-06-02
 * @function   :Initialization resistance or capacity touch screen
 * @parameters :None
 * @retvalue   :0-successfully
                1-failed
******************************************************************************/
u16 CTP_Read_ID(void)
{
    u8 id_buf[4] = {0};
    CTP_IIC_Init();
    GT911_gpio_init();
    GT911_int_sync(50);
    GT9XX_ReadData(GT9XX_ID_ADDR, 4, id_buf);
    if ((id_buf[0] == 0x39) && (id_buf[1] == 0x31) && (id_buf[2] == 0x31))
    {
        return 0x911;
    }
    FT5426_RD_Reg(FT_ID_G_LIB_VERSION, id_buf, 2);
    if ((id_buf[0] == 0x00) && (id_buf[1] == 0x02))
    {
        return 0x5426;
    }
    return 0xFFFF;
}

/*****************************************************************************
 * @name       :u8 TP_Init(void)
 * @date       :2020-06-02
 * @function   :Initialization resistance or capacity touch screen
 * @parameters :None
 * @retvalue   :0-successfully
                1-failed
******************************************************************************/
u8 TP_Init(void)
{
    u8 ret = 0;
#if TP_TYPE
    if (CTP_Read_ID() == 0x911)
    {
        ret = GT911_Init();
        tp_dev.CPT_Scan = GT911_Scan;
    }
    else if (CTP_Read_ID() == 0x5426)
    {
        ret = FT5426_Init();
        tp_dev.CPT_Scan = FT5426_Scan;
    }
    tp_dev.touchtype |= 0x80;
#else
    ret = RTP_Init();
    tp_dev.touchtype |= 0x00;
#endif
    return ret;
}
