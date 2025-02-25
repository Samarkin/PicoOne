#include "tetris.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

#define CELL_SIZE 12
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 18
#define FIELD_OFFSET ((LCD_WIDTH - FIELD_WIDTH*CELL_SIZE)/2 - 1)
#define HEADER_HEIGHT (LCD_HEIGHT - FIELD_HEIGHT*CELL_SIZE - 1)
static_assert(FIELD_OFFSET > 0);
static_assert(HEADER_HEIGHT > 0);

#define COLOR_CELL  0x8410 // 50% gray
#define COLOR_LIGHT 0xC618 // 75% gray
#define COLOR_DARK  0x4208 // 25% gray

typedef enum _tetris_shape {
    SHAPE_O,
    SHAPE_T,
    SHAPE_J,
    SHAPE_L,
    SHAPE_I,
    SHAPE_Z,
    SHAPE_S,
    _SHAPE_COUNT,
} tetris_shape_t;

static const uint8_t tetris_shape[_SHAPE_COUNT][4][2] = {
    { // SHAPE_O
        { 0,0 },
        { 0,1 },
        { 1,0 },
        { 1,1 },
    },
    { // SHAPE_T
        { 0,0 },
        { 0,1 },
        { 0,2 },
        { 1,1 },
    },
    { // SHAPE_J
        { 0,0 },
        { 0,1 },
        { 0,2 },
        { 1,2 },
    },
    { // SHAPE_L
        { 0,0 },
        { 0,1 },
        { 0,2 },
        { 1,0 },
    },
    { // SHAPE_I
        { 0,0 },
        { 0,1 },
        { 0,2 },
        { 0,3 },
    },
    { // SHAPE_Z
        { 0,0 },
        { 0,1 },
        { 1,1 },
        { 1,2 },
    },
    { // SHAPE_S
        { 0,1 },
        { 0,2 },
        { 1,0 },
        { 1,1 },
    },
};

typedef enum _tetris_rotation {
    ROTATION_0,
    ROTATION_90,
    ROTATION_180,
    ROTATION_270,
} tetris_rotation_t;

typedef struct _tetris_application_data {
    uint8_t piece_x;
    uint8_t piece_y;
    tetris_shape_t piece_shape;
    tetris_rotation_t piece_rotation;
    bool field[FIELD_WIDTH][FIELD_HEIGHT];
    uint8_t frame_num;
} tetris_application_data_t;

#define APP_DATA ((tetris_application_data_t*)pico_application_data)

static void tetris_get_cells(uint8_t cells[4][2]) {
    memcpy(cells, tetris_shape[APP_DATA->piece_shape], sizeof(tetris_shape[APP_DATA->piece_shape]));
    for (int i = 0; i < 4; i++) {
        cells[i][0] += APP_DATA->piece_x;
        cells[i][1] += APP_DATA->piece_y;
    }
    // TODO: Add rotation
}

typedef enum _DRAW_OR_ERASE {
    DRAW,
    ERASE,
} draw_or_erase_t;

static void tetris_draw_or_erase(draw_or_erase_t command, const uint8_t (*cells)[2], int cell_count) {
    for (int i = 0; i < cell_count; i++) {
        uint8_t x1 = cells[i][0]*CELL_SIZE + FIELD_OFFSET + 1;
        uint8_t x2 = (cells[i][0]+1)*CELL_SIZE + FIELD_OFFSET + 1;
        uint8_t y1 = cells[i][1]*CELL_SIZE + HEADER_HEIGHT;
        uint8_t y2 = (cells[i][1]+1)*CELL_SIZE + HEADER_HEIGHT;
        if (command == ERASE) {
            pico_lcd_fill_rect(x1, x2-1, y1, y2-1, COLOR_BLACK);
        } else if (command == DRAW) {
            pico_ui_draw_rect(x1, x1, y1, y2-1, COLOR_LIGHT);
            pico_ui_draw_rect(x1, x2-1, y1, y1, COLOR_LIGHT);
            pico_lcd_fill_rect(x1+1, x2-2, y1+1, y2-2, COLOR_CELL);
            pico_ui_draw_rect(x1, x2-1, y2-1, y2-1, COLOR_DARK);
            pico_ui_draw_rect(x2-1, x2-1, y1, y2-1, COLOR_DARK);
        }
    }
}

