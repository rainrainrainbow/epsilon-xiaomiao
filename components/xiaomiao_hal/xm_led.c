#include "xiaomiao_hal.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG = "xm_led";
static rmt_channel_handle_t s_led_channel = NULL;
static uint8_t s_led_buf[XM_LED_COUNT * 3];

#define T0H  (9)
#define T0L  (31)
#define T1H  (27)
#define T1L  (13)

static rmt_symbol_word_t ws2812_bits[24 * XM_LED_COUNT];

static void encode_byte(uint8_t byte, rmt_symbol_word_t* bits) {
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
            bits[7 - i].level0 = 1; bits[7 - i].duration0 = T1H;
            bits[7 - i].level1 = 0; bits[7 - i].duration1 = T1L;
        } else {
            bits[7 - i].level0 = 1; bits[7 - i].duration0 = T0H;
            bits[7 - i].level1 = 0; bits[7 - i].duration1 = T0L;
        }
    }
}

void xm_led_init(void) {
    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = XM_PIN_LED,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 80 * 1000 * 1000,
        .mem_block_symbols = 48,
        .trans_queue_depth = 4,
        .flags.with_dma = false,
    };
    rmt_new_tx_channel(&tx_cfg, &s_led_channel);
    rmt_enable(s_led_channel);
    memset(s_led_buf, 0, sizeof(s_led_buf));
    ESP_LOGI(TAG, "WS2812B LED initialized (3 LEDs on GPIO%d)", XM_PIN_LED);
}

void xm_led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= XM_LED_COUNT) return;
    s_led_buf[index * 3 + 0] = g;
    s_led_buf[index * 3 + 1] = r;
    s_led_buf[index * 3 + 2] = b;
}

void xm_led_clear(void) {
    memset(s_led_buf, 0, sizeof(s_led_buf));
}

void xm_led_refresh(void) {
    if (!s_led_channel) return;
    for (int i = 0; i < XM_LED_COUNT; i++) {
        encode_byte(s_led_buf[i * 3 + 0], &ws2812_bits[i * 24 + 0]);
        encode_byte(s_led_buf[i * 3 + 1], &ws2812_bits[i * 24 + 8]);
        encode_byte(s_led_buf[i * 3 + 2], &ws2812_bits[i * 24 + 16]);
    }
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
        .flags.eot_level = 0,
    };
    rmt_transmit(s_led_channel, ws2812_bits, sizeof(ws2812_bits), &tx_config);
    rmt_tx_wait_all_done(s_led_channel, 100);
}
