#include "ui_test.h"
#include "../lvgl/lvgl.h"
#include "../lvgl/demos/lv_demos.h"

static lv_obj_t *screen;
static lv_obj_t *chart;
static lv_obj_t *screen2;

static lv_obj_t *btn1;
static lv_obj_t *btn2;
static lv_obj_t *btn3;
static lv_obj_t *label_add;
static void lv_test(void);

static void screen_next(void);
static void lv_screen_2(void);
static void hook_division_lines(lv_event_t *e);
static void add_faded_area(lv_event_t *e);
static void draw_event_cb(lv_event_t *e);
static void slider_event_cb(lv_event_t *e);

static void event_handler_2(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        if (target == btn1)
        {
            lv_test();
        }
        else if (target == btn2)
        {
            lv_screen_2();
        }
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("Toggled");
    }
}

static void event_handler_3(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        screen_next();
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("Toggled");
    }
}
static void draw_event_cb(lv_event_t *e)
{
    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = draw_task->draw_dsc;

    if (base_dsc->part == LV_PART_ITEMS && draw_task->type == LV_DRAW_TASK_TYPE_LINE)
    {
        add_faded_area(e);
    }
    /*Hook the division lines too*/
    if (base_dsc->part == LV_PART_MAIN && draw_task->type == LV_DRAW_TASK_TYPE_LINE)
    {
        hook_division_lines(e);
    }
}

static void add_faded_area(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);

    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = draw_task->draw_dsc;

    const lv_chart_series_t *ser = lv_chart_get_series_next(obj, NULL);

    /*Draw a triangle below the line witch some opacity gradient*/
    lv_draw_line_dsc_t *draw_line_dsc = draw_task->draw_dsc;
    lv_draw_triangle_dsc_t tri_dsc;

    lv_draw_triangle_dsc_init(&tri_dsc);
    tri_dsc.p[0].x = draw_line_dsc->p1.x;
    tri_dsc.p[0].y = draw_line_dsc->p1.y;
    tri_dsc.p[1].x = draw_line_dsc->p2.x;
    tri_dsc.p[1].y = draw_line_dsc->p2.y;
    tri_dsc.p[2].x = draw_line_dsc->p1.y < draw_line_dsc->p2.y ? draw_line_dsc->p1.x : draw_line_dsc->p2.x;
    tri_dsc.p[2].y = LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y);
    tri_dsc.bg_grad.dir = LV_GRAD_DIR_VER;

    int32_t full_h = lv_obj_get_height(obj);
    int32_t fract_uppter = (int32_t)(LV_MIN(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - obj->coords.y1) * 255 / full_h;
    int32_t fract_lower = (int32_t)(LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - obj->coords.y1) * 255 / full_h;
    tri_dsc.bg_grad.stops[0].color = ser->color;
    tri_dsc.bg_grad.stops[0].opa = 255 - fract_uppter;
    tri_dsc.bg_grad.stops[0].frac = 0;
    tri_dsc.bg_grad.stops[1].color = ser->color;
    tri_dsc.bg_grad.stops[1].opa = 255 - fract_lower;
    tri_dsc.bg_grad.stops[1].frac = 255;

    lv_draw_triangle(base_dsc->layer, &tri_dsc);

    /*Draw rectangle below the triangle*/
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_grad.dir = LV_GRAD_DIR_VER;
    rect_dsc.bg_grad.stops[0].color = ser->color;
    rect_dsc.bg_grad.stops[0].frac = 0;
    rect_dsc.bg_grad.stops[0].opa = 255 - fract_lower;
    rect_dsc.bg_grad.stops[1].color = ser->color;
    rect_dsc.bg_grad.stops[1].frac = 255;
    rect_dsc.bg_grad.stops[1].opa = 0;

    lv_area_t rect_area;
    rect_area.x1 = (int32_t)draw_line_dsc->p1.x;
    rect_area.x2 = (int32_t)draw_line_dsc->p2.x - 1;
    rect_area.y1 = (int32_t)LV_MAX(draw_line_dsc->p1.y, draw_line_dsc->p2.y) - 1;
    rect_area.y2 = (int32_t)obj->coords.y2;
    lv_draw_rect(base_dsc->layer, &rect_dsc, &rect_area);
}

static void hook_division_lines(lv_event_t *e)
{
    lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t *base_dsc = draw_task->draw_dsc;
    lv_draw_line_dsc_t *line_dsc = draw_task->draw_dsc;

    /*Vertical line*/
    if (line_dsc->p1.x == line_dsc->p2.x)
    {
        line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
        if (base_dsc->id1 == 3)
        {
            line_dsc->width = 2;
            line_dsc->dash_gap = 0;
            line_dsc->dash_width = 0;
        }
        else
        {
            line_dsc->width = 1;
            line_dsc->dash_gap = 6;
            line_dsc->dash_width = 6;
        }
    }
    /*Horizontal line*/
    else
    {
        if (base_dsc->id1 == 2)
        {
            line_dsc->width = 2;
            line_dsc->dash_gap = 0;
            line_dsc->dash_width = 0;
        }
        else
        {
            line_dsc->width = 2;
            line_dsc->dash_gap = 6;
            line_dsc->dash_width = 6;
        }

        if (base_dsc->id1 == 1 || base_dsc->id1 == 3)
        {
            line_dsc->color = lv_palette_main(LV_PALETTE_GREEN);
        }
        else
        {
            line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
        }
    }
}

