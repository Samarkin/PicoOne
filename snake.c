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

static uint length;
static int dirx, diry;
static uint8_t applex, appley;
static section_t *head, *tail;
static bool borderless_mode = false;
static bool just_turned;
static uint cycle_count;
static bool still_holding;

static enum _state {
    STATE_INIT,
    STATE_MENU,
    STATE_START,
    STATE_GAME,
    STATE_PAUSE,
    STATE_WILL_GAME_OVER,
    STATE_GAME_OVER,
} state;

static void snake_set_state(enum _state new_state);

typedef enum _menu_item {
    MENU_ITEM_START,
    MENU_ITEM_GAME_SPEED,
    MENU_ITEM_BORDERS,
    _MENU_ITEM_COUNT,
} menu_item_t;
static menu_item_t menu_selection;

#define FIELD_WIDTH (LCD_WIDTH/CELL_SIZE)
static_assert(LCD_WIDTH%CELL_SIZE == 0);

#define FIELD_HEIGHT ((LCD_HEIGHT - HEADER_HEIGHT)/CELL_SIZE)
static_assert((LCD_HEIGHT - HEADER_HEIGHT)%CELL_SIZE == 0);

static void snake_refresh_border(uint8_t x, uint8_t y) {
    if (borderless_mode) return;
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
    sprintf(score_str, "SCORE: %d\0", length - 3);
    pico_ui_draw_string(score_str, 0, 4, &Font16, COLOR_WHITE, COLOR_BLACK); 
}

static void snake_randomize_apple(void) {
    applex = time_us_32() % FIELD_WIDTH;
    // two consequtive calls to `time_us_32` would always be very similar,
    // so XOR it with some random value
    appley = (time_us_32() ^ 0x5F26A193) % FIELD_HEIGHT;
    snake_draw_sprite(applex, appley, apple_sprite, ORIENTATION_NORMAL);
    printf("Apple placed at (%d,%d)\n", applex, appley);
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

    length = 3;
    head = s1;
    tail = s3;
    dirx = 1;
    diry = 0;
    snake_set_state(STATE_GAME);
    just_turned = false;

    printf("Snake starts with %d sections\n", length);
    snake_randomize_apple();

    snake_draw_score();
    if (borderless_mode) {
        pico_lcd_fill_rect(0, LCD_WIDTH-1, HEADER_HEIGHT-1, HEADER_HEIGHT-1, COLOR_GRAY);
    } else {
        pico_ui_draw_rect(0, LCD_WIDTH-1, HEADER_HEIGHT, LCD_HEIGHT-1, COLOR_GRAY);
    }
}

static void snake_start(void) {
    snake_set_state(STATE_INIT);
}

static void snake_stop(void) {
    section_t *s = head;
    while (s != NULL) {
        section_t *old_s = s;
        s = s->next;
        free(old_s);
    }
    head = NULL;
    tail = NULL;
    length = 0;
}

static void snake_set_state(enum _state new_state) {
    cycle_count = 0;
    still_holding = true;
    state = new_state;
}

static bool snake_is_cell_occupied(uint8_t x, uint8_t y) {
    section_t *s = head;
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
    pico_ui_draw_string(borderless_mode ? "BORDERS: OFF" : "BORDERS:  ON",
        LCD_WIDTH/2 - 6*Font20.Width,
        LCD_HEIGHT/2 - Font20.Height*_MENU_ITEM_COUNT/2 + 2*Font20.Height + 14,
        &Font20, COLOR_WHITE, COLOR_BLACK);

    snake_draw_rect_around(menu_selection, COLOR_WHITE);
}

static inline void snake_run_init(void) {
    menu_selection = MENU_ITEM_START;

    pico_lcd_clear();
    snake_draw_menu();
    snake_set_state(STATE_MENU);
}

