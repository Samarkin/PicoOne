#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

#define CELL_SIZE 8
#define HEADER_HEIGHT 24

static const uint16_t apple_sprite[] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_GREEN, COLOR_GREEN, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BLACK, COLOR_BLACK, COLOR_RED,   COLOR_GREEN, COLOR_BLACK, COLOR_RED,   COLOR_BLACK, COLOR_BLACK, 
    COLOR_BLACK, COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_BLACK,
    COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   
    COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   
    COLOR_BLACK, COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_BLACK,
    COLOR_BLACK, COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_RED,   COLOR_BLACK,
    COLOR_BLACK, COLOR_BLACK, COLOR_RED,   COLOR_BLACK, COLOR_BLACK, COLOR_RED,   COLOR_BLACK, COLOR_BLACK, 
};
static_assert(sizeof(apple_sprite)/sizeof(uint16_t) == (CELL_SIZE * CELL_SIZE));

#define COLOR_BODY1 0xAF23
#define COLOR_BODY2 0x7E72
static const uint16_t body_sprite[] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2, COLOR_BLACK,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2, COLOR_BLACK,
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
};
static_assert(sizeof(body_sprite)/sizeof(uint16_t) == (CELL_SIZE * CELL_SIZE));

// South-East curve
static const uint16_t body_curve_sprite[] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2, COLOR_BLACK,
    COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BLACK,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BLACK,
};
static_assert(sizeof(body_curve_sprite)/sizeof(uint16_t) == (CELL_SIZE * CELL_SIZE));

#define COLOR_EYE   0x03FF
#define COLOR_MOUTH 0xF800
static const uint16_t head_sprite[] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_EYE,   COLOR_EYE,   COLOR_BODY1, COLOR_BODY1, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_EYE,   COLOR_EYE,   COLOR_EYE,   COLOR_BODY1, COLOR_BODY1, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_MOUTH, COLOR_MOUTH, COLOR_BLACK, 
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_MOUTH, COLOR_MOUTH, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
};
static_assert(sizeof(head_sprite)/sizeof(uint16_t) == (CELL_SIZE * CELL_SIZE));

static const uint16_t tail_sprite[] = {
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2, COLOR_BLACK,
    COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2,
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BODY1, COLOR_BODY1, COLOR_BODY2, COLOR_BLACK,
    COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, 
};
static_assert(sizeof(tail_sprite)/sizeof(uint16_t) == (CELL_SIZE * CELL_SIZE));

typedef enum _orientation {
    ORIENTATION_NORMAL = 0,
    ORIENTATION_ROTATE = 1,
    ORIENTATION_MIRROR = 2,
    ORIENTATION_FLIP   = 4,
} orientation_t;

struct _section;

typedef struct _section {
    struct _section* prev;
    struct _section* next;
    uint8_t x;
    uint8_t y;
} section_t;

typedef enum _state {
    STATE_INIT,
    STATE_MENU,
    STATE_START,
    STATE_GAME,
    STATE_PAUSE,
    STATE_WILL_GAME_OVER,
    STATE_GAME_OVER,
} state_t;

typedef enum _menu_item {
    MENU_ITEM_START,
    MENU_ITEM_GAME_SPEED,
    MENU_ITEM_BORDERS,
    MENU_ITEM_EXIT,
    _MENU_ITEM_COUNT,
} menu_item_t;

typedef struct _snake_application_data {
    uint length;
    int dirx, diry;
    uint8_t applex, appley;
    section_t *head, *tail;
    bool borderless_mode;
    bool just_turned;
    uint cycle_count;
    bool still_holding;
    state_t state;
    menu_item_t menu_selection;
} snake_application_data_t;

#define APP_DATA ((snake_application_data_t*)pico_application_data)

static void snake_set_state(state_t new_state);

#define FIELD_WIDTH (LCD_WIDTH/CELL_SIZE)
static_assert(LCD_WIDTH%CELL_SIZE == 0);

#define FIELD_HEIGHT ((LCD_HEIGHT - HEADER_HEIGHT)/CELL_SIZE)
static_assert((LCD_HEIGHT - HEADER_HEIGHT)%CELL_SIZE == 0);

