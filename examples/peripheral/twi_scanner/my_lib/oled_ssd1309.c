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
#include "oled_ssd1309.h"
// #include "spi_driver.h"
#include "i2c_driver.h"

#include <math.h>
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define TAG "SSD1306"

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS                    0x3C
#define OLED_WIDTH                          128
#define OLED_HEIGHT                         64
#define OLED_COLUMNS                        128
#define OLED_PAGES                          8
#define OLED_PIXEL_PER_PAGE                 8

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE        0x80
#define OLED_CONTROL_BYTE_CMD_STREAM        0x00
#define OLED_CONTROL_BYTE_DATA_STREAM       0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST               0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM                0xA4
#define OLED_CMD_DISPLAY_ALLON              0xA5
#define OLED_CMD_DISPLAY_NORMAL             0xA6
#define OLED_CMD_DISPLAY_INVERTED           0xA7
#define OLED_CMD_DISPLAY_OFF                0xAE
#define OLED_CMD_DISPLAY_ON                 0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE       0x20    // follow with 0x00 = HORZ mode
#define OLED_CMD_SET_COLUMN_RANGE           0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE             0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE     0x40
#define OLED_CMD_SET_SEGMENT_REMAP          0xA1
#define OLED_CMD_SET_MUX_RATIO              0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE_NORMAL   0xC0
#define OLED_CMD_SET_COM_SCAN_MODE_REMAP    0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET         0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP            0xDA    // follow with 0x12
#define OLED_CMD_NOP                        0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV        0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE              0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT          0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP            0x8D    // follow with 0x14

#define OLED_IIC_FREQ_HZ                    400000  // I2C colock frequency

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static uint8_t send_data(lv_disp_drv_t *disp_drv, void *bytes, size_t bytes_len);
static uint8_t send_pixels(lv_disp_drv_t *disp_drv, void *color_buffer, size_t buffer_len);
// add test
// static void ssd1306_WriteCommand(uint8_t byte);
// static void ssd1306_SetDisplayOn(const uint8_t on);
static void ssd1306_Reset(void);
// static void ssd1306_SetContrast(const uint8_t value) ;
// static void ssd1306_Fill(SSD1306_COLOR color);
// static void ssd1306_UpdateScreen(void);
// static void ssd1306_WriteData(uint8_t* buffer, size_t buff_size);

// static uint8_t SSD1306_Buffer[OLED_WIDTH * OLED_HEIGHT / 8];
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