static inline void snake_run_menu(void) {
    bool full_redraw = false;
    if (pico_lcd_is_pressed(KEY_DOWN)) {
        if (!still_holding) {
            snake_draw_rect_around(menu_selection, COLOR_BLACK);
            menu_selection = (menu_selection + 1) % _MENU_ITEM_COUNT;
            snake_draw_rect_around(menu_selection, COLOR_WHITE);
            still_holding = true;
        }
    }
    else if (pico_lcd_is_pressed(KEY_UP)) {
        if (!still_holding) {
            snake_draw_rect_around(menu_selection, COLOR_BLACK);
            menu_selection = (menu_selection + _MENU_ITEM_COUNT - 1) % _MENU_ITEM_COUNT;
            snake_draw_rect_around(menu_selection, COLOR_WHITE);
            still_holding = true;
        }
    }
    else if (pico_lcd_is_pressed(KEY_LEFT)) {
        if (!still_holding) {
            if (menu_selection == MENU_ITEM_BORDERS) {
                borderless_mode = !borderless_mode;
                full_redraw = true;
            }
        }
    }
    else if (pico_lcd_is_pressed(KEY_RIGHT)) {
        if (!still_holding) {
            if (menu_selection == MENU_ITEM_BORDERS) {
                borderless_mode = !borderless_mode;
                full_redraw = true;
            }
        }
    }
    else if (pico_lcd_is_pressed(KEY_A)) {
        if (!still_holding) {
            if (menu_selection == MENU_ITEM_START) {
                snake_set_state(STATE_START);
            }
            else if (menu_selection == MENU_ITEM_BORDERS) {
                borderless_mode = !borderless_mode;
                full_redraw = true;
            }
        }
    }
    else {
        still_holding = false;
    }

    if (full_redraw) {
        still_holding = true;
        snake_draw_menu();
    }
}

static inline void snake_run_will_game_over(void) {
    printf("Game over with score %d\n", length);
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
    still_holding &= is_a_pressed;
    if (is_a_pressed && !still_holding) {
        pico_lcd_fill_rect(LCD_WIDTH-6*Font16.Width, LCD_WIDTH-1, 0, HEADER_HEIGHT-2, COLOR_BLACK);
        snake_set_state(STATE_GAME);
        return;
    }
    if (cycle_count % 20 == 1) {
        pico_ui_draw_string("PAUSE", LCD_WIDTH-6*Font16.Width, 4, &Font16, COLOR_RED, COLOR_BLACK);
    } else if (cycle_count % 10 == 1) {
        pico_lcd_fill_rect(LCD_WIDTH-6*Font16.Width, LCD_WIDTH-1, 0, HEADER_HEIGHT-2, COLOR_BLACK);
    }
}

