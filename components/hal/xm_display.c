#include "xiaomiao_hal.h"
#include <string.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "xm_display";

#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_VMCTR1  0xC5
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_DISPON  0x29

static spi_device_handle_t s_spi;

static inline void dc_set(bool level) { gpio_set_level(XM_PIN_LCD_DC, level); }

static void spi_write(const uint8_t* data, size_t len) {
    if (len == 0) return;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = len * 8;
    t.tx_buffer = data;
    spi_device_transmit(s_spi, &t);
}

static void write_cmd(uint8_t cmd) {
    dc_set(0);
    spi_write(&cmd, 1);
}

static void write_data(const uint8_t* data, size_t len) {
    dc_set(1);
    spi_write(data, len);
}

static void write_data_u8(uint8_t d) { write_data(&d, 1); }

void xm_display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    write_cmd(ST7735_CASET);
    uint8_t d[4] = {(uint8_t)(x0 >> 8), (uint8_t)x0, (uint8_t)(x1 >> 8), (uint8_t)x1};
    write_data(d, 4);
    write_cmd(ST7735_RASET);
    d[0] = (uint8_t)(y0 >> 8); d[1] = (uint8_t)y0;
    d[2] = (uint8_t)(y1 >> 8); d[3] = (uint8_t)y1;
    write_data(d, 4);
}

void xm_display_fill(uint16_t color) {
    xm_display_set_window(0, 0, XM_DISPLAY_WIDTH - 1, XM_DISPLAY_HEIGHT - 1);
    write_cmd(ST7735_RAMWR);
    dc_set(1);
    size_t total = (size_t)XM_DISPLAY_WIDTH * XM_DISPLAY_HEIGHT;
    static uint8_t buf[320];
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);
    for (size_t i = 0; i < sizeof(buf); i += 2) { buf[i] = hi; buf[i+1] = lo; }
    size_t written = 0;
    size_t bytes_total = total * 2;
    while (written < bytes_total) {
        size_t chunk = (bytes_total - written > sizeof(buf)) ? sizeof(buf) : (bytes_total - written);
        spi_write(buf, chunk);
        written += chunk;
    }
}

void xm_display_push_pixels(const uint16_t* pixels, uint32_t len) {
    if (len == 0) return;
    static uint8_t buf[1280];
    uint32_t i = 0;
    while (i < len) {
        uint32_t chunk = len - i;
        if (chunk > sizeof(buf) / 2) chunk = sizeof(buf) / 2;
        for (uint32_t j = 0; j < chunk; j++) {
            buf[j*2]     = (uint8_t)(pixels[i+j] >> 8);
            buf[j*2 + 1] = (uint8_t)(pixels[i+j] & 0xFF);
        }
        spi_write(buf, chunk * 2);
        i += chunk;
    }
}

/* Scale logical 320x240 -> physical 160x128 via box-averaging */
void xm_display_render_scaled(const uint16_t* fb) {
    static uint16_t out[XM_DISPLAY_WIDTH];
    uint32_t px, py;
    for (py = 0; py < XM_DISPLAY_HEIGHT; py++) {
        uint32_t sy0 = py * XM_DISPLAY_LOGICAL_HEIGHT / XM_DISPLAY_HEIGHT;
        uint32_t sy1 = (py + 1) * XM_DISPLAY_LOGICAL_HEIGHT / XM_DISPLAY_HEIGHT;
        for (px = 0; px < XM_DISPLAY_WIDTH; px++) {
            uint32_t sx0 = px * XM_DISPLAY_LOGICAL_WIDTH / XM_DISPLAY_WIDTH;
            uint32_t sx1 = (px + 1) * XM_DISPLAY_LOGICAL_WIDTH / XM_DISPLAY_WIDTH;
            uint32_t rsum = 0, gsum = 0, bsum = 0, cnt = 0;
            uint32_t sy, sx;
            for (sy = sy0; sy < sy1; sy++) {
                for (sx = sx0; sx < sx1; sx++) {
                    uint16_t c = fb[sy * XM_DISPLAY_LOGICAL_WIDTH + sx];
                    rsum += (c >> 11) & 0x1F;
                    gsum += (c >> 5) & 0x3F;
                    bsum += c & 0x1F;
                    cnt++;
                }
            }
            if (cnt > 0) {
                out[px] = (uint16_t)(((rsum / cnt) << 11) | ((gsum / cnt) << 5) | (bsum / cnt));
            } else {
                out[px] = 0;
            }
        }
        xm_display_set_window(0, (uint16_t)py, XM_DISPLAY_WIDTH - 1, (uint16_t)py);
        write_cmd(ST7735_RAMWR);
        dc_set(1);
        spi_write((const uint8_t*)out, XM_DISPLAY_WIDTH * 2);
    }
}

