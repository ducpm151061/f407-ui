#ifndef __GUI_H__
#define __GUI_H__
#include "sys.h"
void GUI_DrawPoint(u16 x, u16 y, u16 color);
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawLine2(u16 x1, u16 y1, u16 x2, u16 y2, u16 size, u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void Draw_Circle(u16 x0, u16 y0, u16 fc, u8 r);
void Draw_Triangel(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2);
void Fill_Triangel(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_ShowChar(u16 x, u16 y, u16 fc, u16 bc, u8 num, u8 size, u8 mode);
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 flag);
void LCD_Show2Num(u16 x, u16 y, u16 num, u8 len, u8 size, u8 mode);
void LCD_ShowString(u16 x, u16 y, u8 size, u8 *p, u8 mode);
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s, u8 mode);
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s, u8 mode);
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s, u8 mode);
// void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str, u8 size, u8 mode);
void Gui_Drawbmp16(u16 x, u16 y, u16 pwidth, u16 pheight, const unsigned char *p); //��ʾͼƬ
void gui_circle(int xc, int yc, u16 c, int r, int fill);
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str, u8 size, u8 mode);
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Draw9Point(uint16_t x, uint16_t y, uint16_t color);
void Draw_TextBox(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);
void Draw_Button(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);
void EscButton(uint16_t xstart, uint16_t ystart, uint8_t xend, uint16_t yend);
void SetButton(uint8_t xstart, uint16_t ystart, uint8_t xend, uint16_t yend);
void Draw_DirectButton(uint16_t xstart, uint16_t ystart);
void Draw_Window(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend, uint8_t *caption);
#endif
