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
static bool borderless_mode;
static bool just_turned;

static enum _state {
    STATE_RUN,
    STATE_WILL_GAME_OVER,
    STATE_GAME_OVER,
} state;

#define FIELD_WIDTH (LCD_WIDTH/CELL_SIZE)
static_assert(LCD_WIDTH%CELL_SIZE == 0);

#define FIELD_HEIGHT ((LCD_HEIGHT - HEADER_HEIGHT)/CELL_SIZE)
static_assert((LCD_HEIGHT - HEADER_HEIGHT)%CELL_SIZE == 0);

static void snake_draw_sprite(uint8_t x, uint8_t y, const uint16_t *image, orientation_t orientation) {
    if (orientation == ORIENTATION_NORMAL) {
        pico_lcd_draw_image(x*CELL_SIZE, y*CELL_SIZE + HEADER_HEIGHT, CELL_SIZE, CELL_SIZE, image);
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
}

static void snake_erase(uint8_t x, uint8_t y) {
    pico_lcd_fill_rect(
        x*CELL_SIZE, (x+1)*CELL_SIZE - 1,
        y*CELL_SIZE + HEADER_HEIGHT, (y+1)*CELL_SIZE + HEADER_HEIGHT - 1,
        COLOR_BLACK);
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

static void snake_start(void) {
    borderless_mode = pico_lcd_is_pressed(KEY_X);
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
    state = STATE_RUN;
    just_turned = false;

    printf("Snake starts with %d sections\n", length);
    snake_randomize_apple();

    snake_draw_score();
    if (borderless_mode) {
        pico_ui_draw_string("No borders", LCD_WIDTH-10*Font16.Width, 4, &Font16, COLOR_WHITE, COLOR_BLACK);
    }
    pico_lcd_fill_rect(0, LCD_WIDTH-1, HEADER_HEIGHT-1, HEADER_HEIGHT-1, COLOR_GRAY);
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

static void snake_run(void) {
    sleep_ms(100);

    if (state == STATE_WILL_GAME_OVER) {
        printf("Game over with score %d\n", length);
        pico_ui_draw_string("GAME\nOVER", LCD_WIDTH/2-2*Font24.Width, LCD_HEIGHT/2-Font24.Height,
            &Font24, COLOR_RED, COLOR_BLACK); 
        state = STATE_GAME_OVER;
        return;
    } else if (state == STATE_GAME_OVER) {
        if (pico_lcd_is_pressed(KEY_A)) {
            snake_stop();
            snake_start();
        }
        return;
    }
    // state == STATE_RUN

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
        state = STATE_WILL_GAME_OVER;
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
            if (tail->prev->prev->x > tail->prev->x) {
                tail_orientation = ORIENTATION_NORMAL;
            } else if (tail->prev->prev->x < tail->prev->x) {
                tail_orientation = ORIENTATION_MIRROR;
            } else if (tail->prev->prev->y > tail->prev->y) {
                tail_orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
            } else if (tail->prev->prev->y < tail->prev->y) {
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
    if (newx - head->next->x == 2) {
        sprite = body_sprite;
        orientation = ORIENTATION_NORMAL;
    } else if (newx - head->next->x == -2) {
        sprite = body_sprite;
        orientation = ORIENTATION_MIRROR;
    } else if (newy - head->next->y == 2) {
        sprite = body_sprite;
        orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP;
    } else if (newy - head->next->y == -2) {
        sprite = body_sprite;
        orientation = ORIENTATION_ROTATE;
    } else if (newx - head->next->x == 1) {
        sprite = body_curve_sprite;
        if (newy - head->next->y == 1) {
            if (newx - head->x == 1) {
                orientation = ORIENTATION_FLIP;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_FLIP | ORIENTATION_MIRROR;
            }
        } else {
            if (newx - head->x == 1) {
                orientation = ORIENTATION_NORMAL;
            } else {
                orientation = ORIENTATION_ROTATE | ORIENTATION_MIRROR;
            }
        }
    } else if (newx - head->next->x == -1) {
        sprite = body_curve_sprite;
        if (newy - head->next->y == -1) {
            if (newx - head->x == 0) {
                orientation = ORIENTATION_ROTATE;
            } else {
                orientation = ORIENTATION_MIRROR;
            }
        } else {
            if (newx - head->x == 0) {
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

const application_t snake_app = {
    .name = "Snake",
    .start = snake_start,
    .run = snake_run,
    .stop = snake_stop,
};