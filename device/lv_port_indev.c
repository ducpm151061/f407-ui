/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

#include "lv_port_indev.h"
#include "gt911.h"
#include "key.h"
#include "led.h"
#include "lvgl.h"
#include "touch.h"
#include "usart.h"

lv_indev_t *indev_touchpad;
lv_indev_t *indev_keypad;
extern u8 touch_flag;

static void touchpad_init(void)
{
    GT911_Init();
}

/* static bool touchpad_is_pressed(void)
{
    return touch_flag > 0 ? true : false;
} */

static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    static int16_t x = -1, y = -1;
    GT911_Position(&x, &y);
    /*Save the pressed coordinates and the state*/
    // if (touchpad_is_pressed())
    if (x == -1 && y == -1)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        last_x = x;
        last_y = y;
        data->state = LV_INDEV_STATE_PR;
    }
    data->point.x = last_x;
    data->point.y = last_y;

    /*Set the last pressed coordinates*/
}

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    u8 key_value = KEY_Scan(0);
    if (key_value == KEY_UP_PRES)
    {
        data->key = LV_KEY_ESC;
        data->state = LV_INDEV_STATE_PR;
        LED_Toggle();
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void lv_port_indev_init(lv_group_t *keyboard_group)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */
    /* Touchpad */
    static lv_indev_drv_t indev_drv;

    touchpad_init();
    /*Register a touchpad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    /*Register a button input device*/
    static lv_indev_drv_t keypad_indev_drv;
    lv_indev_drv_init(&keypad_indev_drv);
    keypad_indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    keypad_indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&keypad_indev_drv);

    lv_indev_set_group(indev_keypad, keyboard_group);
}

#else /*Enable this file at the top*/
/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
