#include "pico_ui.h"

#include <stdio.h>
#include "pico_lcd.h"

void pico_ui_draw_string(char* s, uint8_t x, uint8_t y, const sFONT* font, uint16_t color, uint16_t bg_color) {
    uint8_t orig_x = x;
    uint16_t w = font->Width;
    uint16_t h = font->Height;
    char ch;
    while (ch = *(s++)) {
        if (ch != '\n') {
            pico_ui_draw_char(ch, x, y, font, color, bg_color);
            if (x < LCD_WIDTH) x += w;
        } else {
            y += h;
            if (y >= LCD_HEIGHT) break;
            x = orig_x;
        }
    }
}

void pico_ui_draw_char(char ch, uint8_t x, uint8_t y, const sFONT* font, uint16_t color, uint16_t bg_color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    if (ch < ' ' || ch > '~') {
        // Unsupported character
        ch = '?';
        uint16_t t = color;
        color = bg_color;
        bg_color = t;
    }
    uint16_t w = font->Width, h = font->Height;
    uint16_t line_stride = w / 8 + (w % 8 ? 1 : 0);
    if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - x;
    if (w == 0 || h == 0) return;
    uint16_t pixels[w*h];
    uint16_t idx = (ch - ' ') * h * line_stride;
    for (int j = 0; j < h; j++) {
        int i = 0;
        for (int k = 0; k < line_stride; k++, idx++) {
            uint8_t byte = font->table[idx];
            for (int l = 0; l < 8 && i < w; l++, i++) {
                pixels[i + j*w] = byte & 0x80 ? color : bg_color;
                byte <<= 1;
            }
        }
    }
    pico_lcd_draw_image(x, y, w, h, pixels);
}

void pico_ui_draw_rect(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint16_t color) {
    pico_lcd_fill_rect(x1, x1, y1, y2, color);
    pico_lcd_fill_rect(x1, x2, y1, y1, color);
    pico_lcd_fill_rect(x2, x2, y1, y2, color);
    pico_lcd_fill_rect(x1, x2, y2, y2, color);
}