#include "tetris.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

#define CELL_SIZE 12
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 19
#define FIELD_OFFSET ((LCD_WIDTH - FIELD_WIDTH*CELL_SIZE)/2 - 1)
#define HEADER_HEIGHT 5
#define FIELD_END (HEADER_HEIGHT + FIELD_HEIGHT*CELL_SIZE)
#define SCORE_OFFSET ((FIELD_OFFSET-5*Font8.Width)/2)
static_assert(FIELD_OFFSET > 0);
static_assert(HEADER_HEIGHT + CELL_SIZE * FIELD_HEIGHT + 1 <= LCD_WIDTH);

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

typedef struct _tetris_point {
    uint8_t x;
    uint8_t y;
} tetris_point_t;

static const tetris_point_t tetris_shape[_SHAPE_COUNT][4] = {
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
    uint8_t score;
    uint8_t piece_x;
    uint8_t piece_y;
    tetris_shape_t piece_shape;
    tetris_rotation_t piece_rotation;
    bool field[FIELD_WIDTH][FIELD_HEIGHT];
    uint8_t frame_num;
} tetris_application_data_t;

#define APP_DATA ((tetris_application_data_t*)pico_application_data)

static void tetris_get_cells(tetris_point_t *cells) {
    memcpy(cells, tetris_shape[APP_DATA->piece_shape], sizeof(tetris_shape[APP_DATA->piece_shape]));
    for (int i = 0; i < 4; i++) {
        cells[i].x += APP_DATA->piece_x;
        cells[i].y += APP_DATA->piece_y;
    }
    // TODO: Add rotation
}

typedef enum _DRAW_OR_ERASE {
    DRAW,
    ERASE,
} draw_or_erase_t;

static void tetris_draw_or_erase(draw_or_erase_t command, const tetris_point_t *cells, int cell_count) {
    for (int i = 0; i < cell_count; i++) {
        uint8_t x1 = cells[i].x*CELL_SIZE + FIELD_OFFSET + 1;
        uint8_t x2 = (cells[i].x+1)*CELL_SIZE + FIELD_OFFSET + 1;
        uint8_t y1 = cells[i].y*CELL_SIZE + HEADER_HEIGHT;
        uint8_t y2 = (cells[i].y+1)*CELL_SIZE + HEADER_HEIGHT;
        if (command == ERASE) {
            pico_lcd_fill_rect(x1, x2-1, y1, y2-1, COLOR_BLACK);
        } else if (command == DRAW) {
            pico_ui_draw_rect(x1, x1, y1, y2-1, COLOR_LIGHT);
            pico_ui_draw_rect(x1, x2-1, y1, y1, COLOR_LIGHT);
            pico_ui_draw_rect(x1+1, x1+1, y1+1, y2-2, COLOR_LIGHT);
            pico_ui_draw_rect(x1+1, x2-2, y1+1, y1+1, COLOR_LIGHT);
            pico_lcd_fill_rect(x1+2, x2-3, y1+2, y2-2, COLOR_CELL);
            pico_ui_draw_rect(x1, x2-1, y2-1, y2-1, COLOR_DARK);
            pico_ui_draw_rect(x2-1, x2-1, y1, y2-1, COLOR_DARK);
            pico_ui_draw_rect(x1+1, x2-2, y2-2, y2-2, COLOR_DARK);
            pico_ui_draw_rect(x2-2, x2-2, y1+1, y2-2, COLOR_DARK);
        }
    }
}

static void tetris_draw_score(void) {
    char buf[3] = { '0' + APP_DATA->score / 10, '0' + APP_DATA->score %10, '\0' };
    bool double_digit = APP_DATA->score >= 10;
    pico_ui_draw_string(double_digit ? buf : buf+1,
        (FIELD_OFFSET-(double_digit ? 2 : 1)*Font16.Width)/2, SCORE_OFFSET + 11,
        &Font16, COLOR_RED, COLOR_BLACK);
}

