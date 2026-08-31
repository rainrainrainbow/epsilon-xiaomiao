#include "ui.h"
#include "xiaomiao_hal.h"
#include "mathcore.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Logical framebuffer 320x240 RGB565 */
static uint16_t s_fb[XM_DISPLAY_LOGICAL_WIDTH * XM_DISPLAY_LOGICAL_HEIGHT];
static ui_screen_t s_current_screen = UI_SCREEN_HOME;
static int s_menu_index = 0;
static char s_input_buf[64];
static int s_input_len = 0;
static double s_result = 0.0;
static bool s_has_result = false;

/* Color definitions (RGB565) */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_BLUE    0x001F
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_GRAY    0x7BEF
#define COLOR_DKBLUE  0x000A

static void fb_clear(uint16_t color) {
    for (int i = 0; i < XM_DISPLAY_LOGICAL_WIDTH * XM_DISPLAY_LOGICAL_HEIGHT; i++) {
        s_fb[i] = color;
    }
}

static void fb_draw_char(int cx, int cy, char ch, uint16_t color) {
    /* Simple 5x7 bitmap font - minimal set */
    /* For now, draw a small rectangle as placeholder */
    int x0 = cx * 6;
    int y0 = cy * 8;
    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            int px = x0 + dx;
            int py = y0 + dy;
            if (px >= 0 && px < XM_DISPLAY_LOGICAL_WIDTH &&
                py >= 0 && py < XM_DISPLAY_LOGICAL_HEIGHT) {
                s_fb[py * XM_DISPLAY_LOGICAL_WIDTH + px] = color;
            }
        }
    }
}

static void fb_draw_rect(int x0, int y0, int w, int h, uint16_t color) {
    for (int y = y0; y < y0 + h && y < XM_DISPLAY_LOGICAL_HEIGHT; y++) {
        for (int x = x0; x < x0 + w && x < XM_DISPLAY_LOGICAL_WIDTH; x++) {
            if (x >= 0 && y >= 0) {
                s_fb[y * XM_DISPLAY_LOGICAL_WIDTH + x] = color;
            }
        }
    }
}

static void fb_draw_text_simple(int x, int y, const char* text, uint16_t color) {
    /* Draw each character as a colored block (placeholder for real font) */
    int cx = x;
    while (*text) {
        fb_draw_char(cx, y, *text, color);
        cx++;
        text++;
    }
}

/* Home screen: menu with Calc, Graph, Settings, About */
static void render_home(void) {
    fb_clear(COLOR_BLACK);
    /* Title bar */
    fb_draw_rect(0, 0, 320, 20, COLOR_DKBLUE);
    fb_draw_text_simple(2, 1, "Epsilon Calculator", COLOR_WHITE);

    /* Menu items */
    const char* items[] = {"Calculate", "Graph", "Settings", "About"};
    int n = 4;
    for (int i = 0; i < n; i++) {
        int yy = 30 + i * 25;
        uint16_t bg = (i == s_menu_index) ? COLOR_BLUE : COLOR_BLACK;
        uint16_t fg = (i == s_menu_index) ? COLOR_WHITE : COLOR_GRAY;
        fb_draw_rect(10, yy, 300, 20, bg);
        fb_draw_text_simple(3, yy + 1, items[i], fg);
    }

    /* Footer */
    fb_draw_text_simple(2, 28, "A=Select B=Back", COLOR_CYAN);
}

/* Calc screen: simple expression evaluator */
static void render_calc(void) {
    fb_clear(COLOR_BLACK);
    fb_draw_rect(0, 0, 320, 20, COLOR_DKBLUE);
    fb_draw_text_simple(2, 1, "Calculator", COLOR_WHITE);

    /* Input line */
    fb_draw_text_simple(2, 5, s_input_buf, COLOR_WHITE);

    /* Result */
    if (s_has_result) {
        char buf[32];
        snprintf(buf, sizeof(buf), "= %.6g", s_result);
        fb_draw_text_simple(2, 8, buf, COLOR_GREEN);
    }

    fb_draw_text_simple(2, 28, "A=Eval B=Back", COLOR_CYAN);
}

/* Settings screen */
static void render_settings(void) {
    fb_clear(COLOR_BLACK);
    fb_draw_rect(0, 0, 320, 20, COLOR_DKBLUE);
    fb_draw_text_simple(2, 1, "Settings", COLOR_WHITE);

    float v = xm_battery_voltage();
    char buf[48];
    snprintf(buf, sizeof(buf), "Battery: %.2f V", v);
    fb_draw_text_simple(2, 5, buf, COLOR_WHITE);
    fb_draw_text_simple(2, 8, "Backlight: 80%", COLOR_WHITE);
    fb_draw_text_simple(2, 11, "Screen: 160x128", COLOR_WHITE);
    fb_draw_text_simple(2, 14, "CPU: 240 MHz", COLOR_WHITE);

    fb_draw_text_simple(2, 28, "B=Back", COLOR_CYAN);
}

