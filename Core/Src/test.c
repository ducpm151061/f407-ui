#include "test.h"

u16 ColorTab[5] = {RED, GREEN, BLUE, YELLOW, BRED};
u16 ColornTab[8] = {RED, MAGENTA, GREEN, DARKBLUE, BLUE, BLACK, LIGHTGREEN};
const u16 POINT_COLOR_TBL[CTP_MAX_TOUCH] = {RED, GREEN, BLUE, BROWN, GRED};

static void Ctp_Test(void)
{
    u8 t = 0;
    u8 i = 0, j = 0;
    u16 lastpos[5][2] = {0};
    LCD_ShowString(0, 0, 16, "RST", 1);
    POINT_COLOR = RED;
    while (1)
    {
        j++;
        tp_dev.CPT_Scan();
        for (t = 0; t < CTP_MAX_TOUCH; t++)
        {
            if ((tp_dev.sta) & (1 << t))
            {
                printf("tp_dev.sta: %d\r\n", tp_dev.sta);
                if (tp_dev.x[t] < lcddev.width && tp_dev.y[t] < lcddev.height)
                {
                    if (lastpos[t][0] == 0XFFFF)
                    {
                        lastpos[t][0] = tp_dev.x[t];
                        lastpos[t][1] = tp_dev.y[t];
                    }
                    if (tp_dev.x[t] > (0) && tp_dev.y[t] < 16)
                    {
                        if (j > 1)
                        {
                            continue;
                        }
                        LCD_ShowString(0, 0, 16, (char *)"RST", 1);
                        POINT_COLOR = RED;
                    }
                    else
                    {
                        LCD_DrawLine2(lastpos[t][0], lastpos[t][1], tp_dev.x[t], tp_dev.y[t], 2, POINT_COLOR_TBL[t]);
                    }
                    lastpos[t][0] = tp_dev.x[t];
                    lastpos[t][1] = tp_dev.y[t];
                }
            }
            else
                lastpos[t][0] = 0XFFFF;
        }
        delay_ms(5);
        i++;
        if (i % 30 == 0)
            LED0 = !LED0;
        if (j > 4)
        {
            j = 0;
        }
    }
}

static void W25Q64_Test()
{
    u8 send_test[] = "STM32F407VxT6 SPI TEST";
    u8 rece_buf[sizeof(send_test)];
    W25Q64_Write((u8 *)send_test, FLASH_SIZE - 64, sizeof(send_test));
    delay_ms(100);
    W25Q64_Read(rece_buf, FLASH_SIZE - 64, sizeof(send_test));
    PRINT("read data: %s\r\n", rece_buf);
}

static void AT24C02_Test()
{
    u8 send_test[] = "STM32F407VxT6 IIC TEST";
    u8 rece_buf[sizeof(send_test)];
    AT24C02_Write(0, (u8 *)send_test, sizeof(send_test));
    delay_ms(100);
    AT24C02_Read(0, rece_buf, sizeof(send_test));
    PRINT("read data: %s\r\n", rece_buf);
}

static void WS_AT45DBXX_Test()
{
    u8 ret[10];
    AT45DBXX_Read_ID(ret);
    PRINT("WS_AT45DBXX ID: %s\r\n", (char *)ret);
}

static void Check_SDCard(void)
{
    while (SD_Init())
    {
        PRINT("SD_Init() error!\r\n");
    }
    PRINT("SD_Init() ok!\r\n");
}

static void Check_Font(void)
{
    u8 key = 0;
    POINT_COLOR = RED;
    BACK_COLOR = WHITE;
    while (font_init())
    {
        key = KEY_Scan(0);
        LCD_ShowString(lcddev.width / 2 - 44, 90, 16, (char *)"Font Error!", 1);
        delay_ms(200);
        LCD_Fill(lcddev.width / 2 - 44, 90, lcddev.width / 2 + 44, 106, WHITE);
        delay_ms(200);
        LCD_ShowString(lcddev.width / 2 - 104, 120, 16, (char *)"press KEY0 to update Font!", 1);
        if (key == KEY0_PRES)
        {
            LCD_Fill(lcddev.width / 2 - 104, 120, lcddev.width / 2 + 104, 136, WHITE);
            while (update_font(lcddev.width / 2 - 100, 120, 16, (u8 *)"0:"))
            {
                LCD_ShowString(lcddev.width / 2 - 76, 120, 16, (char *)"Font Update Failed!", 1);
                delay_ms(200);
                LCD_Fill(lcddev.width / 2 - 76, 120, lcddev.width / 2 + 76, 136, WHITE);
                delay_ms(200);
            }
            POINT_COLOR = BLUE;
            LCD_ShowString(lcddev.width / 2 - 100, 120, 16, (char *)"  Font Update Success!  ", 0);
        }
    }
    POINT_COLOR = BLUE;
    LCD_ShowString(lcddev.width / 2 - 28, 90, 16, (char *)"Font OK", 0);
}

static u16 pic_get_tnum(u8 *path)
{
    u8 res;
    u16 rval = 0;
    DIR tdir;
    FILINFO tfileinfo;
    u8 *fn;
    res = f_opendir(&tdir, (const TCHAR *)path);
    tfileinfo.lfsize = _MAX_LFN * 2 + 1;
    tfileinfo.lfname = mymalloc(SRAMIN, tfileinfo.lfsize);
    if (res == FR_OK && tfileinfo.lfname != NULL)
    {
        while (1)
        {
            res = f_readdir(&tdir, &tfileinfo);
            if (res != FR_OK || tfileinfo.fname[0] == 0)
                break;
            fn = (u8 *)(*tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == 0X50)
            {
                rval++;
            }
        }
    }
    return rval;
}