static void snake_refresh_border(uint8_t x, uint8_t y) {
    if (APP_DATA->borderless_mode) return;
    if (x == 0) {
        pico_lcd_fill_rect(
            0, 0,
            y*CELL_SIZE + HEADER_HEIGHT, (y+1)*CELL_SIZE + HEADER_HEIGHT - 1,
            COLOR_GRAY);
    } else if (x == FIELD_WIDTH-1) {
        pico_lcd_fill_rect(
            LCD_WIDTH-1, LCD_WIDTH-1,
            y*CELL_SIZE + HEADER_HEIGHT, (y+1)*CELL_SIZE + HEADER_HEIGHT - 1,
            COLOR_GRAY);
    }
    if (y == 0) {
        pico_lcd_fill_rect(
            x*CELL_SIZE, (x+1)*CELL_SIZE - 1,
            HEADER_HEIGHT, HEADER_HEIGHT,
            COLOR_GRAY);
    } else if (y == FIELD_HEIGHT-1) {
        pico_lcd_fill_rect(
            x*CELL_SIZE, (x+1)*CELL_SIZE - 1,
            LCD_HEIGHT-1, LCD_HEIGHT-1,
            COLOR_GRAY);
    }
}

static void snake_draw_sprite(uint8_t x, uint8_t y, const uint16_t *image, orientation_t orientation) {
    if (orientation == ORIENTATION_NORMAL) {
        pico_lcd_draw_image(x*CELL_SIZE, y*CELL_SIZE + HEADER_HEIGHT, CELL_SIZE, CELL_SIZE, image);
        snake_refresh_border(x, y);
        return;
    }
    uint16_t copy[CELL_SIZE * CELL_SIZE];
    memcpy(copy, image, CELL_SIZE * CELL_SIZE * sizeof(uint16_t));
    if (orientation & ORIENTATION_ROTATE) {
        for (int i = 0; i < CELL_SIZE/2; i++) {
            for (int j = 0; j < CELL_SIZE/2; j++) {
                // (i,j) <- (CELL_SIZE-j,i) <- (CELL_SIZE-i,CELL_SIZE-j) <- (j,CELL_SIZE-i)
                uint16_t t = copy[i + j*CELL_SIZE];
                copy[i + j*CELL_SIZE] = copy[CELL_SIZE-j-1 + i*CELL_SIZE];
                copy[CELL_SIZE-j-1 + i*CELL_SIZE] = copy[CELL_SIZE-i-1 + (CELL_SIZE-j-1)*CELL_SIZE];
                copy[CELL_SIZE-i-1 + (CELL_SIZE-j-1)*CELL_SIZE] = copy[j + (CELL_SIZE-i-1)*CELL_SIZE];
                copy[j + (CELL_SIZE-i-1)*CELL_SIZE] = t;
            }
        }
    }
    if (orientation & ORIENTATION_MIRROR) {
        for (int i = 0; i < CELL_SIZE/2; i++) {
            for (int j = 0; j < CELL_SIZE; j++) {
                uint16_t t = copy[i + j*CELL_SIZE];
                copy[i + j*CELL_SIZE] = copy[CELL_SIZE-i-1 + j*CELL_SIZE];
                copy[CELL_SIZE-i-1 + j*CELL_SIZE] = t;
            }
        }
    }
    if (orientation & ORIENTATION_FLIP) {
        for (int i = 0; i < CELL_SIZE; i++) {
            for (int j = 0; j < CELL_SIZE/2; j++) {
                uint16_t t = copy[i + j*CELL_SIZE];
                copy[i + j*CELL_SIZE] = copy[i + (CELL_SIZE-j-1)*CELL_SIZE];
                copy[i + (CELL_SIZE-j-1)*CELL_SIZE] = t;
            }
        }
    }
    snake_draw_sprite(x, y, copy, ORIENTATION_NORMAL);
    snake_refresh_border(x, y);
}

static void snake_erase(uint8_t x, uint8_t y) {
    pico_lcd_fill_rect(
        x*CELL_SIZE, (x+1)*CELL_SIZE - 1,
        y*CELL_SIZE + HEADER_HEIGHT, (y+1)*CELL_SIZE + HEADER_HEIGHT - 1,
        COLOR_BLACK);
    snake_refresh_border(x, y);
}