static void st7735_init_sequence(void) {
    write_cmd(ST7735_SWRESET); vTaskDelay(pdMS_TO_TICKS(150));
    write_cmd(ST7735_SLPOUT);  vTaskDelay(pdMS_TO_TICKS(500));
    write_cmd(ST7735_FRMCTR1); write_data_u8(0x01); write_data_u8(0x2C); write_data_u8(0x2D);
    write_cmd(ST7735_FRMCTR2); write_data_u8(0x01); write_data_u8(0x2C); write_data_u8(0x2D);
    write_cmd(ST7735_FRMCTR3); write_data_u8(0x01); write_data_u8(0x2C); write_data_u8(0x2D);
                                   write_data_u8(0x01); write_data_u8(0x2C); write_data_u8(0x2D);
    write_cmd(ST7735_INVCTR);  write_data_u8(0x07);
    write_cmd(ST7735_PWCTR1);  write_data_u8(0xA2); write_data_u8(0x02); write_data_u8(0x84);
    write_cmd(ST7735_PWCTR2);  write_data_u8(0xC5);
    write_cmd(ST7735_PWCTR3);  write_data_u8(0x0A); write_data_u8(0x00);
    write_cmd(ST7735_VMCTR1);  write_data_u8(0x8E); write_data_u8(0x00);
    write_cmd(ST7735_MADCTL);  write_data_u8(0x00);
    write_cmd(ST7735_COLMOD);  write_data_u8(0x05);
    write_cmd(ST7735_DISPON);  vTaskDelay(pdMS_TO_TICKS(100));
}

void xm_display_init(void) {
    gpio_config_t io_conf = {0};
    io_conf.pin_bit_mask = (1ULL << XM_PIN_LCD_DC) | (1ULL << XM_PIN_LCD_CS);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    cs_set(1);

    spi_bus_config_t bus_cfg = {0};
    bus_cfg.mosi_io_num = XM_PIN_LCD_MOSI;
    bus_cfg.miso_io_num = XM_PIN_LCD_MISO;
    bus_cfg.sclk_io_num = XM_PIN_LCD_SCLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.max_transfer_sz = 4096;
    spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t dev_cfg = {0};
    dev_cfg.clock_speed_hz = 40 * 1000 * 1000;
    dev_cfg.mode = 0;
    dev_cfg.spics_io_num = -1;
    dev_cfg.queue_size = 7;
    dev_cfg.command_bits = 0;
    dev_cfg.address_bits = 0;
    dev_cfg.dummy_bits = 0;
    spi_bus_add_device(SPI2_HOST, &dev_cfg, &s_spi);

    st7735_init_sequence();
    xm_display_fill(0x0000);
    ESP_LOGI(TAG, "ST7735 display initialized (%dx%d -> %dx%d)",
             XM_DISPLAY_LOGICAL_WIDTH, XM_DISPLAY_LOGICAL_HEIGHT,
             XM_DISPLAY_WIDTH, XM_DISPLAY_HEIGHT);
}
