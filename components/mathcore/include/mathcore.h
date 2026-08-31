#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum expression length */
#define MC_MAX_EXPR_LEN 256
/* Maximum variables */
#define MC_MAX_VARS 26

/* Token types */
typedef enum {
    TK_NUM, TK_PLUS, TK_MINUS, TK_MUL, TK_DIV,
    TK_LPAREN, TK_RPAREN, TK_POWER,
    TK_VAR, TK_FUNC, TK_COMMA, TK_END
} mc_token_type_t;

/* Variable storage */
typedef struct {
    char name;
    double value;
    bool used;
} mc_var_t;

/* Initialize math engine */
void mc_init(void);

/* Evaluate expression string, result in *out. Returns 0 on success. */
int mc_eval(const char* expr, double* out);

/* Set variable (a-z) */
void mc_set_var(char name, double value);

/* Get variable */
double mc_get_var(char name);

/* Clear all variables */
void mc_clear_vars(void);

#ifdef __cplusplus
}
#endif