#define BIT_SET(a,b) ((a) |= (1U<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1U<<(b)))

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
#define CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void ssd1306_init(void)
{
	NRF_LOG_INFO("ssd1306_Init call");
	// user_spi_driver_init();
    bool status = false;
    status = user_i2c_init();
    if (!status) {
        NRF_LOG_INFO("i2c init failed");
        return;
    }
    else NRF_LOG_INFO("i2c init succeeded");

	// Wait for the screen to boot
	nrf_delay_ms(100);
    ssd1306_Reset();
    nrf_delay_ms(100);
    uint8_t orientation_1 = 0;
    uint8_t orientation_2 = 0;

#if defined (CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE)
    orientation_1 = OLED_CMD_SET_SEGMENT_REMAP;
    orientation_2 = OLED_CMD_SET_COM_SCAN_MODE_REMAP;
#elif defined (CONFIG_LV_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED)
    orientation_1 = 0xA0;
    orientation_2 = OLED_CMD_SET_COM_SCAN_MODE_NORMAL;
#else
    #error "Unsupported orientation"
#endif

    uint8_t display_mode = 0;

#if defined CONFIG_LV_INVERT_COLORS
    display_mode = OLED_CMD_DISPLAY_INVERTED;
#else
    display_mode = OLED_CMD_DISPLAY_NORMAL;
#endif

     uint8_t conf[] = {
         OLED_CONTROL_BYTE_CMD_STREAM,
         OLED_CMD_SET_CHARGE_PUMP,
         0x14,
         orientation_1,
         orientation_2,
         OLED_CMD_SET_CONTRAST,
         display_mode,
         0xFF,
         OLED_CMD_DISPLAY_ON
     };

    uint8_t err = send_data(NULL, conf, sizeof(conf));
//    assert(0 == err);

//         NRF_LOG_INFO("ssd1306_Init call");
//     user_spi_driver_init();
//     // Wait for the screen to boot
//     nrf_delay_ms(100);
//     // Reset OLED
//     ssd1306_Reset();
//     nrf_delay_ms(100);
//     // Init OLED
//     ssd1306_SetDisplayOn(0); //display off

//     ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode
//     ssd1306_WriteCommand(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
//                                 // 10b,Page Addressing Mode (RESET); 11b,Invalid

//     ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

// #ifdef SSD1306_MIRROR_VERT
//     ssd1306_WriteCommand(0xC0); // Mirror vertically
// #else
//     ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
// #endif

//     ssd1306_WriteCommand(0x00); //---set low column address
//     ssd1306_WriteCommand(0x10); //---set high column address

//     ssd1306_WriteCommand(0x40); //--set start line address - CHECK

//     ssd1306_SetContrast(0xFF);

// #ifdef SSD1306_MIRROR_HORIZ
//     ssd1306_WriteCommand(0xA0); // Mirror horizontally
// #else
//     ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127 - CHECK
// #endif

// #ifdef SSD1306_INVERSE_COLOR
//     ssd1306_WriteCommand(0xA7); //--set inverse color
// #else
//     ssd1306_WriteCommand(0xA6); //--set normal color
// #endif

// // Set multiplex ratio.
// #if (OLED_HEIGHT == 128)
//     // Found in the Luma Python lib for SH1106.
//     ssd1306_WriteCommand(0xFF);
// #else
//     ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64) - CHECK
// #endif

// #if (OLED_HEIGHT == 32)
//     ssd1306_WriteCommand(0x1F); //
// #elif (OLED_HEIGHT == 64)
//     ssd1306_WriteCommand(0x3F); //
// #elif (OLED_HEIGHT == 128)
//     ssd1306_WriteCommand(0x3F); // Seems to work for 128px high displays too.
// #else
// #error "Only 32, 64, or 128 lines of height are supported!"
// #endif

//     ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

//     ssd1306_WriteCommand(0xD3); //-set display offset - CHECK
//     ssd1306_WriteCommand(0x00); //-not offset

//     ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
//     ssd1306_WriteCommand(0xF0); //--set divide ratio

//     ssd1306_WriteCommand(0xD9); //--set pre-charge period
//     ssd1306_WriteCommand(0x22); //

//     ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration - CHECK
// #if (OLED_HEIGHT == 32)
//     ssd1306_WriteCommand(0x02);
// #elif (OLED_HEIGHT == 64)
//     ssd1306_WriteCommand(0x12);
// #elif (OLED_HEIGHT == 128)
//     ssd1306_WriteCommand(0x12);
// #else
// #error "Only 32, 64, or 128 lines of height are supported!"
// #endif

//     ssd1306_WriteCommand(0xDB); //--set vcomh
//     ssd1306_WriteCommand(0x20); //0x20,0.77xVcc

//     ssd1306_WriteCommand(0x8D); //--set DC-DC enable
//     ssd1306_WriteCommand(0x14); //
//     ssd1306_SetDisplayOn(1); //--turn on SSD1306 panel

//     // Clear screen
//     ssd1306_Fill(Black);

//     // Flush buffer to screen
//     ssd1306_UpdateScreen();

//     // Set default values for screen object
//     // SSD1306.CurrentX = 0;
//     // SSD1306.CurrentY = 0;

//     // SSD1306.Initialized = 1;
//     NRF_LOG_INFO("ssd1306_Init done");
}

void ssd1306_set_px_cb(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
        lv_color_t color, lv_opa_t opa)
{
    uint16_t byte_index = x + (( y>>3 ) * buf_w);
    uint8_t  bit_index  = y & 0x7;

    if ((color.full == 0) && (LV_OPA_TRANSP != opa)) {
        BIT_SET(buf[byte_index], bit_index);
    } else {
        BIT_CLEAR(buf[byte_index], bit_index);
    }
}

void ssd1306_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /* Divide by 8 */
    uint8_t row1 = area->y1 >> 3;
    uint8_t row2 = area->y2 >> 3;

    uint8_t conf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_SET_MEMORY_ADDR_MODE,
        0x00,
        OLED_CMD_SET_COLUMN_RANGE,
        (uint8_t) area->x1,
        (uint8_t) area->x2,
        OLED_CMD_SET_PAGE_RANGE,
        row1,
        row2,
    };

    uint8_t err = send_data(disp_drv, conf, sizeof(conf));
