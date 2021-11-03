
#include "rtp.h"
#include "24c02.h"
#include "delay.h"
#include "exti.h"
#include "gui.h"
#include "lcd.h"
#include "math.h"
#include "stdlib.h"
#include "touch.h"

extern u8 touch_flag;

//���败������ȡx,y���������
u8 CMD_RDX = 0XD0;
u8 CMD_RDY = 0X90;

/*****************************************************************************
 * @name       :void RTP_Write_Byte(u8 num)
 * @date       :2018-08-09
 * @function   :Write a byte data to the touch screen IC with SPI bus
 * @parameters :num:Data to be written
 * @retvalue   :None
 ******************************************************************************/
void RTP_Write_Byte(u8 num)
{
    u8 count = 0;
    for (count = 0; count < 8; count++)
    {
        if (num & 0x80)
            TDIN = 1;
        else
            TDIN = 0;
        num <<= 1;
        TCLK = 0;
        delay_us(1);
        TCLK = 1; //��������Ч
    }
}

/*****************************************************************************
 * @name       :u16 RTP_Read_AD(u8 CMD)
 * @date       :2018-08-09
 * @function   :Reading adc values from touch screen IC with SPI bus
 * @parameters :CMD:Read command,0xD0 for x,0x90 for y
 * @retvalue   :Read data
 ******************************************************************************/
u16 RTP_Read_AD(u8 CMD)
{
    u8 count = 0;
    u16 Num = 0;
    TCLK = 0;            //������ʱ��
    TDIN = 0;            //����������
    TCS = 0;             //ѡ�д�����IC
    RTP_Write_Byte(CMD); //����������
    delay_us(6);         // ADS7846��ת��ʱ���Ϊ6us
    TCLK = 0;
    delay_us(1);
    TCLK = 1; //��1��ʱ�ӣ����BUSY
    delay_us(1);
    TCLK = 0;
    for (count = 0; count < 16; count++) //����16λ����,ֻ�и�12λ��Ч
    {
        Num <<= 1;
        TCLK = 0; //�½�����Ч
        delay_us(1);
        TCLK = 1;
        if (DOUT)
            Num++;
    }
    Num >>= 4; //ֻ�и�12λ��Ч.
    TCS = 1;   //�ͷ�Ƭѡ
    return (Num);
}

