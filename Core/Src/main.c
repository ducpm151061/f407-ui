#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "stdio.h"
#include "ui_test.h"

static __IO uint32_t system_ticks;

void SysTick_Handler(void);

static void InitOS();
static void InitOS()
{
    if (SysTick_Config(SystemCoreClock / 1000))
        while (1)
            ;
    NVIC_SetPriority(SysTick_IRQn, 0x0);
}

/* Configure the SysTick handler priority */
inline static void on_tick(uint32_t period)
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
    delay_init(168);
    Debug_Init(921600);
    LED_Init();
    // LCD_Init();
    // TP_Init();
    KEY_Init();
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
    AT24C02_Init();
    W25Q64_Init();
    // AS_Init();
    // AHT_Init();
    TCS34725_Init();
    RNG_Init();
    Check_SDCard();
    // my_mem_init(SRAMIN);
    // my_mem_init(SRAMCCM);
    // my_mem_init(SRAMBANK);
    // exfuns_init();
    // AT45DBXX_Init();

    // W25Q64_Test();
    // AT24C02_Test();
    // WS_AT45DBXX_Test();

    lv_port_disp_init();
    lv_port_indev_init(keyboard_group);
    // lv_test();
    // lv_demo_music();
    lv_demo_stress();
    // lv_demo_widgets();
    // lv_demo_benchmark();
    // lv_demo_keypad_encoder();
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