static void screen_next(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);

    // /* gif */
    // LV_IMG_DECLARE(leg);
    // lv_obj_t *gif = lv_gif_create(scr);
    // lv_gif_set_src(gif, &leg);
    // lv_obj_center(gif);

    /*Create a chart*/
    lv_obj_t *chart = lv_chart_create(scr);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
    lv_obj_set_size(chart, 200, 150);
    lv_obj_set_style_pad_all(chart, 0, 0);
    lv_obj_set_style_radius(chart, 0, 0);
    lv_obj_center(chart);

    lv_chart_set_div_line_count(chart, 5, 7);

    lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(chart, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    lv_chart_series_t *ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    uint32_t i;
    for (i = 0; i < 10; i++)
    {
        lv_chart_set_next_value(chart, ser, lv_rand(10, 80));
    }

    btn1 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn1, event_handler_2, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_set_height(btn1, 50);
    lv_obj_set_width(btn1, 50);

    lv_obj_t *label = lv_label_create(btn1);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);

    btn2 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn2, event_handler_2, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_OUT_LEFT_BOTTOM, 270, 0);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, 50);
    lv_obj_set_width(btn2, 50);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Next");
    lv_obj_center(label);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, true);
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        LV_LOG_USER("Clicked");
        screen_next();
    }
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("Toggled");
    }
}

static void add_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_label_set_text_fmt(label_add, "%d", atoi(lv_label_get_text(label_add)) + 1);
    }
}

static void lv_test(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    /* wallpaper */
    LV_IMG_DECLARE(simpson);
    lv_obj_t *icon = lv_img_create(scr);
    lv_img_set_src(icon, &simpson);
    lv_obj_center(icon);

    /* button */
    lv_obj_t *label1;

    btn2 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_OUT_LEFT_BOTTOM, 270, 0);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, 50);
    lv_obj_set_width(btn2, 50);

    label1 = lv_label_create(btn2);
    lv_label_set_text(label1, "Next");
    lv_obj_center(label1);

    btn3 = lv_btn_create(scr);
    lv_obj_add_event_cb(btn3, add_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn3, LV_ALIGN_OUT_BOTTOM_MID, 50, 50);
    lv_obj_add_flag(btn3, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn3, 50);
    lv_obj_set_width(btn3, 50);

    label_add = lv_label_create(btn3);
    lv_label_set_text(label_add, "1");
    lv_obj_center(label_add);

    screen = scr;
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_ON, 1300, 0, true);
}

static void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    /*Provide some extra space for the value*/
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        lv_event_set_ext_draw_size(e, 50);
    }
    else if (code == LV_EVENT_DRAW_MAIN_END)
    {
        if (!lv_obj_has_state(obj, LV_STATE_PRESSED))
            return;

        lv_slider_t *slider = (lv_slider_t *)obj;
        const lv_area_t *indic_area = &slider->bar.indic_area;
        char buf[16];
        lv_snprintf(buf, sizeof(buf), "%d - %d", (int)lv_slider_get_left_value(obj), (int)lv_slider_get_value(obj));

        lv_point_t label_size;
        lv_text_get_size(&label_size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, 0);
        lv_area_t label_area;
        label_area.x1 = 0;
        label_area.x2 = label_size.x - 1;
        label_area.y1 = 0;
        label_area.y2 = label_size.y - 1;

        lv_area_align(indic_area, &label_area, LV_ALIGN_OUT_TOP_MID, 0, -10);

        lv_draw_label_dsc_t label_draw_dsc;
        lv_draw_label_dsc_init(&label_draw_dsc);
        label_draw_dsc.color = lv_color_hex3(0x888);
        label_draw_dsc.text = buf;
        label_draw_dsc.text_local = true;
        lv_layer_t *layer = lv_event_get_layer(e);
        lv_draw_label(layer, &label_draw_dsc, &label_area);
    }
}

static void lv_screen_2(void)
{
    /*Create a chart*/
    screen2 = lv_obj_create(NULL);

    /* button */
    lv_obj_t *label;

    lv_obj_t *btn1 = lv_btn_create(screen2);
    lv_obj_add_event_cb(btn1, event_handler_3, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_OUT_LEFT_BOTTOM, 0, 0);
    lv_obj_set_height(btn1, 50);
    lv_obj_set_width(btn1, 50);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);

    lv_obj_t *slider;
    slider = lv_slider_create(screen2);
    lv_obj_center(slider);

    lv_slider_set_mode(slider, LV_SLIDER_MODE_RANGE);
    lv_slider_set_value(slider, 70, LV_ANIM_OFF);
    lv_slider_set_left_value(slider, 20, LV_ANIM_OFF);

    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(slider);

    lv_scr_load_anim(screen2, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 300, 0, true);
}

void ui()
{
    // lv_test();
    // lv_demo_music();
    lv_demo_stress();
    // lv_demo_widgets();
    // lv_demo_benchmark();
    // lv_demo_keypad_encoder();
}
