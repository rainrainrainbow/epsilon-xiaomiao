#include <stdio.h>
#include "xiaomiao_hal.h"
#include "mathcore.h"
#include "ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "main";

void app_main(void) {
    ESP_LOGI(TAG, "Epsilon for XiaoMiao starting...");

    /* Initialize all HAL components */
    xm_display_init();
    xm_backlight_init();
    xm_keyboard_init();
    xm_led_init();
    xm_battery_init();

    /* Set backlight to 80% */
    xm_backlight_set(80);

    /* Initialize math engine */
    mc_init();

    /* Initialize UI */
    ui_init();

    /* Startup LED indicator */
    xm_led_set(0, 0, 0, 64);   /* Blue */
    xm_led_set(1, 0, 64, 0);   /* Green */
    xm_led_set(2, 64, 0, 0);   /* Red */
    xm_led_refresh();

    ESP_LOGI(TAG, "Initialization complete. Entering main loop.");

    /* Main loop: ~30 FPS */
    while (1) {
        ui_tick();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}
