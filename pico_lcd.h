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

/*! \brief Initialize the LCD module. Both screen and the keys.
 */
void pico_lcd_init(void);

/*! \brief Set one pixel on the LCD screen.
 * \param x Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param c Color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 */
void pico_lcd_set_pixel(uint8_t x, uint8_t y, uint16_t color);

/*! \brief Clear the entire screen (fill with black).
 */
void pico_lcd_clear(void);

/*! \brief Check if the key is pressed
 * \param key GPIO pin number (one of the `KEY_*` constants)
 * \return true if the key is pressed, false otherwise
 */
bool pico_lcd_is_pressed(uint key);

#endif //__PICO_LCD_H__