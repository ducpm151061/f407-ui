#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"

#define TP_TYPE 1

#define SCAN_TYPE 1

#define CTP_MAX_TOUCH 1

#define TP_PRES_DOWN 0x80 //����������
#define TP_CATH_PRES 0x40 //�а���������

typedef struct
{
    u8 (*CPT_Scan)(void); //���ݴ���ɨ�躯��
    u16 x[CTP_MAX_TOUCH]; //��ǰ����(�˴�ɨ��ʱ,����������)
    u16 y[CTP_MAX_TOUCH];
    u8 sta; //�ʵ�״̬
            // b7:����1/�ɿ�0;
    // b6:0,û�а�������;1,�а�������.
    // b5:����
    // b4~b0:���ݴ��������µĵ���(0,��ʾδ����,1��ʾ����)
    ////////////////////////������������У׼����/////////////////////////
    float xfac;
    float yfac;
    short xoff;
    short yoff;
    // b0~6:����.
    // b7:0,������
    //    1,������
    u8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev; //������������touch.c���涨��

u8 TP_Init(void);
#endif
