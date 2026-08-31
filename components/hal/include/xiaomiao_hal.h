#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===================== Display (ST7735) ===================== */
#define XM_DISPLAY_WIDTH   160   /* physical ST7735 width */
#define XM_DISPLAY_HEIGHT  128   /* physical ST7735 height */
#define XM_DISPLAY_LOGICAL_WIDTH  320   /* logical rendering width */
#define XM_DISPLAY_LOGICAL_HEIGHT 240   /* logical rendering height */

/* ST7735 SPI pins (XiaoMiao hardware spec) */
#define XM_PIN_LCD_SCLK  GPIO_NUM_18
#define XM_PIN_LCD_MOSI  GPIO_NUM_23
#define XM_PIN_LCD_MISO  GPIO_NUM_19
#define XM_PIN_LCD_CS    GPIO_NUM_5
#define XM_PIN_LCD_DC    GPIO_NUM_4

/* Backlight PWM pin (⚠ startup-mode select pin GPIO0) */
#define XM_PIN_BACKLIGHT GPIO_NUM_0

/* ===================== Keyboard (6-key) ===================== */
#define XM_KEY_UP    0
#define XM_KEY_DOWN  1
#define XM_KEY_LEFT  2
#define XM_KEY_RIGHT 3
#define XM_KEY_A     4   /* OK / confirm */
#define XM_KEY_B     5   /* Back / cancel */
#define XM_KEY_COUNT 6

/* Keyboard GPIO pins */
#define XM_PIN_KEY_UP    GPIO_NUM_2
#define XM_PIN_KEY_DOWN  GPIO_NUM_12
#define XM_PIN_KEY_LEFT  GPIO_NUM_13
#define XM_PIN_KEY_RIGHT GPIO_NUM_27
#define XM_PIN_KEY_A     GPIO_NUM_34   /* input-only */
#define XM_PIN_KEY_B     GPIO_NUM_35   /* input-only */

/* ===================== LED (WS2812B) ===================== */
#define XM_PIN_LED       GPIO_NUM_14
#define XM_LED_COUNT     3

/* ===================== Battery ===================== */
/* ADC1 Channel 6 = GPIO34 (shared with key A) */
#define XM_BATTERY_ADC_CHANNEL ADC1_CHANNEL_6

/* ===================== Public API ===================== */

/* Display */
void xm_display_init(void);
void xm_display_fill(uint16_t color);
void xm_display_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void xm_display_push_pixels(const uint16_t* pixels, uint32_t len);
/* Draw a full scaled frame: internal logical buffer -> physical screen */
void xm_display_render_scaled(const uint16_t* logical_fb);

/* Backlight (0-100) */
void xm_backlight_init(void);
void xm_backlight_set(uint8_t percent);

/* Keyboard */
void xm_keyboard_init(void);
/* Returns bitmask of currently-pressed keys (bits 0..5) */
uint8_t xm_keyboard_scan(void);

/* LED */
void xm_led_init(void);
void xm_led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void xm_led_clear(void);
void xm_led_refresh(void);

/* Battery voltage in volts (e.g. 3.7f) */
float xm_battery_voltage(void);

/* Timing helpers (blocking ms / us) */
void xm_delay_ms(uint32_t ms);
uint32_t xm_millis(void);

#ifdef __cplusplus
}
#endif