static void Show_Picture(void)
{
    u8 res;
    DIR picdir;
    FILINFO picfileinfo;
    u8 *fn;
    u8 *pname;
    u16 totpicnum;
    u16 curindex;
    u8 key;
    u8 pause = 0;
    u8 t;
    u16 temp;
    u16 *picindextbl;
    POINT_COLOR = RED;
    while (f_opendir(&picdir, "0:/PICTURE"))
    {
        Show_Str(lcddev.width / 2 - 72, 145, lcddev.width, 16, (char *)"PICTURE!", 16, 1);
        delay_ms(200);
        LCD_Fill(lcddev.width / 2 - 72, 145, lcddev.width / 2 + 72, 161, WHITE);
        delay_ms(200);
    }
    totpicnum = pic_get_tnum((u8 *)("0:/PICTURE"));
    while (totpicnum == NULL)
    {
        Show_Str(lcddev.width / 2 - 52, 145, lcddev.width, 16, (char *)"!", 16, 1);
        delay_ms(200);
        LCD_Fill(lcddev.width / 2 - 52, 145, lcddev.width / 2 + 52, 161, WHITE);
        delay_ms(200);
    }
    picfileinfo.lfsize = _MAX_LFN * 2 + 1;
    picfileinfo.lfname = mymalloc(SRAMIN, picfileinfo.lfsize);
    pname = mymalloc(SRAMIN, picfileinfo.lfsize);
    picindextbl = mymalloc(SRAMIN, 2 * totpicnum);
    while (picfileinfo.lfname == NULL || pname == NULL || picindextbl == NULL)
    {
        Show_Str(lcddev.width / 2 - 52, 145, lcddev.width, 16, (char *)"!", 16, 1);
        delay_ms(200);
        LCD_Fill(lcddev.width / 2 - 52, 145, lcddev.width / 2 + 52, 161, WHITE);
        delay_ms(200);
    }

    res = f_opendir(&picdir, "0:/PICTURE");
    if (res == FR_OK)
    {
        curindex = 0;
        while (1)
        {
            temp = picdir.index;
            res = f_readdir(&picdir, &picfileinfo);
            if (res != FR_OK || picfileinfo.fname[0] == 0)
                break;
            fn = (u8 *)(*picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == 0X50)
            {
                picindextbl[curindex] = temp;
                curindex++;
            }
        }
    }
    POINT_COLOR = BLUE;
    Show_Str(lcddev.width / 2 - 44, 145, lcddev.width, 16, (char *)"...", 16, 1);
    delay_ms(1500);
    delay_ms(500);
    piclib_init();
    curindex = 0;
    res = f_opendir(&picdir, (const TCHAR *)"0:/PICTURE");
    while (res == FR_OK)
    {
        dir_sdi(&picdir, picindextbl[curindex]);
        res = f_readdir(&picdir, &picfileinfo);
        if (res != FR_OK || picfileinfo.fname[0] == 0)
            break;
        fn = (u8 *)(*picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
        strcpy((char *)pname, "0:/PICTURE/");
        strcat((char *)pname, (const char *)fn);
        LCD_Clear(BLACK);
        ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1);
        Show_Str(2, 2, 240, 16, (char *)pname, 16, 1);
        t = 0;
        while (1)
        {
            key = KEY_Scan(0);
            if (t > 250)
                key = 1;
            if ((t % 20) == 0)
                LED0 = !LED0;
            if (key == KEY_UP_PRES)
            {
                if (curindex)
                    curindex--;
                else
                    curindex = totpicnum - 1;
                break;
            }
            else if (key == KEY0_PRES)
            {
                curindex++;
                if (curindex >= totpicnum)
                    curindex = 0;
                break;
            }
            if (pause == 0)
                t++;
            delay_ms(10);
        }
        res = 0;
    }
    myfree(SRAMIN, picfileinfo.lfname);
    myfree(SRAMIN, pname);
    myfree(SRAMIN, picindextbl);
}

static void TCS34725_Test(void)
{
    COLOR_RGBC rgb;
    COLOR_HSL hsl;
    TCS34725_GetRawData(&rgb);
    RGBtoHSL(&rgb, &hsl);
    PRINT("R=%d G=%d B=%d C=%d\r\n", rgb.r, rgb.g, rgb.b, rgb.c);
    PRINT("H=%d S=%d L=%d\r\n", hsl.h, hsl.s, hsl.l);
}

static void AHT10_Test(void)
{
    int32_t temp;
    int32_t humi;
    AHT_Measure();
    ATH_Read(&temp, &humi);
    PRINT("Temperature =%d Humidity=%d\r\n", temp, humi);
}

static void AS5600_Test(void)
{
    PRINT("magnet=%d\r\n", detect_magnet());
    PRINT("angle=%d\r\n", get_angle());
    PRINT("agc=%d\r\n", get_agc());
}

static void BH1750_Test(void)
{
    PRINT("light=%d\r\n", BH1750_Read_Measure());
}

void InitHardware(void)
{

    LED_Init();
    // LCD_Init();
    // TP_Init();
    KEY_Init();
    AT24C02_Init();
    W25Q64_Init();
    // AS_Init();
    // AHT_Init();
    // TCS34725_Init();
    // BH1750_Init();
    Check_SDCard();
    USB_VCP_Init();
    // AT45DBXX_Init();
    // my_mem_init(SRAMIN);
    // my_mem_init(SRAMCCM);
    // my_mem_init(SRAMBANK);
    // exfuns_init();
}

void test()
{
    // Ctp_Test();
    // W25Q64_Test();
    // AT24C02_Test();
    // WS_AT45DBXX_Test();
    // TCS34725_Test();
    // AHT10_Test();
    // AS5600_Test();
    // BH1750_Test();
}
