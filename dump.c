#include "dump.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

#define ROM_SIZE 32768
#define ROM_END (ROM_BASE + ROM_SIZE)
#define XIP_SIZE 4194304

static const char HEX[16] = "0123456789ABCDEF";

typedef struct _dump_application_data {
    uint32_t offset, new_offset;
    uint32_t highlight_start, highlight_len;
} dump_application_data_t;

#define APP_DATA ((dump_application_data_t*)pico_application_data)

static void dump_start(void) {
    pico_lcd_clear();
    APP_DATA->offset = 0xFFFFFFFF;
    APP_DATA->new_offset = 0;
}

static uint32_t dump_safe_addr_add(uint32_t base, int distance) {
    uint32_t addr = base + distance;
    if (addr >= ROM_END && addr < XIP_BASE) {
        addr = addr - ROM_END + XIP_BASE;
    } else if (addr >= XIP_BASE+XIP_SIZE && addr < SRAM_BASE) {
        addr = addr - (XIP_BASE+XIP_SIZE) + SRAM_BASE;
    }
    return addr;
}

static void dump_run(void) {
    int step = pico_lcd_is_pressed(KEY_Y) ? 0x800 : 8;
    if (pico_lcd_is_pressed(KEY_DOWN) && APP_DATA->offset < SRAM_END - step) {
        APP_DATA->new_offset = dump_safe_addr_add(APP_DATA->offset, step);
    } else if (pico_lcd_is_pressed(KEY_UP) && APP_DATA->offset >= step) {
        APP_DATA->new_offset = dump_safe_addr_add(APP_DATA->offset, -step);
    } else if (pico_lcd_is_pressed(KEY_A)) {
        uint32_t addr = (uint32_t)(&APP_DATA->offset);
        APP_DATA->new_offset = addr & 0xFFFFFFE0;
        APP_DATA->highlight_start = addr;
        APP_DATA->highlight_len = 4;
    } else if (pico_lcd_is_pressed(KEY_B)) {
        uint32_t addr = (uint32_t)HEX;
        APP_DATA->new_offset = addr & 0xFFFFFFE0;
        APP_DATA->highlight_start = addr;
        APP_DATA->highlight_len = 16;
    } else if (pico_lcd_is_pressed(KEY_X)) {
        uint32_t addr = (uint32_t)dump_app.name;
        APP_DATA->new_offset = addr & 0xFFFFFFE0;
        APP_DATA->highlight_start = addr;
        APP_DATA->highlight_len = strlen(dump_app.name);
    } else if (pico_lcd_is_pressed(KEY_LEFT)) {
        pico_application_stop();
    }
    if (APP_DATA->new_offset == APP_DATA->offset) {
        return;
    }
    bool moving_up = APP_DATA->new_offset < APP_DATA->offset;
    APP_DATA->offset = APP_DATA->new_offset;
    char line[44];
    for (int i = moving_up ? 0 : LCD_HEIGHT/Font8.Height-1; moving_up ? (i < LCD_HEIGHT/Font8.Height) : (i >= 0); i += moving_up ? 1 : -1) {
        uint32_t addr = dump_safe_addr_add(APP_DATA->offset, 8*i);
        const uint8_t *mem = (const uint8_t *)addr;
        uint8_t y = i*Font8.Height;
        sprintf(line, "%08X:", addr);
        pico_ui_draw_string(line, 0, y, &Font8, COLOR_WHITE, COLOR_BLACK);
        pico_ui_draw_char('|', 23*Font8.Width, y, &Font8, COLOR_WHITE, COLOR_BLACK);
        pico_ui_draw_char('|', 37*Font8.Width, y, &Font8, COLOR_WHITE, COLOR_BLACK);
        for(int j = 0; j < 8; j++) {
            uint16_t color = (addr + j >= APP_DATA->highlight_start && addr + j < APP_DATA->highlight_start + APP_DATA->highlight_len)
                ? COLOR_YELLOW : COLOR_WHITE;
            pico_ui_draw_char(HEX[mem[j] / 16], (11 + j*3 + j/4*2)*Font8.Width, y, &Font8, color, COLOR_BLACK);
            pico_ui_draw_char(HEX[mem[j] % 16], (12 + j*3 + j/4*2)*Font8.Width, y, &Font8, color, COLOR_BLACK);
            pico_ui_draw_char(mem[j], (39+j)*Font8.Width, y, &Font8, color, COLOR_BLACK);
        }
    }
}

static void dump_stop(void) {
}

const application_t dump_app = {
    .name = "Memory Dump",
    .data_size = sizeof(dump_application_data_t),
    .start = dump_start,
    .run = dump_run,
    .stop = dump_stop,
};