static void tetris_advance(void) {
    APP_DATA->piece_x = FIELD_WIDTH/2 - 1;
    APP_DATA->piece_y = 0;
    APP_DATA->piece_shape = time_us_32() % _SHAPE_COUNT;
    APP_DATA->piece_rotation = time_us_32() % 4;

    uint8_t cells[4][2];
    tetris_get_cells(cells);
    for (int i = 0; i < 4; i++) {
        int cell_x = cells[i][0];
        int cell_y = cells[i][1];
        if (APP_DATA->field[cell_x][cell_y]) {
            pico_ui_draw_string("GAME\nOVER", LCD_WIDTH/2-2*Font24.Width, LCD_HEIGHT/2-Font24.Height,
                &Font24, COLOR_RED, COLOR_BLACK); 
            while (!pico_lcd_is_pressed(KEY_A));
            pico_application_stop();
        }
    }
    tetris_draw_or_erase(DRAW, cells, 4);
}

static void tetris_start(void) {
    pico_lcd_clear();
    pico_lcd_fill_rect(FIELD_OFFSET, FIELD_OFFSET, HEADER_HEIGHT, LCD_HEIGHT-1, COLOR_GRAY);
    pico_lcd_fill_rect(FIELD_OFFSET, LCD_WIDTH-FIELD_OFFSET, LCD_HEIGHT-1, LCD_HEIGHT-1, COLOR_GRAY);
    pico_lcd_fill_rect(LCD_WIDTH-FIELD_OFFSET, LCD_WIDTH-FIELD_OFFSET, HEADER_HEIGHT, LCD_HEIGHT-1, COLOR_GRAY);
    tetris_advance();
}

static void tetris_run(void) {
    sleep_ms(100);

    uint8_t cells[4][2];
    tetris_get_cells(cells);
    int move = 0;
    if (pico_lcd_is_pressed(KEY_LEFT)) {
        move = -1;
    } else if (pico_lcd_is_pressed(KEY_RIGHT)) {
        move = 1;
    }
    if (move != 0) {
        bool can_move = true;
        for (int i = 0; i < 4; i++) {
            int cell_x = cells[i][0] + move;
            int cell_y = cells[i][1];
            if (cell_x < 0 || cell_x > FIELD_WIDTH - 1 || APP_DATA->field[cell_x][cell_y]) {
                can_move = false;
            }
        }
        if (can_move) {
            tetris_draw_or_erase(ERASE, cells, 4);
            for (int i = 0; i < 4; i++) {
                cells[i][0] += move;
            }
            APP_DATA->piece_x += move;
            tetris_draw_or_erase(DRAW, cells, 4);
        }
    }
    if (pico_lcd_is_pressed(KEY_DOWN)) {
        APP_DATA->frame_num = 10;
    }

    if (APP_DATA->frame_num >= 10) {
        bool can_fall = true;
        for (int i = 0; i < 4; i++) {
            int cell_x = cells[i][0];
            int cell_y = cells[i][1];
            if (cell_y + 1 > FIELD_HEIGHT - 1 || APP_DATA->field[cell_x][cell_y+1]) {
                can_fall = false;
            }
        }
        if (can_fall) {
            tetris_draw_or_erase(ERASE, cells, 4);
            for (int i = 0; i < 4; i++) {
                cells[i][1]++;
            }
            APP_DATA->piece_y++;
            tetris_draw_or_erase(DRAW, cells, 4);
        } else {
            for (int i = 0; i < 4; i++) {
                int cell_x = cells[i][0];
                int cell_y = cells[i][1];
                APP_DATA->field[cell_x][cell_y] = true;
            }
            tetris_advance();
        }
        APP_DATA->frame_num = 0;
    } else {
        APP_DATA->frame_num++;
    }
}

static void tetris_stop(void) {
}

const application_t tetris_app = {
    .name = "Tetris",
    .data_size = sizeof(tetris_application_data_t),
    .start = tetris_start,
    .run = tetris_run,
    .stop = tetris_stop,
};