static void snake_draw_score(void) {
    char score_str[14];
    sprintf(score_str, "SCORE: %d\0", APP_DATA->length - 3);
    pico_ui_draw_string(score_str, 0, 4, &Font16, COLOR_WHITE, COLOR_BLACK); 
}

static void snake_randomize_apple(void) {
    APP_DATA->applex = time_us_32() % FIELD_WIDTH;
    // two consequtive calls to `time_us_32` would always be very similar,
    // so XOR it with some random value
    APP_DATA->appley = (time_us_32() ^ 0x5F26A193) % FIELD_HEIGHT;
    snake_draw_sprite(APP_DATA->applex, APP_DATA->appley, apple_sprite, ORIENTATION_NORMAL);
    printf("Apple placed at (%d,%d)\n", APP_DATA->applex, APP_DATA->appley);
}

static inline void snake_run_start(void) {
    pico_lcd_clear();

    section_t *s1 = malloc(sizeof(section_t));
    s1->x = FIELD_WIDTH/2;
    s1->y = FIELD_HEIGHT/2;
    snake_draw_sprite(s1->x, s1->y, head_sprite, ORIENTATION_NORMAL);

    section_t *s2 = malloc(sizeof(section_t));
    s2->x = FIELD_WIDTH/2 - 1;
    s2->y = FIELD_HEIGHT/2;
    snake_draw_sprite(s2->x, s2->y, body_sprite, ORIENTATION_NORMAL);

    section_t *s3 = malloc(sizeof(section_t));
    s3->x = FIELD_WIDTH/2 - 2;
    s3->y = FIELD_HEIGHT/2;
    snake_draw_sprite(s3->x, s3->y, tail_sprite, ORIENTATION_NORMAL);

    s1->prev = NULL;
    s1->next = s2;
    s2->prev = s1;
    s2->next = s3;
    s3->prev = s2;
    s3->next = NULL;

    APP_DATA->length = 3;
    APP_DATA->head = s1;
    APP_DATA->tail = s3;
    APP_DATA->dirx = 1;
    APP_DATA->diry = 0;
    snake_set_state(STATE_GAME);
    APP_DATA->just_turned = false;

    snake_randomize_apple();

    snake_draw_score();
    if (APP_DATA->borderless_mode) {
        pico_lcd_fill_rect(0, LCD_WIDTH-1, HEADER_HEIGHT-1, HEADER_HEIGHT-1, COLOR_GRAY);
    } else {
        pico_ui_draw_rect(0, LCD_WIDTH-1, HEADER_HEIGHT, LCD_HEIGHT-1, COLOR_GRAY);
    }
}

static void snake_start(void) {
    snake_set_state(STATE_INIT);
}

static void snake_stop(void) {
    section_t *s = APP_DATA->head;
    while (s != NULL) {
        section_t *old_s = s;
        s = s->next;
        free(old_s);
    }
    APP_DATA->head = NULL;
    APP_DATA->tail = NULL;
    APP_DATA->length = 0;
}

static void snake_set_state(state_t new_state) {
    APP_DATA->cycle_count = 0;
    APP_DATA->still_holding = true;
    APP_DATA->state = new_state;
}

static bool snake_is_cell_occupied(uint8_t x, uint8_t y) {
    section_t *s = APP_DATA->head;
    while (s != NULL) {
        if (s->x == x && s->y == y) {
            return true;
        }
        s = s->next;
    }
    return false;
}

static void snake_draw_rect_around(menu_item_t item, uint16_t color) {
    uint y = LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2 + Font20.Height*item + (item?14:0);
    pico_ui_draw_rect(10, LCD_WIDTH-11, y-1, y+Font20.Height-3, color);
}

static void snake_draw_menu(void) {
    pico_ui_draw_string("START", LCD_WIDTH/2 - 5*Font20.Width/2,
        LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2,
        &Font20, COLOR_WHITE, COLOR_BLACK);
    pico_ui_draw_string("GAME SPEED:  3", LCD_WIDTH/2 - 7*Font20.Width,
        LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2 + Font20.Height + 14,
        &Font20, COLOR_WHITE, COLOR_BLACK);
    pico_ui_draw_string(APP_DATA->borderless_mode ? "BORDERS: OFF" : "BORDERS:  ON",
        LCD_WIDTH/2 - 6*Font20.Width,
        LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2 + 2*Font20.Height + 14,
        &Font20, COLOR_WHITE, COLOR_BLACK);
    pico_ui_draw_string("EXIT", LCD_WIDTH/2 - 2*Font20.Width,
        LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2 + 3*Font20.Height + 14,
        &Font20, COLOR_WHITE, COLOR_BLACK);

    snake_draw_rect_around(APP_DATA->menu_selection, COLOR_WHITE);
}