#define READ_TIMES 5 //��ȡ����
#define LOST_VAL 1   //����ֵ
/*****************************************************************************
 * @name       :u16 RTP_Read_XOY(u8 xy)
 * @date       :2018-08-09
 * @function   :Read the touch screen coordinates (x or y),
                                Read the READ_TIMES secondary data in succession
                                and sort the data in ascending order,
                                Then remove the lowest and highest number of LOST_VAL
                                and take the average
 * @parameters :xy:Read command(CMD_RDX/CMD_RDY)
 * @retvalue   :Read data
******************************************************************************/
u16 RTP_Read_XOY(u8 xy)
{
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum = 0;
    u16 temp;
    for (i = 0; i < READ_TIMES; i++)
        buf[i] = RTP_Read_AD(xy);
    for (i = 0; i < READ_TIMES - 1; i++) //����
    {
        for (j = i + 1; j < READ_TIMES; j++)
        {
            if (buf[i] > buf[j]) //��������
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for (i = LOST_VAL; i < READ_TIMES - LOST_VAL; i++)
        sum += buf[i];
    temp = sum / (READ_TIMES - 2 * LOST_VAL);
    return temp;
}

/*****************************************************************************
 * @name       :u8 RTP_Read_XY(u16 *x,u16 *y)
 * @date       :2018-08-09
 * @function   :Read touch screen x and y coordinates,
                                The minimum value can not be less than 100
 * @parameters :x:Read x coordinate of the touch screen
                                y:Read y coordinate of the touch screen
 * @retvalue   :0-fail,1-success
******************************************************************************/
u8 RTP_Read_XY(u16 *x, u16 *y)
{
    u16 xtemp, ytemp;
    xtemp = RTP_Read_XOY(CMD_RDX);
    ytemp = RTP_Read_XOY(CMD_RDY);
    // if(xtemp<100||ytemp<100)return 0;//����ʧ��
    *x = xtemp;
    *y = ytemp;
    return 1; //�����ɹ�
}

#define ERR_RANGE 50 //��Χ
/*****************************************************************************
 * @name       :u8 TP_Read_XY2(u16 *x,u16 *y)
 * @date       :2018-08-09
 * @function   :Read the touch screen coordinates twice in a row,
                                and the deviation of these two times can not exceed ERR_RANGE,
                                satisfy the condition, then think the reading is correct,
                                otherwise the reading is wrong.
                                This function can greatly improve the accuracy.
 * @parameters :x:Read x coordinate of the touch screen
                                y:Read y coordinate of the touch screen
 * @retvalue   :0-fail,1-success
******************************************************************************/
u8 RTP_Read_XY2(u16 *x, u16 *y)
{
    u16 x1, y1;
    u16 x2, y2;
    u8 flag;
    flag = RTP_Read_XY(&x1, &y1);
    if (flag == 0)
        return (0);
    flag = RTP_Read_XY(&x2, &y2);
    if (flag == 0)
        return (0);
    if (((x2 <= x1 && x1 < x2 + ERR_RANGE) || (x1 <= x2 && x2 < x1 + ERR_RANGE)) //ǰ�����β�����+-50��
        && ((y2 <= y1 && y1 < y2 + ERR_RANGE) || (y1 <= y2 && y2 < y1 + ERR_RANGE)))
    {
        *x = (x1 + x2) / 2;
        *y = (y1 + y2) / 2;
        return 1;
    }
    else
        return 0;
}

/*****************************************************************************
 * @name       :void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
 * @date       :2018-08-09
 * @function   :Draw a touch point,Used to calibrate
 * @parameters :x:Read x coordinate of the touch screen
                                y:Read y coordinate of the touch screen
                                color:the color value of the touch point
 * @retvalue   :None
******************************************************************************/
void RTP_Drow_Touch_Point(u16 x, u16 y, u16 color)
{
    POINT_COLOR = color;
    LCD_DrawLine(x - 12, y, x + 13, y); //����
    LCD_DrawLine(x, y - 12, x, y + 13); //����
    LCD_DrawPoint(x + 1, y + 1);
    LCD_DrawPoint(x - 1, y + 1);
    LCD_DrawPoint(x + 1, y - 1);
    LCD_DrawPoint(x - 1, y - 1);
    gui_circle(x, y, POINT_COLOR, 6, 0); //������Ȧ
}

/*****************************************************************************
 * @name       :void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
 * @date       :2018-08-09
 * @function   :Draw a big point(2*2)
 * @parameters :x:Read x coordinate of the point
                                y:Read y coordinate of the point
                                color:the color value of the point
 * @retvalue   :None
******************************************************************************/
void RTP_Draw_Big_Point(u16 x, u16 y, u16 color)
{
    POINT_COLOR = color;
    LCD_DrawPoint(x, y); //���ĵ�
    LCD_DrawPoint(x + 1, y);
    LCD_DrawPoint(x, y + 1);
    LCD_DrawPoint(x + 1, y + 1);
}

/*****************************************************************************
 * @name       :u8 TP_Scan(u8 tp)
 * @date       :2018-08-09
 * @function   :Scanning touch event
 * @parameters :tp:0-screen coordinate
                                     1-Physical coordinates(For special occasions such as calibration)
 * @retvalue   :Current touch screen status,
                                0-no touch
                                1-touch
******************************************************************************/
u8 RTP_Scan(u8 tp)
{
#if SCAN_TYPE
    u8 irq = 1;
    if (touch_flag)
    {
        irq = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
    }
    if (!irq)
#else
    if (PEN == 0) //�а�������
#endif
    {
        if (tp)
            RTP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0]);      //��ȡ��������
        else if (RTP_Read_XY2(&tp_dev.x[0], &tp_dev.y[0])) //��ȡ��Ļ����
        {
            tp_dev.x[0] = tp_dev.xfac * tp_dev.x[0] + tp_dev.xoff; //�����ת��Ϊ��Ļ����
            tp_dev.y[0] = tp_dev.yfac * tp_dev.y[0] + tp_dev.yoff;
        }
        if ((tp_dev.sta & TP_PRES_DOWN) == 0) //֮ǰû�б�����
        {
            tp_dev.sta = TP_PRES_DOWN | TP_CATH_PRES; //��������
            tp_dev.x[4] = tp_dev.x[0];                //��¼��һ�ΰ���ʱ������
            tp_dev.y[4] = tp_dev.y[0];
        }
    }
    else
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
            tp_dev.x[4] = 0;
            tp_dev.y[4] = 0;
            tp_dev.x[0] = 0xffff;
            tp_dev.y[0] = 0xffff;
        }
    }
    return tp_dev.sta & TP_PRES_DOWN; //���ص�ǰ�Ĵ���״̬
}

