#include "lvgl/lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif
#ifndef LV_ATTRIBUTE_IMG_BATTERY_24PX
#define LV_ATTRIBUTE_IMG_BATTERY_24PX
#endif
const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_IMG_BATTERY_24PX uint8_t battery_24px_map[] = {
  0x00, 0x00, 0x00, 0x02, 	/*Color of index 0*/
  0x00, 0x00, 0x00, 0xc6, 	/*Color of index 1*/

  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0xff, 0xff, 0xf8, 
  0xff, 0xff, 0xf8, 
  0xc0, 0x00, 0x18, 
  0xcc, 0xd8, 0x18, 
  0xcd, 0xdc, 0x1f, 
  0xdd, 0xdc, 0x1f, 
  0xcd, 0xdc, 0x1b, 
  0xdd, 0xdc, 0x1b, 
  0xcd, 0xdc, 0x1f, 
  0xdd, 0xdc, 0x1f, 
  0xcc, 0xd8, 0x18, 
  0xc0, 0x00, 0x18, 
  0xff, 0xff, 0xf8, 
  0xff, 0xff, 0xf8, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 
};

const lv_img_dsc_t battery_24px = {
  .header.always_zero = 0,
  .header.w = 24,
  .header.h = 24,
  .data_size = 81,
  .header.cf = LV_IMG_CF_INDEXED_1BIT,
  .data = battery_24px_map,
};