/* About screen */
static void render_about(void) {
    fb_clear(COLOR_BLACK);
    fb_draw_rect(0, 0, 320, 20, COLOR_DKBLUE);
    fb_draw_text_simple(2, 1, "About", COLOR_WHITE);

    fb_draw_text_simple(2, 5, "Epsilon for XiaoMiao", COLOR_WHITE);
    fb_draw_text_simple(2, 8, "v0.1.0", COLOR_YELLOW);
    fb_draw_text_simple(2, 11, "Based on NumWorks", COLOR_GRAY);
    fb_draw_text_simple(2, 14, "ESP32-WROVER-B", COLOR_GRAY);

    fb_draw_text_simple(2, 28, "B=Back", COLOR_CYAN);
}

/* Graph screen placeholder */
static void render_graph(void) {
    fb_clear(COLOR_BLACK);
    fb_draw_rect(0, 0, 320, 20, COLOR_DKBLUE);
    fb_draw_text_simple(2, 1, "Graph", COLOR_WHITE);

    /* Draw axes */
    fb_draw_rect(160, 20, 1, 200, COLOR_GRAY);
    fb_draw_rect(0, 120, 320, 1, COLOR_GRAY);

    /* Draw a simple sine wave */
    for (int x = 0; x < 320; x++) {
        double angle = (x - 160) * 3.14159 / 80.0;
        double y = sin(angle) * 60.0;
        int py = 120 - (int)y;
        if (py >= 20 && py < 240) {
            s_fb[py * XM_DISPLAY_LOGICAL_WIDTH + x] = COLOR_GREEN;
        }
    }

    fb_draw_text_simple(2, 28, "B=Back", COLOR_CYAN);
}

void ui_init(void) {
    fb_clear(COLOR_BLACK);
    s_current_screen = UI_SCREEN_HOME;
    s_menu_index = 0;
    s_input_len = 0;
    s_input_buf[0] = 0;
    s_has_result = false;
}

void ui_set_screen(ui_screen_t screen) {
    s_current_screen = screen;
}

ui_screen_t ui_get_screen(void) {
    return s_current_screen;
}

void ui_tick(void) {
    uint8_t keys = xm_keyboard_scan();

    switch (s_current_screen) {
    case UI_SCREEN_HOME:
        if (keys & (1 << XM_KEY_UP)) {
            if (s_menu_index > 0) s_menu_index--;
        }
        if (keys & (1 << XM_KEY_DOWN)) {
            if (s_menu_index < 3) s_menu_index++;
        }
        if (keys & (1 << XM_KEY_A)) {
            switch (s_menu_index) {
                case 0: ui_set_screen(UI_SCREEN_CALC); break;
                case 1: ui_set_screen(UI_SCREEN_GRAPH); break;
                case 2: ui_set_screen(UI_SCREEN_SETTINGS); break;
                case 3: ui_set_screen(UI_SCREEN_ABOUT); break;
            }
        }
        render_home();
        break;

    case UI_SCREEN_CALC:
        if (keys & (1 << XM_KEY_B)) {
            ui_set_screen(UI_SCREEN_HOME);
            s_input_len = 0;
            s_input_buf[0] = 0;
        }
        if (keys & (1 << XM_KEY_A)) {
            if (s_input_len > 0) {
                mc_eval(s_input_buf, &s_result);
                s_has_result = true;
            }
        }
        render_calc();
        break;

    case UI_SCREEN_GRAPH:
        if (keys & (1 << XM_KEY_B)) {
            ui_set_screen(UI_SCREEN_HOME);
        }
        render_graph();
        break;

    case UI_SCREEN_SETTINGS:
        if (keys & (1 << XM_KEY_B)) {
            ui_set_screen(UI_SCREEN_HOME);
        }
        render_settings();
        break;

    case UI_SCREEN_ABOUT:
        if (keys & (1 << XM_KEY_B)) {
            ui_set_screen(UI_SCREEN_HOME);
        }
        render_about();
        break;

    default:
        ui_set_screen(UI_SCREEN_HOME);
        break;
    }

    /* Render framebuffer to display */
    xm_display_render_scaled(s_fb);
}