//////////////////////////////////////////////////////////////////////////
//������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 40
/*****************************************************************************
 * @name       :void TP_Save_Adjdata(void)
 * @date       :2018-08-09
 * @function   :Save calibration parameters
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void RTP_Save_Adjdata(void)
{
    s32 temp;
    //����У�����!
    temp = tp_dev.xfac * 100000000; //����xУ������
    AT24C02_WriteLenByte(SAVE_ADDR_BASE, temp, 4);
    temp = tp_dev.yfac * 100000000; //����yУ������
    AT24C02_WriteLenByte(SAVE_ADDR_BASE + 4, temp, 4);
    //����xƫ����
    AT24C02_WriteLenByte(SAVE_ADDR_BASE + 8, tp_dev.xoff, 2);
    //����yƫ����
    AT24C02_WriteLenByte(SAVE_ADDR_BASE + 10, tp_dev.yoff, 2);
    //���津������
    AT24C02_WriteOneByte(SAVE_ADDR_BASE + 12, tp_dev.touchtype);
    temp = 0X0A; //���У׼����
    AT24C02_WriteOneByte(SAVE_ADDR_BASE + 13, temp);
}

/*****************************************************************************
 * @name       :u8 TP_Get_Adjdata(void)
 * @date       :2018-08-09
 * @function   :Gets the calibration values stored in the EEPROM
 * @parameters :None
 * @retvalue   :1-get the calibration values successfully
                                0-get the calibration values unsuccessfully and Need to recalibrate
******************************************************************************/
u8 RTP_Get_Adjdata(void)
{
    s32 tempfac;
    tempfac = AT24C02_ReadOneByte(SAVE_ADDR_BASE + 13); //��ȡ�����,���Ƿ�У׼����
    if (tempfac == 0X0A)                                //�������Ѿ�У׼����
    {
        tempfac = AT24C02_ReadLenByte(SAVE_ADDR_BASE, 4);
        tp_dev.xfac = (float)tempfac / 100000000; //�õ�xУ׼����
        tempfac = AT24C02_ReadLenByte(SAVE_ADDR_BASE + 4, 4);
        tp_dev.yfac = (float)tempfac / 100000000; //�õ�yУ׼����
                                                  //�õ�xƫ����
        tp_dev.xoff = AT24C02_ReadLenByte(SAVE_ADDR_BASE + 8, 2);
        //�õ�yƫ����
        tp_dev.yoff = AT24C02_ReadLenByte(SAVE_ADDR_BASE + 10, 2);
        tp_dev.touchtype = AT24C02_ReadOneByte(SAVE_ADDR_BASE + 12); //��ȡ�������ͱ��
        if (tp_dev.touchtype)                                        // X,Y��������Ļ�෴
        {
            CMD_RDX = 0X90;
            CMD_RDY = 0XD0;
        }
        else // X,Y��������Ļ��ͬ
        {
            CMD_RDX = 0XD0;
            CMD_RDY = 0X90;
        }
        return 1;
    }
    return 0;
}