static void tetris_advance(void) {
    APP_DATA->piece_x = FIELD_WIDTH/2 - 1;
    APP_DATA->piece_y = 0;
    APP_DATA->piece_shape = time_us_32() % _SHAPE_COUNT;
    APP_DATA->piece_rotation = time_us_32() % 4;

    tetris_point_t cells[4];
    tetris_get_cells(cells);
    for (int i = 0; i < 4; i++) {
        if (APP_DATA->field[cells[i].x][cells[i].y]) {
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
    pico_lcd_fill_rect(FIELD_OFFSET, FIELD_OFFSET, HEADER_HEIGHT, FIELD_END, COLOR_GRAY);
    pico_lcd_fill_rect(FIELD_OFFSET, LCD_WIDTH-FIELD_OFFSET, FIELD_END, FIELD_END, COLOR_GRAY);
    pico_lcd_fill_rect(LCD_WIDTH-FIELD_OFFSET, LCD_WIDTH-FIELD_OFFSET, HEADER_HEIGHT, FIELD_END, COLOR_GRAY);

    pico_ui_draw_rect(SCORE_OFFSET-2, FIELD_OFFSET-SCORE_OFFSET+2, SCORE_OFFSET-2, FIELD_OFFSET-SCORE_OFFSET+2, COLOR_GRAY);
    pico_ui_draw_string("LINES", SCORE_OFFSET, SCORE_OFFSET, &Font8, COLOR_WHITE, COLOR_BLACK);
    tetris_draw_score();
    tetris_advance();
}

static bool tetris_try_move_piece(tetris_point_t *cells, int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        int cell_x = cells[i].x + dx;
        int cell_y = cells[i].y + dy;
        if (cell_x < 0 || cell_x > FIELD_WIDTH - 1 ||
            cell_y > FIELD_HEIGHT - 1 || APP_DATA->field[cell_x][cell_y])
        {
            // can't move
            return false;
        }
    }

    // The LCD screen is slow, so excessive updates create visible flickering,
    // but the CPU is [relatively] fast, so we will try to eliminate unnecessary
    // screen updates at the cost of extra computation.
    bool field[4][4];
    memset(field, 0, sizeof(field));
    for (int i = 0; i < 4; i++) {
        int x = cells[i].x-APP_DATA->piece_x;
        int y = cells[i].y-APP_DATA->piece_y;
        field[x][y] = true;
    }
    for (int i = 0; i < 4; i++) {
        cells[i].x += dx;
        cells[i].y += dy;
        int x = cells[i].x-APP_DATA->piece_x;
        int y = cells[i].y-APP_DATA->piece_y;
        if (x >= 0 && x < 4 && y >= 0 && y < 4 && field[x][y]) {
            // If the new cell was already occupied, mark it as used
            field[x][y] = false;
        } else {
            // If the new cell was not occupied yet, draw it
            tetris_draw_or_erase(DRAW, &cells[i], 1);
        }
    }
    // Erase all cells that existed but were not reused
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            if (field[x][y]) {
                tetris_point_t cell = { x+APP_DATA->piece_x, y+APP_DATA->piece_y };
                tetris_draw_or_erase(ERASE, &cell, 1);
            }
        }
    }
    APP_DATA->piece_x += dx;
    APP_DATA->piece_y += dy;
    return true;
}

static inline void tetris_erase_complete_lines(void) {
    bool filled[FIELD_HEIGHT];
    for (int y = 0; y < FIELD_HEIGHT; y++) {
        filled[y] = true;
        for (int x = 0; x < FIELD_WIDTH; x++) {
            if (!APP_DATA->field[x][y]) {
                filled[y] = false;
                break;
            }
        }
        // update APP_DATA->field
        if (filled[y]) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                APP_DATA->field[x][y] = false;
            }
            APP_DATA->score++;
        }
    }

    // erase lines with animation to match APP_DATA->field
    for (int x = FIELD_OFFSET + 1; x < LCD_WIDTH - FIELD_OFFSET - 1; x++) {
        for (int y = 0; y < FIELD_HEIGHT; y++) {
            if (filled[y]) {
                pico_lcd_fill_rect(x, x, HEADER_HEIGHT + y * CELL_SIZE, HEADER_HEIGHT + (y+1) * CELL_SIZE - 1, COLOR_BLACK);
            }
        }
        sleep_ms(3);
    }

    for (int y = FIELD_HEIGHT-1, y2 = FIELD_HEIGHT-1; y >= 0; y--, y2--) {
        // ensure row y2 is not filled
        while (y2 >= 0 && filled[y2]) {
            y2--;
        }
        if (y != y2) {
            for (int x = 0; x < FIELD_WIDTH; x++) {
                // update screen
                bool is = APP_DATA->field[x][y];
                bool should = y2 >= 0 && APP_DATA->field[x][y2];
                if (is && !should) {
                    tetris_point_t p = { x, y };
                    tetris_draw_or_erase(ERASE, &p, 1);
                    APP_DATA->field[x][y] = false;
                } else if (should && !is) {
                    tetris_point_t p = { x, y };
                    tetris_draw_or_erase(DRAW, &p, 1);
                    APP_DATA->field[x][y] = true;
                }
            }
        }
    }

    // update score
    tetris_draw_score();
}

static void tetris_run(void) {
    sleep_ms(100);

    tetris_point_t cells[4];
    tetris_get_cells(cells);
    if (pico_lcd_is_pressed(KEY_LEFT)) {
        tetris_try_move_piece(cells, -1, 0);
    } else if (pico_lcd_is_pressed(KEY_RIGHT)) {
        tetris_try_move_piece(cells, 1, 0);
    } else if (pico_lcd_is_pressed(KEY_DOWN)) {
        APP_DATA->frame_num = 10;
    }

    if (APP_DATA->frame_num >= 10) {
        if (!tetris_try_move_piece(cells, 0, 1)) {
            for (int i = 0; i < 4; i++) {
                APP_DATA->field[cells[i].x][cells[i].y] = true;
            }
            tetris_erase_complete_lines();
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