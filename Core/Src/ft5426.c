#include "ft5426.h"
#include "ctpiic.h"
#include "delay.h"
#include "exti.h"
#include "lcd.h"
#include "string.h"
#include "touch.h"

extern u8 touch_flag;

/*****************************************************************************
 * @name       :u8 FT5426_WR_Reg(u16 reg,u8 *buf,u8 len)
 * @date       :2020-05-13
 * @function   :Write data to ft5426 once
 * @parameters :reg:Start register address for written
                                buf:the buffer of data written
                                len:Length of data written
 * @retvalue   :0-Write succeeded
                                1-Write failed
******************************************************************************/
u8 FT5426_WR_Reg(u16 reg, u8 *buf, u8 len)
{
    u8 i;
    u8 ret = 0;
    CTP_IIC_Start();
    CTP_IIC_Send_Byte(FT_CMD_WR); //����д����
    CTP_IIC_Wait_Ack();
    CTP_IIC_Send_Byte(reg & 0XFF); //���͵�8λ��ַ
    CTP_IIC_Wait_Ack();
    for (i = 0; i < len; i++)
    {
        CTP_IIC_Send_Byte(buf[i]); //������
        ret = CTP_IIC_Wait_Ack();
        if (ret)
        {
            break;
        }
    }
    CTP_IIC_Stop(); //����һ��ֹͣ����
    return ret;
}

/*****************************************************************************
 * @name       :void FT5426_RD_Reg(u16 reg,u8 *buf,u8 len)
 * @date       :2020-05-13
 * @function   :Read data to ft5426 once
 * @parameters :reg:Start register address for read
                                buf:the buffer of data read
                                len:Length of data read
 * @retvalue   :none
******************************************************************************/
void FT5426_RD_Reg(u16 reg, u8 *buf, u8 len)
{
    u8 i;
    CTP_IIC_Start();
    CTP_IIC_Send_Byte(FT_CMD_WR); //����д����
    CTP_IIC_Wait_Ack();
    CTP_IIC_Send_Byte(reg & 0XFF); //���͵�8λ��ַ
    CTP_IIC_Wait_Ack();
    CTP_IIC_Start();
    CTP_IIC_Send_Byte(FT_CMD_RD); //���Ͷ�����
    CTP_IIC_Wait_Ack();
    for (i = 0; i < len; i++)
    {
        buf[i] = CTP_IIC_Read_Byte(i == (len - 1) ? 0 : 1); //������
    }
    CTP_IIC_Stop(); //����һ��ֹͣ����
}

/*****************************************************************************
 * @name       :u8 FT5426_Init(void)
 * @date       :2020-05-13
 * @function   :Initialize the ft5426 touch screen
 * @parameters :none
 * @retvalue   :0-Initialization successful
                                1-initialization failed
******************************************************************************/
u8 FT5426_Init(void)
{
    u8 temp[2];
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE); //ʹ��GPIOB,Cʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;          // PB1����Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;       //����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);             //��ʼ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;    // PC13����Ϊ�������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //���ģʽ
    GPIO_Init(GPIOC, &GPIO_InitStructure);        //��ʼ��
#if SCAN_TYPE
    Touch_EXTI_Init();
#endif
    CTP_IIC_Init(); //��ʼ����������I2C����
    FT_RST = 0;     //��λ
    delay_ms(20);
    FT_RST = 1; //�ͷŸ�λ
    delay_ms(50);
    temp[0] = 0;
    FT5426_WR_Reg(FT_DEVIDE_MODE, temp, 1); //������������ģʽ
#if SCAN_TYPE
    temp[0] = 1;
#endif
    FT5426_WR_Reg(FT_ID_G_MODE, temp, 1);    //��ѯģʽ
    temp[0] = 22;                            //������Чֵ��22��ԽСԽ����
    FT5426_WR_Reg(FT_ID_G_THGROUP, temp, 1); //���ô�����Чֵ
    temp[0] = 12;                            //�������ڣ�����С��12�����14
    FT5426_WR_Reg(FT_ID_G_PERIODACTIVE, temp, 1);
    //��ȡ�汾�ţ��ο�ֵ��0x3003
    FT5426_RD_Reg(FT_ID_G_LIB_VERSION, &temp[0], 2);
    if (temp[1] == 0X02) //�汾:0X3003
    {
        //	printf("CTP ID:%x\r\n",((u16)temp[0]<<8)+temp[1]);
        return 0;
    }
    return 1;
}

