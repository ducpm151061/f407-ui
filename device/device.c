#include "../platform.h"
#include "stdio.h"
#include "test.h"

static __IO u32 system_ticks;
void SysTick_Handler(void);

static void lvgl_log_cb(const char *buf)
{
    PRINT("%s\r", buf);
    USB_PRINT("%s\r", buf);
}

static void InitOS();
static void InitSoftware();
static void InitOS()
{
    if (SysTick_Config(SystemCoreClock / 1000))
        while (1)
            ;
    NVIC_SetPriority(SysTick_IRQn, 0x0);
}

static void InitSoftware()
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
    delay_init(168);
    Debug_Init(921600);
    RNG_Init();
}

/* Configure the SysTick handler priority */
inline static void on_tick(u32 period)
{
    lv_tick_inc(period);
}

void system_init(lv_group_t *keyboard_group)
{
#if DEBUG
    lv_log_register_print_cb(lvgl_log_cb);
#endif
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    InitOS();
    InitSoftware();
    InitHardware();
    lv_port_disp_init();
    lv_port_indev_init(keyboard_group);

    test();
}
void SysTick_Handler(void)
{
    system_ticks++;
    lv_tick_inc(1);
}