static inline void snake_run_game(void) {
    bool is_a_pressed = pico_lcd_is_pressed(KEY_A);
    still_holding &= is_a_pressed;
    // only allow 90 degree turns
    if (pico_lcd_is_pressed(KEY_LEFT) && diry != 0) {
        dirx = -1;
        diry = 0;
        just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_RIGHT) && diry != 0) {
        dirx = 1;
        diry = 0;
        just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_UP) && dirx != 0) {
        dirx = 0;
        diry = -1;
        just_turned = true;
    } else if (pico_lcd_is_pressed(KEY_DOWN) && dirx != 0) {
        dirx = 0;
        diry = 1;
        just_turned = true;
    } else if (is_a_pressed && !still_holding) {
        snake_set_state(STATE_PAUSE);
    } else if (pico_lcd_is_pressed(KEY_X)) {
        if (!just_turned) {
            int t = dirx;
            dirx = diry;
            diry = -t;
            just_turned = true;
        }
    } else if (pico_lcd_is_pressed(KEY_Y)) {
        if (!just_turned) {
            int t = dirx;
            dirx = -diry;
            diry = t;
            just_turned = true;
        }
    } else {
        just_turned = false;
    }

    // determine new head position
    int newx = head->x + dirx;
    int newy = head->y + diry;
    if (borderless_mode) {
        newx = (newx+FIELD_WIDTH)%FIELD_WIDTH;
        newy = (newy+FIELD_HEIGHT)%FIELD_HEIGHT;
    }
    if (newx < 0 || newx >= FIELD_WIDTH || newy < 0 || newy >= FIELD_HEIGHT || snake_is_cell_occupied(newx, newy)) {
        snake_set_state(STATE_WILL_GAME_OVER);
        return;
    }

    section_t* new_head;
    if (newx == applex && newy == appley) {
        // grow new section
        new_head = malloc(sizeof(section_t));
        snake_randomize_apple();
        length += 1;
        snake_draw_score();
        printf("Snake has %d sections\n", length);
    } else {
        // erase tail
        if (!(tail->x == applex && tail->y == appley)) {
            snake_erase(tail->x, tail->y);
        }
        // draw new tail
        if (!(tail->prev->x == applex && tail->prev->y == appley)) {
            orientation_t tail_orientation;
            int tail_dirx = (tail->prev->prev->x - tail->prev->x + FIELD_WIDTH)%FIELD_WIDTH;
            int tail_diry = (tail->prev->prev->y - tail->prev->y + FIELD_HEIGHT)%FIELD_HEIGHT;
            if (tail_dirx == 1) {
                tail_orientation = ORIENTATION_NORMAL;
            } else if (tail_dirx == FIELD_WIDTH-1) {
                tail_orientation = ORIENTATION_MIRROR;
            } else if (tail_diry == 1) {
                tail_orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
            } else if (tail_diry == FIELD_HEIGHT-1) {
                tail_orientation = ORIENTATION_ROTATE;
            }
            snake_draw_sprite(tail->prev->x, tail->prev->y, tail_sprite, tail_orientation);
        }
        // repurpose tail section
        new_head = tail;
        tail = tail->prev;
        tail->next = NULL;
    }

    // draw body segment where the current head is
    const uint16_t *sprite;
    orientation_t orientation;
    int head_dirx = (newx - head->next->x + FIELD_WIDTH)%FIELD_WIDTH;
    int head_diry = (newy - head->next->y + FIELD_HEIGHT)%FIELD_HEIGHT;
    printf("(%d,%d) -> (%d,%d) -> (%d,%d)\n", head->next->x, head->next->y, head->x, head->y, newx, newy);
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
            if (dirx == 1) {
                orientation = ORIENTATION_FLIP;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP | ORIENTATION_MIRROR;
            }
        } else {
            if (dirx == 1) {
                orientation = ORIENTATION_NORMAL;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_MIRROR;
            }
        }
    } else if (head_dirx == FIELD_WIDTH-1) {
        sprite = body_curve_sprite;
        if (head_diry == FIELD_HEIGHT-1) {
            if (dirx == 0) {
                orientation = ORIENTATION_ROTATE;
            } else {
                orientation = ORIENTATION_MIRROR;
            }
        } else {
            if (dirx == 0) {
                orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
            } else {
                orientation = ORIENTATION_FLIP | ORIENTATION_MIRROR;
            }
        }
    }
    snake_draw_sprite(head->x, head->y, sprite, orientation);

    // place new head
    new_head->x = newx;
    new_head->y = newy;
    new_head->next = head;
    new_head->prev = NULL;
    head->prev = new_head;
    head = new_head;

    // draw new head
    orientation_t head_orientation;
    if (dirx == 1 && diry == 0) {
        head_orientation = ORIENTATION_NORMAL;
    } else if (dirx == -1 && diry == 0) {
        head_orientation = ORIENTATION_MIRROR;
    } else if (dirx == 0 && diry == -1) {
        head_orientation = ORIENTATION_ROTATE;
    } else if (dirx == 0 && diry == 1) {
        head_orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
    }
    snake_draw_sprite(head->x, head->y, head_sprite, head_orientation);
}

static void snake_run(void) {
    sleep_ms(100);
    switch (state)
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
    cycle_count++;
}

const application_t snake_app = {
    .name = "Snake",
    .start = snake_start,
    .run = snake_run,
    .stop = snake_stop,
};