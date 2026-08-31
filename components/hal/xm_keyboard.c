#include "xiaomiao_hal.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "xm_keyboard";

static const int key_pins[XM_KEY_COUNT] = {
    XM_PIN_KEY_UP, XM_PIN_KEY_DOWN, XM_PIN_KEY_LEFT,
    XM_PIN_KEY_RIGHT, XM_PIN_KEY_A, XM_PIN_KEY_B
};

void xm_keyboard_init(void) {
    gpio_config_t io_conf = {0};
    uint64_t pin_mask = 0;
    for (int i = 0; i < XM_KEY_COUNT; i++) {
        pin_mask |= (1ULL << key_pins[i]);
    }
    io_conf.pin_bit_mask = pin_mask;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "Keyboard initialized (6 keys)");
}

uint8_t xm_keyboard_scan(void) {
    uint8_t mask = 0;
    for (int i = 0; i < XM_KEY_COUNT; i++) {
        if (gpio_get_level(key_pins[i]) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}
