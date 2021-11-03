
/**
 * @file lv_port_indev_templ.h
 *
 */

/*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lvgl/lvgl.h"

    /*********************
     *      DEFINES
     *********************/

    /**********************
     *      TYPEDEFS
     **********************/

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    /**********************
     *      MACROS
     **********************/

    void lv_port_indev_init(lv_group_t *keyboard_group);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_H*/

#endif /*Disable/Enable content*/
