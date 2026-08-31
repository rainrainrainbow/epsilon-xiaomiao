#include "mathcore.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

static mc_var_t s_vars[MC_MAX_VARS];

void mc_init(void) {
    memset(s_vars, 0, sizeof(s_vars));
}

void mc_set_var(char name, double value) {
    int idx = -1;
    for (int i = 0; i < MC_MAX_VARS; i++) {
        if (s_vars[i].used && s_vars[i].name == name) {
            s_vars[i].value = value;
            return;
        }
        if (!s_vars[i].used && idx < 0) idx = i;
    }
    if (idx >= 0) {
        s_vars[idx].name = name;
        s_vars[idx].value = value;
        s_vars[idx].used = true;
    }
}

double mc_get_var(char name) {
    for (int i = 0; i < MC_MAX_VARS; i++) {
        if (s_vars[i].used && s_vars[i].name == name) return s_vars[i].value;
    }
    return 0.0;
}

void mc_clear_vars(void) {
    memset(s_vars, 0, sizeof(s_vars));
}

/* Tokenizer */
typedef struct {
    mc_token_type_t type;
    double num_val;
    char name;
} token_t;

static const char* s_pos;

static token_t next_token(void) {
    token_t t = {TK_END, 0, 0};
    while (*s_pos == ' ' || *s_pos == '\t') s_pos++;
    if (*s_pos == 0) return t;
    if (isdigit(*s_pos) || *s_pos == '.') {
        char* end;
        t.type = TK_NUM;
        t.num_val = strtod(s_pos, &end);
        s_pos = end;
        return t;
    }
    if (isalpha(*s_pos)) {
        t.name = *s_pos;
        s_pos++;
        /* Check if function name */
        if (*s_pos == '(') {
            t.type = TK_FUNC;
        } else {
            t.type = TK_VAR;
        }
        return t;
    }
    switch (*s_pos) {
        case '+': s_pos++; t.type = TK_PLUS; return t;
        case '-': s_pos++; t.type = TK_MINUS; return t;
        case '*':
            s_pos++;
            if (*s_pos == '*') { s_pos++; t.type = TK_POWER; }
            else t.type = TK_MUL;
            return t;
        case '/': s_pos++; t.type = TK_DIV; return t;
        case '(': s_pos++; t.type = TK_LPAREN; return t;
        case ')': s_pos++; t.type = TK_RPAREN; return t;
        case ',': s_pos++; t.type = TK_COMMA; return t;
        default: s_pos++; return t;
    }
}

/* Recursive descent parser */
static double parse_expr(void);
static double parse_term(void);
static double parse_factor(void);
static double parse_unary(void);

static double parse_primary(void) {
    token_t t = next_token();
    if (t.type == TK_NUM) return t.num_val;
    if (t.type == TK_VAR) return mc_get_var(t.name);
    if (t.type == TK_FUNC) {
        /* function call: skip '(' */
        s_pos++; /* skip '(' */
        double arg = parse_expr();
        /* skip ')' */
        while (*s_pos == ' ') s_pos++;
        if (*s_pos == ')') s_pos++;
        /* Built-in functions */
        if (t.name == 's') return sin(arg);   /* sin */
        if (t.name == 'c') return cos(arg);   /* cos */
        if (t.name == 't') return tan(arg);   /* tan */
        if (t.name == 'l') return log(arg);   /* ln */
        if (t.name == 'L') return log10(arg); /* log10 */
        if (t.name == 'q') return sqrt(arg);  /* sqrt */
        if (t.name == 'a') return fabs(arg);  /* abs */
        return arg;
    }
    if (t.type == TK_LPAREN) {
        double val = parse_expr();
        while (*s_pos == ' ') s_pos++;
        if (*s_pos == ')') s_pos++;
        return val;
    }
    return 0.0;
}

static double parse_unary(void) {
    while (*s_pos == ' ') s_pos++;
    if (*s_pos == '-') {
        s_pos++;
        return -parse_unary();
    }
    if (*s_pos == '+') {
        s_pos++;
        return parse_unary();
    }
    return parse_primary();
}

static double parse_power(void) {
    double base = parse_unary();
    while (*s_pos == ' ') s_pos++;
    if (*s_pos == '^' || (*s_pos == '*' && *(s_pos+1) == '*')) {
        if (*s_pos == '*') s_pos += 2;
        else s_pos++;
        double exp = parse_power();
        return pow(base, exp);
    }
    return base;
}

static double parse_term(void) {
    double val = parse_power();
    for (;;) {
        while (*s_pos == ' ') s_pos++;
        if (*s_pos == '*') {
            s_pos++;
            val *= parse_power();
        } else if (*s_pos == '/') {
            s_pos++;
            double d = parse_power();
            if (d != 0.0) val /= d;
            else val = NAN;
        } else break;
    }
    return val;
}

static double parse_expr(void) {
    double val = parse_term();
    for (;;) {
        while (*s_pos == ' ') s_pos++;
        if (*s_pos == '+') {
            s_pos++;
            val += parse_term();
        } else if (*s_pos == '-') {
            s_pos++;
            val -= parse_term();
        } else break;
    }
    return val;
}

int mc_eval(const char* expr, double* out) {
    if (!expr || !out) return -1;
    s_pos = expr;
    *out = parse_expr();
    return 0;
}
