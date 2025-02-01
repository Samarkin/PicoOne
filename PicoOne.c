#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/spi.h"
#include "hardware/pwm.h"

#define LCD_BL_PIN 13 // Backlight
#define LCD_RST_PIN 12 // Reset, low active
#define LCD_DIN_PIN 11 // SPI data
#define LCD_CLK_PIN 10 // SPI clock
#define LCD_CS_PIN 9 // Chip select, low active
#define LCD_DC_PIN 8 // Data(high) / Command(low)
#define LCD_SPI_SPEED (10 * 1000 * 1000) // 10 MHz

#define KEY_A_PIN 15
#define KEY_B_PIN 17
#define KEY_X_PIN 19
#define KEY_Y_PIN 21
#define KEY_LEFT_PIN 16
#define KEY_DOWN_PIN 18
#define KEY_RIGHT_PIN 20
#define KEY_UP_PIN 2
#define KEY_PRESSED false

static void pico_lcd_gpio_init_out(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
}

static void pico_lcd_gpio_init_key(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

/*! \brief Initialize LCD
 */
void pico_lcd_init(void) {

}

void pico_lcd_reset(void) {
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(100); // at worst 120ms
    gpio_put(LCD_RST_PIN, 0);
    sleep_ms(100); // at least 10us
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(100); // at worst 120ms
}

static void pico_lcd_send_command(uint8_t cmd) {
    gpio_put(LCD_DC_PIN, 0);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(spi1, &cmd, 1);
    gpio_put(LCD_CS_PIN, 1);
}

static void pico_lcd_send_data(uint8_t value) {
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    spi_write_blocking(spi1, &value, 1);
    gpio_put(LCD_CS_PIN, 1);
}

static void pico_lcd_send_one_byte_command(uint8_t cmd, uint8_t value) {
    pico_lcd_send_command(cmd);
    pico_lcd_send_data(value);
}

static void pico_lcd_clear(void) {
    pico_lcd_send_command(0x2A);
    pico_lcd_send_data(0);
    pico_lcd_send_data(0);
    pico_lcd_send_data(0);
    pico_lcd_send_data(240);

    pico_lcd_send_command(0x2B);
    pico_lcd_send_data(0);
    pico_lcd_send_data(0);
    pico_lcd_send_data(0);
    pico_lcd_send_data(240);

    pico_lcd_send_command(0x2C);
    for (int i = 0; i < 240; i++) {
        for (int j = 0; j < 240; j++) {
            pico_lcd_send_data(0);
            pico_lcd_send_data(0);
        }
    }
}

static void pico_lcd_set_pixel(uint8_t x, uint8_t y, uint16_t color) {
    pico_lcd_send_command(0x2A);
    pico_lcd_send_data(0);
    pico_lcd_send_data(x);
    pico_lcd_send_data(0);
    pico_lcd_send_data(x);

    pico_lcd_send_command(0x2B);
    pico_lcd_send_data(0);
    pico_lcd_send_data(y);
    pico_lcd_send_data(0);
    pico_lcd_send_data(y);

    pico_lcd_send_command(0x2C);
    pico_lcd_send_data(color >> 8);
    pico_lcd_send_data(color & 0xFF);
}

int main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    pico_lcd_init();

    // Backlight PWM
    pico_lcd_gpio_init_out(LCD_BL_PIN);
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_set_wrap(slice_num, 100);
    uint brightness = 0;
    pwm_set_chan_level(slice_num, PWM_CHAN_B, brightness);
    pwm_set_clkdiv(slice_num, 50);
    pwm_set_enabled(slice_num, true);

    // Reset pin
    pico_lcd_gpio_init_out(LCD_RST_PIN);

    // SPI
    pico_lcd_gpio_init_out(LCD_DIN_PIN);
    gpio_set_function(LCD_DIN_PIN, GPIO_FUNC_SPI);
    pico_lcd_gpio_init_out(LCD_CLK_PIN);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    spi_init(spi1, LCD_SPI_SPEED);

    // Chip select pin
    pico_lcd_gpio_init_out(LCD_CS_PIN);
    gpio_put(LCD_CS_PIN, 1);

    // Data/command pin
    pico_lcd_gpio_init_out(LCD_DC_PIN);
    gpio_put(LCD_DC_PIN, 0);

    pico_lcd_reset();
    pico_lcd_send_one_byte_command(0x36, 0x60); // axis order
    pico_lcd_send_one_byte_command(0x3A, 0x05); // color mode
    // voltage
    pico_lcd_send_one_byte_command(0xBB, 0x19);
    pico_lcd_send_one_byte_command(0xC3, 0x12);
    pico_lcd_send_one_byte_command(0xC4, 0x20);
    pico_lcd_send_command(0x21); // inversion
    pico_lcd_send_command(0x11); // sleep out
    pico_lcd_send_command(0x29); // display on

    pico_lcd_clear();

    uint16_t c = 0x0FF0;

    pico_lcd_gpio_init_key(KEY_A_PIN);
    pico_lcd_gpio_init_key(KEY_B_PIN);
    pico_lcd_gpio_init_key(KEY_X_PIN);
    pico_lcd_gpio_init_key(KEY_Y_PIN);
    pico_lcd_gpio_init_key(KEY_LEFT_PIN);
    pico_lcd_gpio_init_key(KEY_DOWN_PIN);
    pico_lcd_gpio_init_key(KEY_UP_PIN);
    pico_lcd_gpio_init_key(KEY_RIGHT_PIN);

    stdio_init_all();

    uint x = 100, y = 100;
    pico_lcd_set_pixel(x, y, c);

    pwm_set_chan_level(slice_num, PWM_CHAN_B, 100);
    while (true) {
        sleep_ms(10);

        if (gpio_get(KEY_A_PIN) == KEY_PRESSED) {
            pico_lcd_clear();
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        if (gpio_get(KEY_B_PIN) == KEY_PRESSED) {
            c = 0x1FE3;
        }
        else if (gpio_get(KEY_X_PIN) == KEY_PRESSED) {
            c = 0xF800;
        }
        else if (gpio_get(KEY_Y_PIN) == KEY_PRESSED) {
            c = 0x30FF;
        }
        else if (gpio_get(KEY_LEFT_PIN) == KEY_PRESSED) {
            if (x > 0) x -= 1;
        }
        else if (gpio_get(KEY_RIGHT_PIN) == KEY_PRESSED) {
            if (x < 239) x += 1;
        }
        else if (gpio_get(KEY_UP_PIN) == KEY_PRESSED) {
            if (y > 0) y -= 1;
        }
        else if (gpio_get(KEY_DOWN_PIN) == KEY_PRESSED) {
            if (y < 239) y += 1;
        }
        else {
            // nothing is pressed
            continue;
        }

        pico_lcd_set_pixel(x, y, c);
        printf("%d %d\n", x, y);
    }
}
