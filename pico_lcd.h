#ifndef __PICO_LCD_H__
#define __PICO_LCD_H__

#include "pico/types.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

#define KEY_A 15
#define KEY_B 17
#define KEY_X 19
#define KEY_Y 21
#define KEY_LEFT 16
#define KEY_DOWN 18
#define KEY_RIGHT 20
#define KEY_UP 2
#define KEY_CENTER 3

#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_GREEN 0x07E0
#define COLOR_GRAY  0x8410
#define COLOR_RED   0xF800
#define COLOR_YELLOW 0xFFE0

/*! \brief Initialize the LCD module. Both screen and the keys.
 */
void pico_lcd_init(void);

/*! \brief Set one pixel on the LCD screen.
 * \param x Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param color Color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 */
void pico_lcd_set_pixel(uint8_t x, uint8_t y, uint16_t color);

/*! \brief Fill rectangle on the LCD screen with a solid color.
 * \param x1 Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param x2 Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y1 Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param y2 Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param color Color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 */
void pico_lcd_fill_rect(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint16_t color);

/*! \brief Draw image on the LCD screen.
 * \param x Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param width Value between 0 and LCD_WIDTH-x (inclusive)
 * \param height Value between 0 and LCD_HEIGHT-y (inclusive)
 * \param pixels Array of width*height 16-bit pixels in RGB 5:6:5 format
 */
void pico_lcd_draw_image(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint16_t* pixels);

/*! \brief Clear the entire screen (fill with black).
 */
void pico_lcd_clear(void);

/*! \brief Check if the key is pressed
 * \param key GPIO pin number (one of the `KEY_*` constants)
 * \return true if the key is pressed, false otherwise
 */
bool pico_lcd_is_pressed(uint key);

#endif //__PICO_LCD_H__