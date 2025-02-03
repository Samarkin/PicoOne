#ifndef __PICO_UI_H__
#define __PICO_UI_H__

#include "pico/types.h"
#include "fonts/fonts.h"

/*! \brief Draw a string on the LCD screen.
 * \param s Pointer to the null-terminated sequence of characters to draw
 * \param x Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param color Font color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 * \param bg_color Background color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 */
void pico_ui_draw_string(char *s, uint8_t x, uint8_t y, const sFONT* font, uint16_t color, uint16_t bg_color);

/*! \brief Draw a character on the LCD screen.
 * \param ch Character to draw
 * \param x Value between 0 and LCD_WIDTH-1 (inclusive)
 * \param y Value between 0 and LCD_HEIGHT-1 (inclusive)
 * \param color Font color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 * \param bg_color Background color in RGB 5:6:5 format (5 bits red, 6 bits green, 5 bits blue)
 */
void pico_ui_draw_char(char ch, uint8_t x, uint8_t y, const sFONT* font, uint16_t color, uint16_t bg_color);

#endif // __PICO_UI_H__