//    assert(0 == err);
    err = send_pixels(disp_drv, color_p, OLED_COLUMNS * (1 + row2 - row1));
//    assert(0 == err);

    lv_disp_flush_ready(disp_drv);
}

void ssd1306_rounder(lv_disp_drv_t * disp_drv, lv_area_t *area)
{
    uint8_t hor_max = disp_drv->hor_res;
    uint8_t ver_max = disp_drv->ver_res;

    area->x1 = 0;
    area->y1 = 0;
    area->x2 = hor_max - 1;
    area->y2 = ver_max - 1;
}

void ssd1306_sleep_in(void)
{
    uint8_t conf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_OFF
    };

    uint8_t err = send_data(NULL, conf, sizeof(conf));
//    assert(0 == err);
}

void ssd1306_sleep_out(void)
{
    uint8_t conf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_ON
    };

    uint8_t err = send_data(NULL, conf, sizeof(conf));
//    assert(0 == err);
}
/***********************************************************************************************************************
* static functions
***********************************************************************************************************************/
static uint8_t send_data(lv_disp_drv_t *disp_drv, void *bytes, size_t bytes_len)
{ // command
    (void) disp_drv;
    uint8_t *data = (uint8_t *) bytes;
    for (int idx = 0; idx < bytes_len; idx++)
    {
      user_i2c_write(OLED_I2C_ADDRESS, 0x00, &data[idx], 1);
    }
    return true;
}

static uint8_t send_pixels(lv_disp_drv_t *disp_drv, void *color_buffer, size_t buffer_len)
{
    (void) disp_drv;
    uint8_t *data = (uint8_t *) color_buffer;
	for (int idx = 0; idx < buffer_len; idx++)
	{
        user_i2c_write(OLED_I2C_ADDRESS, 0x40, &data[idx], 1);
	}
    return true;
}

/////////////////// test
// static void ssd1306_SetDisplayOn(const uint8_t on) {
//     uint8_t value;
//     if (on) {
//         value = 0xAF;   // Display on
// //        SSD1306.DisplayOn = 1;
//     } else {
//         value = 0xAE;   // Display off
// //        SSD1306.DisplayOn = 0;
//     }
//     ssd1306_WriteCommand(value);
// }

// // Send a byte to the command register
// static void ssd1306_WriteCommand(uint8_t byte) {
//     spi_gpio_clear_dc();
//     //spiWriteVal(byte);
//         user_spi_send_data(&byte, 1);
// }

static void ssd1306_Reset(void) {
//    // Reset the OLED
//    spi_gpio_clear_res();
//    nrf_delay_ms(100);
//    spi_gpio_set_res();
//    nrf_delay_ms(100);
}

// static void ssd1306_SetContrast(const uint8_t value) {
//     const uint8_t kSetContrastControlRegister = 0x81;
//     ssd1306_WriteCommand(kSetContrastControlRegister);
//     ssd1306_WriteCommand(value);
// }


// // Write the screenbuffer with changed to the screen
// static void ssd1306_UpdateScreen(void) {
//     // Write data to each page of RAM. Number of pages
//     // depends on the screen height:
//     //
//     //  * 32px   ==  4 pages
//     //  * 64px   ==  8 pages
//     //  * 128px  ==  16 pages
//     for(uint8_t i = 0; i < OLED_HEIGHT/8; i++) {
//         ssd1306_WriteCommand(0xB0 + i); // Set the current RAM page address.
//         ssd1306_WriteCommand(0x00);
//         ssd1306_WriteCommand(0x10);
//         ssd1306_WriteData(&SSD1306_Buffer[OLED_WIDTH*i],OLED_WIDTH);
//     }
// }

// static void ssd1306_Fill(SSD1306_COLOR color) {
//     /* Set memory */
//     uint32_t i;

//     for(i = 0; i < sizeof(SSD1306_Buffer); i++) {
//         SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
//     }
// }

// // Send data
// static void ssd1306_WriteData(uint8_t* buffer, size_t buff_size) {
//     spi_gpio_set_dc();
//     for (int i = 0; i < buff_size; i++)
//     {
//         //spiWriteVal(buffer[i]);
//             user_spi_send_data( &buffer[i], 1);
//     }
// }

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/