static inline void snake_run_init(void) {
    APP_DATA->menu_selection = MENU_ITEM_START;

    pico_lcd_clear();
    snake_draw_menu();
    snake_set_state(STATE_MENU);
}

static inline void snake_run_menu(void) {
    bool full_redraw = false;
    if (pico_lcd_is_pressed(KEY_DOWN)) {
        if (!APP_DATA->still_holding) {
            snake_draw_rect_around(APP_DATA->menu_selection, COLOR_BLACK);
            APP_DATA->menu_selection = (APP_DATA->menu_selection + 1) % _MENU_ITEM_COUNT;
            snake_draw_rect_around(APP_DATA->menu_selection, COLOR_WHITE);
            APP_DATA->still_holding = true;
        }
    }
    else if (pico_lcd_is_pressed(KEY_UP)) {
        if (!APP_DATA->still_holding) {
            snake_draw_rect_around(APP_DATA->menu_selection, COLOR_BLACK);
            APP_DATA->menu_selection = (APP_DATA->menu_selection + _MENU_ITEM_COUNT - 1) % _MENU_ITEM_COUNT;
            snake_draw_rect_around(APP_DATA->menu_selection, COLOR_WHITE);
            APP_DATA->still_holding = true;
        }
    }
    else if (pico_lcd_is_pressed(KEY_LEFT)) {
        if (!APP_DATA->still_holding) {
            if (APP_DATA->menu_selection == MENU_ITEM_BORDERS) {
                APP_DATA->borderless_mode = !APP_DATA->borderless_mode;
                full_redraw = true;
            }
        }
    }
    else if (pico_lcd_is_pressed(KEY_RIGHT)) {
        if (!APP_DATA->still_holding) {
            if (APP_DATA->menu_selection == MENU_ITEM_BORDERS) {
                APP_DATA->borderless_mode = !APP_DATA->borderless_mode;
                full_redraw = true;
            }
        }
    }
    else if (pico_lcd_is_pressed(KEY_A)) {
        if (!APP_DATA->still_holding) {
            if (APP_DATA->menu_selection == MENU_ITEM_START) {
                snake_set_state(STATE_START);
            }
            else if (APP_DATA->menu_selection == MENU_ITEM_BORDERS) {
                APP_DATA->borderless_mode = !APP_DATA->borderless_mode;
                full_redraw = true;
            }
            else if (APP_DATA->menu_selection == MENU_ITEM_EXIT) {
                pico_application_stop();
            }
        }
    }
    else {
        APP_DATA->still_holding = false;
    }

    if (full_redraw) {
        APP_DATA->still_holding = true;
        snake_draw_menu();
    }
}

static inline void snake_run_will_game_over(void) {
    printf("Game over with score %d\n", APP_DATA->length-3);
    pico_ui_draw_string("GAME\nOVER", LCD_WIDTH/2-2*Font24.Width, LCD_HEIGHT/2-Font24.Height,
        &Font24, COLOR_RED, COLOR_BLACK); 
    snake_set_state(STATE_GAME_OVER);
}

static inline void snake_run_game_over(void) {
    if (pico_lcd_is_pressed(KEY_A)) {
        snake_stop();
        snake_start();
    }
}

static inline void snake_run_pause(void) {
    bool is_a_pressed = pico_lcd_is_pressed(KEY_A);
    APP_DATA->still_holding &= is_a_pressed;
    if (is_a_pressed && !APP_DATA->still_holding) {
        pico_lcd_fill_rect(LCD_WIDTH-6*Font16.Width, LCD_WIDTH-1, 0, HEADER_HEIGHT-2, COLOR_BLACK);
        snake_set_state(STATE_GAME);
        return;
    }
    if (APP_DATA->cycle_count % 20 == 1) {
        pico_ui_draw_string("PAUSE", LCD_WIDTH-6*Font16.Width, 4, &Font16, COLOR_RED, COLOR_BLACK);
    } else if (APP_DATA->cycle_count % 10 == 1) {
        pico_lcd_fill_rect(LCD_WIDTH-6*Font16.Width, LCD_WIDTH-1, 0, HEADER_HEIGHT-2, COLOR_BLACK);
    }
}

