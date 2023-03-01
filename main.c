#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "platform.h"
#include "ui_test.h"

int main(void)
{
    lv_init();
    lv_group_t *keyboard_group = lv_group_create();
    system_init(keyboard_group);
    ui();
    while (1)
    {
        lv_timer_handler();
        delay_ms(1);
    }
}