/*****************************************************************************
 * @name       :void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
 * @date       :2018-08-09
 * @function   :Display calibration results
 * @parameters :x0:the x coordinates of first calibration point
                                y0:the y coordinates of first calibration point
                                x1:the x coordinates of second calibration point
                                y1:the y coordinates of second calibration point
                                x2:the x coordinates of third calibration point
                                y2:the y coordinates of third calibration point
                                x3:the x coordinates of fourth calibration point
                                y3:the y coordinates of fourth calibration point
                                fac:calibration factor
 * @retvalue   :None
******************************************************************************/
void RTP_Adj_Info_Show(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 fac)
{
    POINT_COLOR = RED;
    LCD_ShowString(40, 140, 16, "x1:", 1);
    LCD_ShowString(40 + 80, 140, 16, "y1:", 1);
    LCD_ShowString(40, 160, 16, "x2:", 1);
    LCD_ShowString(40 + 80, 160, 16, "y2:", 1);
    LCD_ShowString(40, 180, 16, "x3:", 1);
    LCD_ShowString(40 + 80, 180, 16, "y3:", 1);
    LCD_ShowString(40, 200, 16, "x4:", 1);
    LCD_ShowString(40 + 80, 200, 16, "y4:", 1);
    LCD_ShowString(40, 220, 16, "fac is:", 1);
    LCD_ShowNum(40 + 24, 140, x0, 4, 16, 0);      //��ʾ��ֵ
    LCD_ShowNum(40 + 24 + 80, 140, y0, 4, 16, 0); //��ʾ��ֵ
    LCD_ShowNum(40 + 24, 160, x1, 4, 16, 0);      //��ʾ��ֵ
    LCD_ShowNum(40 + 24 + 80, 160, y1, 4, 16, 0); //��ʾ��ֵ
    LCD_ShowNum(40 + 24, 180, x2, 4, 16, 0);      //��ʾ��ֵ
    LCD_ShowNum(40 + 24 + 80, 180, y2, 4, 16, 0); //��ʾ��ֵ
    LCD_ShowNum(40 + 24, 200, x3, 4, 16, 0);      //��ʾ��ֵ
    LCD_ShowNum(40 + 24 + 80, 200, y3, 4, 16, 0); //��ʾ��ֵ
    LCD_ShowNum(40 + 56, 220, fac, 3, 16, 0); //��ʾ��ֵ,����ֵ������95~105��Χ֮��.
}

