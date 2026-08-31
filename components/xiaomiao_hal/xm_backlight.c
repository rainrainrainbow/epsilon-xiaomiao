#include "xiaomiao_hal.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "xm_backlight";

void xm_backlight_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_1,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = XM_PIN_BACKLIGHT,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    ESP_LOGI(TAG, "Backlight PWM initialized on GPIO%d", XM_PIN_BACKLIGHT);
}

void xm_backlight_set(uint8_t percent) {
    if (percent > 100) percent = 100;
    uint32_t duty = (255 * percent) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}
