

/***********************************************************************************************************************
* History      : DD.MM.YYYY Version    Description
*              : 22.03.2020   1.00     First Release
*              : DD.MM.YYYY   x.00     First Release
*              : DD.MM.YYYY   x.00     First Release
***********************************************************************************************************************/

/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "common.h"
#include "user_lvgl_config.h"
#include "timer_service.h"
#include "oled_ssd1309.h"
#include "lvgl.h"
#include "app_timer.h"

#include "my_logo.h"
#include "battery_32px.h"
#include "battery_charger_64px.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define LVGL_TIMER_MEAS_INTERVAL APP_TIMER_TICKS(200)          // 100ms
#define LVGL_TICK_TASK_TIMER_MEAS_INTERVAL APP_TIMER_TICKS(20) // 1 ms
APP_TIMER_DEF(m_lvgl_timer_id);
APP_TIMER_DEF(m_lvgl_tick_task_id);
APP_TIMER_DEF(m_lvgl_count_test_id);

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
typedef struct
{
    lv_obj_t *spo2_value;
    lv_obj_t *bmp;
} sensor_t;

sensor_t sensor;
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void disp_driver_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lvgl_timer_init(void);
static void lvgl_timer_timeout_handler(void *p_context);
static void lvgl_tick_task_timeout_handler(void *p_context);
static void lvgl_timer_count_test_handler(void *p_context);

static void disp_driver_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                               lv_color_t color, lv_opa_t opa);
static void disp_driver_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area);

static void example_text(void);
static void example_number(void);
static void user_lvgl_display_battery_charge(void);
static void user_lvgl_display_logo(void);

static lv_disp_buf_t disp_buf;
static lv_color_t buf1[(LV_HOR_RES_MAX * LV_VER_RES_MAX)];

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
extern volatile bool update_value_status;
LV_IMG_DECLARE(my_logo);
LV_IMG_DECLARE(level3_battery);
LV_IMG_DECLARE(battery_32px);
LV_IMG_DECLARE(battery_charger_64px);
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
lv_obj_t *main_screen;
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void user_lvgl_init(void)
{
    uint32_t size_in_px = (LV_HOR_RES_MAX * LV_VER_RES_MAX);
    size_in_px *= 8;
    lv_init();

    ssd1306_init();
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);
    // lv_disp_buf_init(&disp_buf, buf1, buf2, (LV_HOR_RES_MAX * (LV_VER_RES_MAX / 8)) * 8);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    /* When using a monochrome display we need to register the callbacks:
