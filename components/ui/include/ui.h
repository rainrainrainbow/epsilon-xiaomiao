#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI screens */
typedef enum {
    UI_SCREEN_HOME,
    UI_SCREEN_CALC,
    UI_SCREEN_GRAPH,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_ABOUT,
    UI_SCREEN_COUNT
} ui_screen_t;

/* Initialize UI */
void ui_init(void);

/* Main UI tick - call from main loop */
void ui_tick(void);

/* Get current screen */
ui_screen_t ui_get_screen(void);

/* Switch to screen */
void ui_set_screen(ui_screen_t screen);

#ifdef __cplusplus
}
#endif
