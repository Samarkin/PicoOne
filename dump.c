#include "dump.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

static uint32_t offset, new_offset;

static void dump_start(void) {
    pico_lcd_clear();
    offset = 0xFFFFFFFF;
    new_offset = 0;
}

static void dump_run(void) {
    //sleep_ms(10);
    if (pico_lcd_is_pressed(KEY_DOWN)) {
        new_offset = offset + 8;
    } else if (pico_lcd_is_pressed(KEY_UP) && offset >= 8) {
        new_offset = offset - 8;
    }
    if (new_offset == offset) {
        return;
    }
    bool moving_up = new_offset < offset;
    offset = new_offset;
    char line[44];
    for (int i = moving_up ? 0 : LCD_HEIGHT/Font8.Height-1; moving_up ? (i < LCD_HEIGHT/Font8.Height) : (i >= 0); i += moving_up ? 1 : -1) {
        const uint8_t *mem = (const uint8_t *)(offset + 8*i);
        sprintf(line, "%08X:  %02X %02X %02X %02X | %02X %02X %02X %02X | ",
            offset + 8*i,
            mem[0], mem[1], mem[2], mem[3], mem[4], mem[5], mem[6], mem[7]);
        pico_ui_draw_string(line, 0, i*Font8.Height, &Font8, COLOR_WHITE, COLOR_BLACK);
        for(int j = 0; j < 8; j++) {
            pico_ui_draw_char(mem[j], (39+j)*Font8.Width, i*Font8.Height, &Font8, COLOR_WHITE, COLOR_BLACK);
        }
    }
}

static void dump_stop(void) {
}

const application_t dump_app = {
    .name = "Memory Dump",
    .start = dump_start,
    .run = dump_run,
    .stop = dump_stop,
};