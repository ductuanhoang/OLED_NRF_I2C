#ifndef __user_lvgl_confog_h__
#define __user_lvgl_confog_h__
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

void user_lvgl_init(void);

void user_lvgl_tick(void);


void user_lvgl_update_number(void);

void user_lvgl_timer();

void user_lvgl_display_imange(void);

void lv_ex_line_1(int16_t ecg_value);
/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
#endif //__user_lvgl_confog_h__
