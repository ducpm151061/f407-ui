#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"

#define TP_TYPE 1

#define SCAN_TYPE 1

#define CTP_MAX_TOUCH 5

#define TP_PRES_DOWN 0x80
#define TP_CATH_PRES 0x40

typedef struct
{
    u8 (*CPT_Scan)(void);
    u16 x[CTP_MAX_TOUCH];
    u16 y[CTP_MAX_TOUCH];
    u8 sta;
    float xfac;
    float yfac;
    short xoff;
    short yoff;
    u8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev;

u8 TP_Init(void);
#endif /* __TOUCH_H__ */
