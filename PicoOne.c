#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "pico_lcd.h"

int main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    pico_lcd_init();
    stdio_init_all();

    uint16_t c = 0x0FF0;
    uint x = 100, y = 100;
    pico_lcd_set_pixel(x, y, c);

    while (true) {
        sleep_ms(10);

        if (pico_lcd_is_pressed(KEY_A)) {
            pico_lcd_clear();
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        if (pico_lcd_is_pressed(KEY_B)) {
            c = 0x1FE3;
        }
        else if (pico_lcd_is_pressed(KEY_X)) {
            c = 0xF800;
        }
        else if (pico_lcd_is_pressed(KEY_Y)) {
            c = 0x30FF;
        }
        else if (pico_lcd_is_pressed(KEY_LEFT)) {
            if (x > 0) x -= 1;
        }
        else if (pico_lcd_is_pressed(KEY_RIGHT)) {
            if (x < LCD_WIDTH-1) x += 1;
        }
        else if (pico_lcd_is_pressed(KEY_UP)) {
            if (y > 0) y -= 1;
        }
        else if (pico_lcd_is_pressed(KEY_DOWN)) {
            if (y < LCD_HEIGHT-1) y += 1;
        }
        else {
            // nothing is pressed
            continue;
        }

        pico_lcd_set_pixel(x, y, c);
        printf("%d %d\n", x, y);
    }
}