/*****************************************************************************
 * @name       :u8 TP_Get_Adjdata(void)
 * @date       :2018-08-09
 * @function   :Calibration touch screen and Get 4 calibration parameters
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void RTP_Adjust(void)
{
    u16 pos_temp[4][2]; //���껺��ֵ
    u8 cnt = 0;
    u16 d1, d2;
    u32 tem1, tem2;
    double fac;
    u16 outtime = 0;
    cnt = 0;
    POINT_COLOR = BLUE;
    BACK_COLOR = WHITE;
    LCD_Clear(WHITE);  //����
    POINT_COLOR = RED; //��ɫ
    LCD_Clear(WHITE);  //����
    POINT_COLOR = BLACK;
    LCD_ShowString(10, 40, 16, "Please use the stylus click", 1); //��ʾ��ʾ��Ϣ
    LCD_ShowString(10, 56, 16, "the cross on the screen.", 1);    //��ʾ��ʾ��Ϣ
    LCD_ShowString(10, 72, 16, "The cross will always move", 1);  //��ʾ��ʾ��Ϣ
    LCD_ShowString(10, 88, 16, "until the screen adjustment", 1); //��ʾ��ʾ��Ϣ
    LCD_ShowString(10, 104, 16, "is completed.", 1);              //��ʾ��ʾ��Ϣ

    RTP_Drow_Touch_Point(20, 20, RED); //����1
    tp_dev.sta = 0;                    //���������ź�
    tp_dev.xfac =
        0; // xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������
    while (1) //�������10����û�а���,���Զ��˳�
    {
        RTP_Scan(1);                             //ɨ����������
        if ((tp_dev.sta & 0xc0) == TP_CATH_PRES) //����������һ��(��ʱ�����ɿ���.)
        {
            outtime = 0;
            tp_dev.sta &= ~(1 << 6); //��ǰ����Ѿ�����������.

            pos_temp[cnt][0] = tp_dev.x[0];
            pos_temp[cnt][1] = tp_dev.y[0];
            cnt++;
            switch (cnt)
            {
            case 1:
                RTP_Drow_Touch_Point(20, 20, WHITE);              //�����1
                RTP_Drow_Touch_Point(lcddev.width - 20, 20, RED); //����2
                break;
            case 2:
                RTP_Drow_Touch_Point(lcddev.width - 20, 20, WHITE); //�����2
                RTP_Drow_Touch_Point(20, lcddev.height - 20, RED);  //����3
                break;
            case 3:
                RTP_Drow_Touch_Point(20, lcddev.height - 20, WHITE);              //�����3
                RTP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, RED); //����4
                break;
            case 4:                                          //ȫ���ĸ����Ѿ��õ�
                                                             //�Ա����
                tem1 = abs(pos_temp[0][0] - pos_temp[1][0]); // x1-x2
                tem2 = abs(pos_temp[0][1] - pos_temp[1][1]); // y1-y2
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,2�ľ���

                tem1 = abs(pos_temp[2][0] - pos_temp[3][0]); // x3-x4
                tem2 = abs(pos_temp[2][1] - pos_temp[3][1]); // y3-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�3,4�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05 || d1 == 0 || d2 == 0) //���ϸ�
                {
                    cnt = 0;
                    RTP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //�����4
                    RTP_Drow_Touch_Point(20, 20, RED);                                  //����1
                    RTP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
                                      pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
                    continue;
                }
                tem1 = abs(pos_temp[0][0] - pos_temp[2][0]); // x1-x3
                tem2 = abs(pos_temp[0][1] - pos_temp[2][1]); // y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,3�ľ���

                tem1 = abs(pos_temp[1][0] - pos_temp[3][0]); // x2-x4
                tem2 = abs(pos_temp[1][1] - pos_temp[3][1]); // y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�2,4�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //���ϸ�
                {
                    cnt = 0;
                    RTP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //�����4
                    RTP_Drow_Touch_Point(20, 20, RED);                                  //����1
                    RTP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
                                      pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
                    continue;
                } //��ȷ��

                //�Խ������
                tem1 = abs(pos_temp[1][0] - pos_temp[2][0]); // x1-x3
                tem2 = abs(pos_temp[1][1] - pos_temp[2][1]); // y1-y3
                tem1 *= tem1;
                tem2 *= tem2;
                d1 = sqrt(tem1 + tem2); //�õ�1,4�ľ���

                tem1 = abs(pos_temp[0][0] - pos_temp[3][0]); // x2-x4
                tem2 = abs(pos_temp[0][1] - pos_temp[3][1]); // y2-y4
                tem1 *= tem1;
                tem2 *= tem2;
                d2 = sqrt(tem1 + tem2); //�õ�2,3�ľ���
                fac = (float)d1 / d2;
                if (fac < 0.95 || fac > 1.05) //���ϸ�
                {
                    cnt = 0;
                    RTP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //�����4
                    RTP_Drow_Touch_Point(20, 20, RED);                                  //����1
                    RTP_Adj_Info_Show(pos_temp[0][0], pos_temp[0][1], pos_temp[1][0], pos_temp[1][1], pos_temp[2][0],
                                      pos_temp[2][1], pos_temp[3][0], pos_temp[3][1], fac * 100); //��ʾ����
                    continue;
                } //��ȷ��
                //������
                tp_dev.xfac = (float)(lcddev.width - 40) / (pos_temp[1][0] - pos_temp[0][0]);       //�õ�xfac
                tp_dev.xoff = (lcddev.width - tp_dev.xfac * (pos_temp[1][0] + pos_temp[0][0])) / 2; //�õ�xoff

                tp_dev.yfac = (float)(lcddev.height - 40) / (pos_temp[2][1] - pos_temp[0][1]);       //�õ�yfac
                tp_dev.yoff = (lcddev.height - tp_dev.yfac * (pos_temp[2][1] + pos_temp[0][1])) / 2; //�õ�yoff
                if (abs(tp_dev.xfac) > 2 || abs(tp_dev.yfac) > 2) //������Ԥ����෴��.
                {
                    cnt = 0;
                    RTP_Drow_Touch_Point(lcddev.width - 20, lcddev.height - 20, WHITE); //�����4
                    RTP_Drow_Touch_Point(20, 20, RED);                                  //����1
                    LCD_ShowString(40, 26, 16, "TP Need readjust!", 1);
                    tp_dev.touchtype = !tp_dev.touchtype; //�޸Ĵ�������.
                    if (tp_dev.touchtype)                 // X,Y��������Ļ�෴
                    {
                        CMD_RDX = 0X90;
                        CMD_RDY = 0XD0;
                    }
                    else // X,Y��������Ļ��ͬ
                    {
                        CMD_RDX = 0XD0;
                        CMD_RDY = 0X90;
                    }
                    continue;
                }
                POINT_COLOR = BLUE;
                LCD_Clear(WHITE);                                          //����
                LCD_ShowString(35, 110, 16, "Touch Screen Adjust OK!", 1); //У�����
                delay_ms(1000);
                RTP_Save_Adjdata();
                LCD_Clear(WHITE); //����
                return;           //У�����
            }
        }
        delay_ms(10);
        outtime++;
        if (outtime > 1000)
        {
            RTP_Get_Adjdata();
            break;
        }
    }
}

/*****************************************************************************
 * @name       :u8 TP_Init(void)
 * @date       :2018-08-09
 * @function   :Initialization touch screen
 * @parameters :None
 * @retvalue   :0-no calibration
                                1-Has been calibrated
******************************************************************************/
u8 RTP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //ע��,ʱ��ʹ��֮��,��GPIO�Ĳ�������Ч
    //��������֮ǰ,����ʹ��ʱ��.����ʵ���������������
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE); //ʹ��GPIOB,C,Fʱ��

    // GPIOB1,2��ʼ������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2; // PB1/PB2 ����Ϊ��������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;           //����ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;         //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;     // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;           //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);                 //��ʼ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     // PB0����Ϊ�������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //���ģʽ
    GPIO_Init(GPIOB, &GPIO_InitStructure);        //��ʼ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_13; // PC13����Ϊ�������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           //���ģʽ
    GPIO_Init(GPIOC, &GPIO_InitStructure);                  //��ʼ��

#if SCAN_TYPE
    Touch_EXTI_Init();
#endif

    RTP_Read_XY(&tp_dev.x[0], &tp_dev.y[0]); //��һ�ζ�ȡ��ʼ��
    if (RTP_Get_Adjdata())
    {
        return 0; //�Ѿ�У׼
    }
    else //δУ׼?
    {
        LCD_Clear(WHITE); //����
        RTP_Adjust();     //��ĻУ׼
        RTP_Save_Adjdata();
    }
    RTP_Get_Adjdata();
    return 1;
}
