#include "pico_lcd.h"

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

static void pico_lcd_reset(void) {
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(120);
    gpio_put(LCD_RST_PIN, 0);
    sleep_us(10);
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(120);
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

void pico_lcd_clear(void) {
    pico_lcd_fill_rect(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1, COLOR_BLACK);
}

void pico_lcd_init(void) {
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

    pwm_set_chan_level(slice_num, PWM_CHAN_B, 100);

    pico_lcd_gpio_init_key(KEY_A);
    pico_lcd_gpio_init_key(KEY_B);
    pico_lcd_gpio_init_key(KEY_X);
    pico_lcd_gpio_init_key(KEY_Y);
    pico_lcd_gpio_init_key(KEY_LEFT);
    pico_lcd_gpio_init_key(KEY_DOWN);
    pico_lcd_gpio_init_key(KEY_UP);
    pico_lcd_gpio_init_key(KEY_RIGHT);
}

void pico_lcd_fill_rect(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint16_t color) {
    pico_lcd_send_command(0x2A);
    pico_lcd_send_data(0);
    pico_lcd_send_data(x1);
    pico_lcd_send_data(0);
    pico_lcd_send_data(x2);

    pico_lcd_send_command(0x2B);
    pico_lcd_send_data(0);
    pico_lcd_send_data(y1);
    pico_lcd_send_data(0);
    pico_lcd_send_data(y2);

    pico_lcd_send_command(0x2C);
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_CS_PIN, 0);
    uint8_t c[2] = {color >> 8, color & 0xFF};
    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
            spi_write_blocking(spi1, c, 2);
        }
    }
    gpio_put(LCD_CS_PIN, 1);
}

void pico_lcd_set_pixel(uint8_t x, uint8_t y, uint16_t color) {
    pico_lcd_fill_rect(x, x, y, y, color);
}

bool pico_lcd_is_pressed(uint key) {
    return gpio_get(key) == KEY_PRESSED;
}