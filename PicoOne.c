#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"
#include "pico_ui.h"

#include "snake.h"
#include "dump.h"

static const application_t* const APPLICATIONS[] = {
    &snake_app,
    &dump_app,
};
static const int APPLICATION_COUNT = sizeof(APPLICATIONS) / sizeof(const application_t*);
static int pico_menu_selection;
static bool app_is_running;

void pico_application_stop(void) {
    app_is_running = false;
}

static void pico_draw_menu(void) {
    for (int i = 0; i < APPLICATION_COUNT; i++) {
        char s[4];
        int len = snprintf(s, sizeof(s), "%d.", i+1);
        int y = i * Font20.Height;
        pico_ui_draw_string(s, 0, y, &Font20, COLOR_WHITE, COLOR_BLACK);
        pico_ui_draw_string(APPLICATIONS[i]->name, (len+1)*Font20.Width, y, &Font20, COLOR_WHITE, COLOR_BLACK);
    }
}

static void pico_change_selection(int new_selection) {
    pico_ui_draw_rect(0, LCD_WIDTH-1, pico_menu_selection*Font20.Height, (pico_menu_selection+1)*Font20.Height-1, COLOR_BLACK);
    pico_menu_selection = new_selection;
    pico_ui_draw_rect(0, LCD_WIDTH-1, pico_menu_selection*Font20.Height, (pico_menu_selection+1)*Font20.Height-1, COLOR_WHITE);
}

int main() {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    pico_lcd_init();
    stdio_init_all();

    while (true) {
        pico_lcd_clear();
        pico_draw_menu();
        pico_change_selection(0);

        int key_pressed = -1;
        while (key_pressed != KEY_A) {
            if (pico_lcd_is_pressed(KEY_DOWN)) {
                if (key_pressed != KEY_DOWN) {
                    key_pressed = KEY_DOWN;
                    if (pico_menu_selection < APPLICATION_COUNT - 1) {
                        pico_change_selection(pico_menu_selection + 1);
                    }
                }
            } else if (pico_lcd_is_pressed(KEY_UP)) {
                if (key_pressed != KEY_UP) {
                    key_pressed = KEY_UP;
                    if (pico_menu_selection > 0) {
                        pico_change_selection(pico_menu_selection - 1);
                    }
                }
            } else if (pico_lcd_is_pressed(KEY_A)) {
                key_pressed = KEY_A;
            } else {
                key_pressed = -1;
            }
        }

        const application_t* current_app = APPLICATIONS[pico_menu_selection];
        printf("Starting application %s\n", current_app->name);
        app_is_running = true;
        current_app->start();
        while (app_is_running) {
            current_app->run();
        }
        current_app->stop();
    }
}
