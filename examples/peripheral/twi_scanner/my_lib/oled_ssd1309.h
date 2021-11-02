#ifndef __olde_ssd1309_h__
#define __olde_ssd1309_h__
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdbool.h>

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
//#include "../lvgl_helpers.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
typedef enum {
    SSD1306_OK = 0x00,
    SSD1306_ERR = 0x01  // Generic error.
} SSD1306_Error_t;

#ifndef SSD1306_BUFFER_SIZE
#define SSD1306_BUFFER_SIZE   128 * 32 / 8
#endif
// Enumeration for screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} SSD1306_COLOR;
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#define SSD1306_DISPLAY_ORIENTATION     TFT_ORIENTATION_LANDSCAPE

void ssd1306_init(void);
void ssd1306_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map);
void ssd1306_rounder(lv_disp_drv_t * disp_drv, lv_area_t *area);
void ssd1306_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
    lv_color_t color, lv_opa_t opa);

void ssd1306_sleep_in(void);
void ssd1306_sleep_out(void);
#endif //__olde_ssd1309_h__