static inline void snake_run_game(void) {
    bool is_a_pressed = pico_lcd_is_pressed(KEY_A);
    APP_DATA->still_holding &= is_a_pressed;
    // only allow 90 degree turns
    if (pico_lcd_is_pressed(KEY_LEFT) && APP_DATA->diry != 0) {
        APP_DATA->dirx = -1;
        APP_DATA->diry = 0;
        APP_DATA->just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_RIGHT) && APP_DATA->diry != 0) {
        APP_DATA->dirx = 1;
        APP_DATA->diry = 0;
        APP_DATA->just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_UP) && APP_DATA->dirx != 0) {
        APP_DATA->dirx = 0;
        APP_DATA->diry = -1;
        APP_DATA->just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_DOWN) && APP_DATA->dirx != 0) {
        APP_DATA->dirx = 0;
        APP_DATA->diry = 1;
        APP_DATA->just_turned = true;
    } else if (is_a_pressed && !APP_DATA->still_holding) {
        snake_set_state(STATE_PAUSE);
    } else if (pico_lcd_is_pressed(KEY_X)) {
        if (!APP_DATA->just_turned) {
            int t = APP_DATA->dirx;
            APP_DATA->dirx = APP_DATA->diry;
            APP_DATA->diry = -t;
            APP_DATA->just_turned = true;
        }
    } else if (pico_lcd_is_pressed(KEY_Y)) {
        if (!APP_DATA->just_turned) {
            int t = APP_DATA->dirx;
            APP_DATA->dirx = -APP_DATA->diry;
            APP_DATA->diry = t;
            APP_DATA->just_turned = true;
        }
    } else {
        APP_DATA->just_turned = false;
    }

    // determine new head position
    int newx = APP_DATA->head->x + APP_DATA->dirx;
    int newy = APP_DATA->head->y + APP_DATA->diry;
    if (APP_DATA->borderless_mode) {
        newx = (newx+FIELD_WIDTH)%FIELD_WIDTH;
        newy = (newy+FIELD_HEIGHT)%FIELD_HEIGHT;
    }
    if (newx < 0 || newx >= FIELD_WIDTH || newy < 0 || newy >= FIELD_HEIGHT || snake_is_cell_occupied(newx, newy)) {
        snake_set_state(STATE_WILL_GAME_OVER);
        return;
    }

    section_t* new_head;
    if (newx == APP_DATA->applex && newy == APP_DATA->appley) {
        // grow new section
        new_head = malloc(sizeof(section_t));
        snake_randomize_apple();
        APP_DATA->length += 1;
        snake_draw_score();
        printf("Snake has %d sections\n", APP_DATA->length);
    } else {
        // erase tail
        if (!(APP_DATA->tail->x == APP_DATA->applex && APP_DATA->tail->y == APP_DATA->appley)) {
            snake_erase(APP_DATA->tail->x, APP_DATA->tail->y);
        }
        // draw new tail
        if (!(APP_DATA->tail->prev->x == APP_DATA->applex && APP_DATA->tail->prev->y == APP_DATA->appley)) {
            orientation_t tail_orientation;
            int tail_dirx = (APP_DATA->tail->prev->prev->x - APP_DATA->tail->prev->x + FIELD_WIDTH)%FIELD_WIDTH;
            int tail_diry = (APP_DATA->tail->prev->prev->y - APP_DATA->tail->prev->y + FIELD_HEIGHT)%FIELD_HEIGHT;
            if (tail_dirx == 1) {
                tail_orientation = ORIENTATION_NORMAL;
            } else if (tail_dirx == FIELD_WIDTH-1) {
                tail_orientation = ORIENTATION_MIRROR;
            } else if (tail_diry == 1) {
                tail_orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
            } else if (tail_diry == FIELD_HEIGHT-1) {
                tail_orientation = ORIENTATION_ROTATE;
            }
            snake_draw_sprite(APP_DATA->tail->prev->x, APP_DATA->tail->prev->y, tail_sprite, tail_orientation);
        }
        // repurpose tail section
        new_head = APP_DATA->tail;
        APP_DATA->tail = APP_DATA->tail->prev;
        APP_DATA->tail->next = NULL;
    }

    // draw body segment where the current head is
    const uint16_t *sprite;
    orientation_t orientation;
    int head_dirx = (newx - APP_DATA->head->next->x + FIELD_WIDTH)%FIELD_WIDTH;
    int head_diry = (newy - APP_DATA->head->next->y + FIELD_HEIGHT)%FIELD_HEIGHT;
    printf("(%d,%d) -> (%d,%d) -> (%d,%d)\n", APP_DATA->head->next->x, APP_DATA->head->next->y,
        APP_DATA->head->x, APP_DATA->head->y, newx, newy);
    printf("head_dir = (%d,%d)\n", head_dirx, head_diry);
    if (head_dirx == 2) {
        sprite = body_sprite;
        orientation = ORIENTATION_NORMAL;
    } else if (head_dirx == FIELD_WIDTH-2) {
        sprite = body_sprite;
        orientation = ORIENTATION_MIRROR;
    } else if (head_diry == 2) {
        sprite = body_sprite;
        orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
    } else if (head_diry == FIELD_HEIGHT-2) {
        sprite = body_sprite;
        orientation = ORIENTATION_ROTATE;
    } else if (head_dirx == 1) {
        sprite = body_curve_sprite;
        if (head_diry == 1) {
            if (APP_DATA->dirx == 1) {
                orientation = ORIENTATION_FLIP;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP | ORIENTATION_MIRROR;
            }
        } else {
            if (APP_DATA->dirx == 1) {
                orientation = ORIENTATION_NORMAL;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_MIRROR;
            }
        }
    } else if (head_dirx == FIELD_WIDTH-1) {
        sprite = body_curve_sprite;
        if (head_diry == FIELD_HEIGHT-1) {
            if (APP_DATA->dirx == 0) {
                orientation = ORIENTATION_ROTATE;
            } else {
                orientation = ORIENTATION_MIRROR;
            }
        } else {
            if (APP_DATA->dirx == 0) {
                orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
            } else {
                orientation = ORIENTATION_FLIP | ORIENTATION_MIRROR;
            }
        }
    }
    snake_draw_sprite(APP_DATA->head->x, APP_DATA->head->y, sprite, orientation);

    // place new head
    new_head->x = newx;
    new_head->y = newy;
    new_head->next = APP_DATA->head;
    new_head->prev = NULL;
    APP_DATA->head->prev = new_head;
    APP_DATA->head = new_head;

    // draw new head
    orientation_t head_orientation;
    if (APP_DATA->dirx == 1 && APP_DATA->diry == 0) {
        head_orientation = ORIENTATION_NORMAL;
    } else if (APP_DATA->dirx == -1 && APP_DATA->diry == 0) {
        head_orientation = ORIENTATION_MIRROR;
    } else if (APP_DATA->dirx == 0 && APP_DATA->diry == -1) {
        head_orientation = ORIENTATION_ROTATE;
    } else if (APP_DATA->dirx == 0 && APP_DATA->diry == 1) {
        head_orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
    }
    snake_draw_sprite(APP_DATA->head->x, APP_DATA->head->y, head_sprite, head_orientation);
}

static void snake_run(void) {
    sleep_ms(100);
    switch (APP_DATA->state)
    {
    case STATE_INIT:
        snake_run_init();
        break;
    case STATE_MENU:
        snake_run_menu();
        break;
    case STATE_START:
        snake_run_start();
        break;
    case STATE_GAME:
        snake_run_game();
        break;
    case STATE_PAUSE:
        snake_run_pause();
        break;
    case STATE_GAME_OVER:
        snake_run_game_over();
        break;
    case STATE_WILL_GAME_OVER:
        snake_run_will_game_over();
        break;
    }
    APP_DATA->cycle_count++;
}

const application_t snake_app = {
    .name = "Snake",
    .data_size = sizeof(snake_application_data_t),
    .start = snake_start,
    .run = snake_run,
    .stop = snake_stop,
};