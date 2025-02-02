#include <stdio.h>
#include "pico/stdlib.h"
#include "pico_lcd.h"

#include "snake.h"

int main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    pico_lcd_init();
    stdio_init_all();

    application_t current_app = snake_app;
    printf("Starting application %s\n", current_app.name);
    current_app.start();

    while (true) {
        current_app.run();
    }
}