const u16 FT5426_TPX_TBL[5] = {FT_TP1_REG, FT_TP2_REG, FT_TP3_REG, FT_TP4_REG, FT_TP5_REG};

/*****************************************************************************
 * @name       :u8 FT5426_Scan(void)
 * @date       :2020-05-13
 * @function   :Scan touch screen (query mode)
 * @parameters :none
 * @retvalue   :Current touch screen status
                                0-No touch
                                1-With touch
******************************************************************************/
u8 FT5426_Scan(void)
{
    u8 buf[4];
    u8 i = 0;
    u8 res = 0;
    u8 temp;
    u8 mode;
#if SCAN_TYPE
    if (touch_flag)
#else
    static u8 t = 0; //���Ʋ�ѯ���,�Ӷ�����CPUռ����
    t++;
    if ((t % 10) == 0 || t < 10) //����ʱ,ÿ����10��CTP_Scan�����ż��1��,�Ӷ���ʡCPUʹ����
#endif
    {
        FT5426_RD_Reg(FT_REG_NUM_FINGER, &mode, 1); //��ȡ�������״̬
        if ((mode & 0XF) && ((mode & 0XF) < 6))
        {
            temp = 0XFF << (mode & 0XF); //����ĸ���ת��Ϊ1��λ��,ƥ��tp_dev.sta����
            tp_dev.sta = (~temp) | TP_PRES_DOWN | TP_CATH_PRES;
            for (i = 0; i < CTP_MAX_TOUCH; i++)
            {
                if (tp_dev.sta & (1 << i)) //������Ч?
                {
                    FT5426_RD_Reg(FT5426_TPX_TBL[i], buf, 4); //��ȡXY����ֵ
                    switch (lcddev.dir)
                    {
                    case 0:
                        tp_dev.x[i] = lcddev.width - (((u16)(buf[0] & 0X0F) << 8) + buf[1]);
                        tp_dev.y[i] = ((u16)(buf[2] & 0X0F) << 8) + buf[3];
                        break;
                    case 1:
                        tp_dev.y[i] = ((u16)(buf[0] & 0X0F) << 8) + buf[1];
                        tp_dev.x[i] = ((u16)(buf[2] & 0X0F) << 8) + buf[3];
                        break;
                    case 2:
                        tp_dev.x[i] = ((u16)(buf[0] & 0X0F) << 8) + buf[1];
                        tp_dev.y[i] = lcddev.height - (((u16)(buf[2] & 0X0F) << 8) + buf[3]);
                        break;
                    case 3:
                        tp_dev.y[i] = lcddev.height - (((u16)(buf[0] & 0X0F) << 8) + buf[1]);
                        tp_dev.x[i] = lcddev.width - (((u16)(buf[2] & 0X0F) << 8) + buf[3]);
                        break;
                    }
                    if ((buf[0] & 0XF0) != 0X80)
                        tp_dev.x[i] = tp_dev.y[i] = 0; //������contact�¼�������Ϊ��Ч
                                                       // printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
                }
            }
            res = 1;
            if (tp_dev.x[0] == 0 && tp_dev.y[0] == 0)
                mode = 0; //���������ݶ���0,����Դ˴�����
#if !SCAN_TYPE
            t = 0; //����һ��,��������������10��,�Ӷ����������
#endif
        }
    }
    if ((mode & 0X1F) == 0) //�޴����㰴��
    {
#if SCAN_TYPE
        touch_flag = 0;
#endif
        if (tp_dev.sta & TP_PRES_DOWN) //֮ǰ�Ǳ����µ�
        {
            tp_dev.sta &= ~(1 << 7); //��ǰ����ɿ�
        }
        else //֮ǰ��û�б�����
        {
            tp_dev.x[0] = 0xffff;
            tp_dev.y[0] = 0xffff;
            tp_dev.sta &= 0XE0; //�������Ч���
        }
    }
#if !SCAN_TYPE
    if (t > 240)
        t = 10; //���´�10��ʼ����
#endif
    return res;
}