* - rounder_cb
* - set_px_cb */
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    // lv_indev_drv_t indev_drv;
    // lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_ENCODER;
    // indev_drv.read_cb = read_encoder;
    // lv_indev_drv_register(&indev_drv);

    example_text();
    //user_lvgl_display_logo();
    //nrf_delay_ms(2000);
    //user_lvgl_display_logo();
    //lv_ex_line_1();
    lvgl_timer_init();
}
/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/
static void disp_driver_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    ssd1306_flush(drv, area, color_map);
}
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void disp_driver_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    ssd1306_rounder(disp_drv, area);
}
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void disp_driver_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
                               lv_color_t color, lv_opa_t opa)
{
    ssd1306_set_px_cb(disp_drv, buf, buf_w, x, y, color, opa);
}
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void lvgl_timer_init(void)
{
    ret_code_t err_code;
    // lvgl tick task timer
    err_code = app_timer_create(&m_lvgl_tick_task_id,
                                APP_TIMER_MODE_REPEATED,
                                lvgl_tick_task_timeout_handler);
    // Start lvgl tick task timer
    err_code = app_timer_start(m_lvgl_tick_task_id, LVGL_TICK_TASK_TIMER_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    //
    err_code = app_timer_create(&m_lvgl_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                lvgl_timer_timeout_handler);
    // Start lvgl timer
    err_code = app_timer_start(m_lvgl_timer_id, LVGL_TIMER_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    // for test
    //
    err_code = app_timer_create(&m_lvgl_count_test_id,
                                APP_TIMER_MODE_REPEATED,
                                lvgl_timer_count_test_handler);
    // Start lvgl timer
    //err_code = app_timer_start(m_lvgl_count_test_id, 2000, NULL);
    APP_ERROR_CHECK(err_code);
}
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void lvgl_tick_task_timeout_handler(void *p_context)
{
    lv_tick_inc(1);
    //     NRF_LOG_INFO("lvgl_tick_task_timeout_handler call. %d", count_test++);
}
/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void lvgl_timer_timeout_handler(void *p_context)
{
    // NRF_LOG_DEBUG("lvgl_timer_timeout_handler call");
    lv_task_handler(); /* let the GUI do its work */
}


//uint32_t count_test = 0;

//void user_lvgl_update_number(void)
//{
//    char buffer[10];
//    sprintf(buffer, "%d", count_test++);
//    lv_label_set_text(sensor.spo2_value, buffer);
//    lv_obj_align(sensor.spo2_value, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
//}

static void user_lvgl_display_logo(void)
{
    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    /*Create a Label on the currently active screen*/
    lv_obj_t *img = lv_img_create(scr, NULL);
    lv_img_set_src(img, &my_logo);
    /*Modify the Label's text*/
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
}

static void user_lvgl_display_battery_charge(void)
{
    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    /*Create a Label on the currently active screen*/
    lv_obj_t *img = lv_img_create(scr, NULL);
    lv_img_set_src(img, &battery_charger_64px);
    /*Modify the Label's text*/
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/

static void example_text(void)
{
    /* use a pretty small demo for monochrome displays */
    /* Get the current screen  */
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);

    /*Create a Label on the currently active screen*/
    lv_obj_t *label1 = lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "SpO2 %");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    /*Create a Label on the currently active screen*/
    lv_obj_t *label2 = lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label2, "PR bmp");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 20);

    /*Create a Label on the currently active screen*/
    sensor.spo2_value = lv_label_create(scr, NULL);

    /*Modify the Label's text*/

    lv_label_set_text(sensor.spo2_value, "97");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(sensor.spo2_value, NULL, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES_MAX / 2 + 10, 0);

    /*Create a Label on the currently active screen*/
    sensor.bmp = lv_label_create(scr, NULL);

    /*Modify the Label's text*/

    lv_label_set_text(sensor.bmp, "110");
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(sensor.bmp, NULL, LV_ALIGN_IN_TOP_LEFT, LV_HOR_RES_MAX / 2 + 10, 20);

    /* Display image batterry*/
    /*Create a Label on the currently active screen*/
    lv_obj_t *img = lv_img_create(scr, NULL);
    lv_img_set_src(img, &battery_32px);
    /*Modify the Label's text*/
    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(img, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, -5);
}

//static void example_number(void)
//{
//    char buffer[10];
//    sprintf(buffer, "%d", count_test++);
//    lv_label_set_text(sensor.spo2_value, buffer);
//    lv_obj_align(sensor.spo2_value, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);
//}
#define ecg_step 3
static lv_point_t line_points[LV_HOR_RES_MAX / ecg_step];
static uint16_t ecg_number = 0;
static bool round_two = 0;
/*Create a line and apply the new style*/
lv_obj_t *line1;
void lv_ex_line_1(int16_t ecg_value)
{
    uint16_t max_y = LV_VER_RES_MAX - 40;
    uint16_t max_x = LV_HOR_RES_MAX;
    if( ecg_value > max_y) ecg_value = max_y;
    //printf("ecg_number * ecg_step = %d\r\n", ecg_number * ecg_step);
    line_points[ecg_number].x = (lv_coord_t)ecg_number * ecg_step;
    line_points[ecg_number].y = (lv_coord_t)ecg_value;
    //printf("line_points[%d] = (%d,%d)\r\n", ecg_number, ecg_number * ecg_step, ecg_value);
    ecg_number++;
    /*Create an array for the points of the line*/
    if (ecg_number > ((LV_HOR_RES_MAX / ecg_step) - 1))
    {
        //printf("ecg_number = %d\r\n", ecg_number);
        ecg_number = 0;
        round_two = 1;
    }
    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 1);
    lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_line_rounded(&style_line, LV_STATE_DEFAULT, false);

    line1 = lv_line_create(lv_scr_act(), NULL);

    if (round_two == 1)
        lv_line_set_points(line1, line_points, LV_HOR_RES_MAX / ecg_step); /*Set the points*/
    else lv_line_set_points(line1, line_points, ecg_number); /*Set the points*/

    lv_obj_add_style(line1, LV_LINE_PART_MAIN, &style_line); /*Set the points*/
    lv_obj_align(line1, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
}

/***********************************************************************************************************************
* Function Name: user_lvgl_init
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void lvgl_timer_count_test_handler(void *p_context)
{
    //update_value_status = true;
}
