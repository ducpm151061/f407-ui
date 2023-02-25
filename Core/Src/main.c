#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "stdio.h"
#include "ui_test.h"

static __IO u32 system_ticks;
void SysTick_Handler(void);

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

int main(void)
{
    lv_init();
    lv_log_register_print_cb(lvgl_log_cb);
    lv_group_t *keyboard_group = lv_group_create();
    SystemCoreClockUpdate();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    InitOS();
    InitSoftware();
    InitHardware();
    test();
    lv_port_disp_init();
    lv_port_indev_init();
    ui();
    while (1)
    {
        lv_timer_handler();
        delay_ms(1);
    }
}

void SysTick_Handler(void)
{
    system_ticks++;
    lv_tick_inc(1);
}
