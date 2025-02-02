#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"

#define CELL_SIZE 5

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

static enum _state {
    STATE_RUN,
    STATE_WILL_GAME_OVER,
    STATE_GAME_OVER,
} state;

#define COLOR_SNAKE COLOR_GREEN
#define COLOR_APPLE COLOR_RED

#define FIELD_WIDTH (LCD_WIDTH/CELL_SIZE)
static_assert(LCD_WIDTH%CELL_SIZE == 0);

#define FIELD_HEIGHT (LCD_HEIGHT/CELL_SIZE)
static_assert(LCD_HEIGHT%CELL_SIZE == 0);

static void snake_fill_cell(uint8_t x, uint8_t y, uint16_t color) {
    pico_lcd_fill_rect(x*CELL_SIZE, (x+1)*CELL_SIZE - 1, y*CELL_SIZE, (y+1)*CELL_SIZE - 1, color);
}

static void snake_start(void) {
    pico_lcd_clear();

    section_t *s1 = malloc(sizeof(section_t));
    s1->x = FIELD_WIDTH/2;
    s1->y = FIELD_HEIGHT/2;
    snake_fill_cell(s1->x, s1->y, COLOR_SNAKE);

    section_t *s2 = malloc(sizeof(section_t));
    s2->x = FIELD_WIDTH/2 - 1;
    s2->y = FIELD_HEIGHT/2;
    snake_fill_cell(s2->x, s2->y, COLOR_SNAKE);

    section_t *s3 = malloc(sizeof(section_t));
    s3->x = FIELD_WIDTH/2 - 2;
    s3->y = FIELD_HEIGHT/2;
    snake_fill_cell(s3->x, s3->y, COLOR_SNAKE);

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

    applex = 10;
    appley = 10;
    snake_fill_cell(applex, appley, COLOR_APPLE);

    printf("Snake starts with %d sections\n", length);
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
        pico_lcd_fill_rect(LCD_WIDTH/4, 3*LCD_WIDTH/4, LCD_HEIGHT/4, 3*LCD_HEIGHT/4, COLOR_RED);
        printf("Game over with score %d\n", length);
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

    if (pico_lcd_is_pressed(KEY_LEFT)) {
        dirx = -1;
        diry = 0;
    } else if (pico_lcd_is_pressed(KEY_RIGHT)) {
        dirx = 1;
        diry = 0;
    } else if (pico_lcd_is_pressed(KEY_UP)) {
        dirx = 0;
        diry = -1;
    } else if (pico_lcd_is_pressed(KEY_DOWN)) {
        dirx = 0;
        diry = 1;
    }

    // determine new head position
    int newx = head->x + dirx;
    int newy = head->y + diry;
    if (newx < 0 || newx >= FIELD_WIDTH || newy < 0 || newy >= FIELD_HEIGHT || snake_is_cell_occupied(newx, newy)) {
        state = STATE_WILL_GAME_OVER;
        return;
    }

    section_t* new_head;
    if (newx == applex && newy == appley) {
        // grow new section
        new_head = malloc(sizeof(section_t));
        // TODO: Randomize
        if (applex == 10) {
            applex = 15;
            appley = 15;
        } else if (applex == 15) {
            applex = 20;
            appley = 20;
        }
        snake_fill_cell(applex, appley, COLOR_APPLE);
        length += 1;
        printf("Snake has %d sections\n", length);
    } else {
        // repurpose tail
        snake_fill_cell(tail->x, tail->y, COLOR_BLACK);
        new_head = tail;
        tail = tail->prev;
        tail->next = NULL;
    }

    // place new head
    new_head->x = newx;
    new_head->y = newy;
    new_head->next = head;
    new_head->prev = NULL;
    head->prev = new_head;
    head = new_head;
    snake_fill_cell(head->x, head->y, COLOR_SNAKE);
}

application_t snake_app = {
    .name = "Snake",
    .start = snake_start,
    .run = snake_run,
    .stop = snake_stop,
};