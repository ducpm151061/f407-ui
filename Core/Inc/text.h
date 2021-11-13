#ifndef __TEXT_H__
#define __TEXT_H__
#include "fontupd.h"
#include <stm32f4xx.h>

void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size);
void Show_Font(u16 x, u16 y, char *font, u8 size, u8 mode);
void Show_Str_Mid(u16 x, u16 y, char *str, u8 size, u8 len);
#endif /* __TEXT_H__ */
