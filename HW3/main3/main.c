#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_timer.h"

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define INIT_X 1
#define INIT_Y 1
#define INIT_VX -1
#define INIT_VY 1
#define END_X 6
#define END_Y 4
#define FRAME_RATE 5  // frames per second

int x = INIT_X;
int y = INIT_Y;
int x_direction = INIT_VX;
int y_direction = INIT_VY;
int frame_count = 0;

uint8_t framebuffer[8] = {};
spi_device_handle_t spi;
esp_timer_handle_t timer_led;

void max7219_send(uint8_t address, uint8_t data) {
    uint8_t tx_buf[2] = {address, data};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_buf
    };
    spi_device_transmit(spi, &t);  // Blocking transmit
}

void max7219_init() {
    max7219_send(0x0F, 0x00);  // Display test off
    max7219_send(0x0C, 0x01);  // Shutdown mode off
    max7219_send(0x0B, 0x07);  // Scan all digits
    max7219_send(0x0A, 0x08);  // Intensity
    max7219_send(0x09, 0x00);  // No decode mode
    for (int i = 1; i <= 8; i++) {
        max7219_send(i, 0x00);
    }
}

void clear() {
    for (int i = 0; i < 8; i++) {
        framebuffer[i] = 0x00;
        max7219_send(i + 1, framebuffer[i]);
    }
}

void set_led(uint8_t row, uint8_t col) {
    framebuffer[row] |= (1 << (7 - col));
    max7219_send(row + 1, framebuffer[row]);
}

void on_timer(void* args) {
    if (x + x_direction <= 0 || x + x_direction > 7) {
        x_direction *= -1;
        frame_count++;
    }
    if (y + y_direction < 0 || y + y_direction > 7) {
        y_direction *= -1;
        frame_count++;
    }
    x += x_direction;
    y += y_direction;

    if (x == END_X && y == END_Y) {
        esp_timer_stop(timer_led);
        clear();
        for (int i = 0; i < 8; i++) set_led(0, i);
        set_led(y, x);
        printf("%d\n", frame_count);
        return;
    }

    clear();
    for (int i = 0; i < 8; i++) set_led(0, i);
    set_led(y, x);
}

void start_timer() {
    const esp_timer_create_args_t timer_args = {
        .callback = &on_timer,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "frame_timer"
    };
    esp_timer_create(&timer_args, &timer_led);
    esp_timer_start_periodic(timer_led, 1000000 / FRAME_RATE);
}

void spi_setup() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
}

void app_main(void) {
    spi_setup();
    max7219_init();
    clear();
    start_timer();
}
