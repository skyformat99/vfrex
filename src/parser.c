/* The MIT License (MIT)
 *
 * Copyright (c) 2012, Yichao Zhou
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "parser.h"

#include <string.h>
#include <ctype.h>

#ifdef DEBUG
#include <stdio.h>
#endif

static vfrex_option_t option;
uchar next_char;

static uchar tooppo(int c)
{
    if (islower(c))
        return (uchar)toupper(c);
    else
        return (uchar)tolower(c);
}

static uint32_t precedence(operator_t opt)
{
    switch (opt) {
    case REGEX_ZERO_ONE:
    case REGEX_REPEAT:
    case REGEX_REPEAT_ALO:
    case REGEX_ZERO_ONE_NG:
    case REGEX_REPEAT_NG:
    case REGEX_REPEAT_ALO_NG:
        return 4;
    case REGEX_CONCATE:
        return 3;
    case REGEX_OR:
        return 2;
    case REGEX_PARENT_LEFT:
    case REGEX_PARENT_RIGHT:
        return 1;
    case REGEX_REGEX_START:
    case REGEX_REGEX_END:
        return 0;
    default:
        assert(0);
    }
}

static void gen_default_charset(range_a *range) {
    switch (next_char) {
    case '.':
        arr_push(*range, ((range_t){ 32, 127 }));
        arr_push(*range, ((range_t){ '\t', '\t' }));
        break;

    case 's':
        arr_push(*range, ((range_t){ ' ', ' ' }));
        arr_push(*range, ((range_t){ '\t', '\t' }));
        break;

    case 'd':
        arr_push(*range, ((range_t){ '0', '9' }));
        break;

    case 'x':
        arr_push(*range, ((range_t){ '0', '9' }));
        arr_push(*range, ((range_t){ 'A', 'F' }));
        arr_push(*range, ((range_t){ 'a', 'f' }));
        break;

    case 'o':
        arr_push(*range, ((range_t){ '0', '7' }));
        break;

    case 'w':
        arr_push(*range, ((range_t){ '0', '9' }));
        arr_push(*range, ((range_t){ 'A', 'Z' }));
        arr_push(*range, ((range_t){ 'a', 'z' }));
        break;

    case 'h':
        arr_push(*range, ((range_t){ 'A', 'Z' }));
        arr_push(*range, ((range_t){ 'a', 'z' }));
        arr_push(*range, ((range_t){ '_', '_' }));
        break;

    case 'a':
        arr_push(*range, ((range_t){ 'A', 'Z' }));
        arr_push(*range, ((range_t){ 'a', 'z' }));
        break;

    case 'l':
        arr_push(*range, ((range_t){ 'a', 'z' }));
        if (option.ignore_case)
            arr_push(*range, ((range_t){ 'A', 'Z' }));
        break;

    case 'u':
        arr_push(*range, ((range_t){ 'A', 'Z' }));
        if (option.ignore_case)
            arr_push(*range, ((range_t){ 'a', 'z' }));
        break;
    }
}

static uint32_t next_token(uchar **s)
{
    uchar c0 = **s;
    (*s)++;
    uchar c1 = **s;

    if (isalnum(c0)) {
        next_char = c0;
        return REGEX_CHAR;
    }

    switch (c0) {
    case '\\':
        (*s)++;
        switch (c1) {
        case '(':
            switch (option.style) {
            case REGEX_STYLE_POSIX:
            case REGEX_STYLE_POSIX_GNU:
            case REGEX_STYLE_PERL:
            case REGEX_STYLE_VIM_MAGIC:
            case REGEX_STYLE_VIM_VERY_MAGIC:
            case REGEX_STYLE_VIM_NOMAGIC:
            case REGEX_STYLE_VIM_VERY_NOMAGIC:
            case REGEX_STYLE_MIXED:
                next_char = '(';
                return REGEX_CHAR;
            }

        case ')':
            switch (option.style) {
            case REGEX_STYLE_POSIX:
            case REGEX_STYLE_POSIX_GNU:
            case REGEX_STYLE_PERL:
            case REGEX_STYLE_VIM_MAGIC:
            case REGEX_STYLE_VIM_VERY_MAGIC:
            case REGEX_STYLE_VIM_NOMAGIC:
            case REGEX_STYLE_VIM_VERY_NOMAGIC:
            case REGEX_STYLE_MIXED:
                next_char = ')';
                return REGEX_CHAR;
            }

        case '+':
            switch (option.style) {
            case REGEX_STYLE_POSIX:
            case REGEX_STYLE_POSIX_GNU:
            case REGEX_STYLE_PERL:
            case REGEX_STYLE_VIM_MAGIC:
            case REGEX_STYLE_VIM_VERY_MAGIC:
            case REGEX_STYLE_VIM_NOMAGIC:
            case REGEX_STYLE_VIM_VERY_NOMAGIC:
            case REGEX_STYLE_MIXED:
                next_char = '+';
                return REGEX_CHAR;
            }

        case '?':
            switch (option.style) {
            case REGEX_STYLE_POSIX:
            case REGEX_STYLE_POSIX_GNU:
            case REGEX_STYLE_PERL:
            case REGEX_STYLE_VIM_MAGIC:
            case REGEX_STYLE_VIM_VERY_MAGIC:
            case REGEX_STYLE_VIM_NOMAGIC:
            case REGEX_STYLE_VIM_VERY_NOMAGIC:
            case REGEX_STYLE_MIXED:
                next_char = '?';
                return REGEX_CHAR;
            }
            assert(0);
            break;

        case '\\':
            next_char = '\\';
            return REGEX_CHAR;

        case 's':
        case 'd':
        case 'x':
        case 'o':
        case 'w':
        case 'h':
        case 'a':
        case 'l':
        case 'u':
            next_char = c1;
            return REGEX_CHARSET;

        default:
            (*s)--;
            next_char = '\\';
            return REGEX_CHAR;
        }

    case '(':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
        case REGEX_STYLE_PERL:
        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_PARENT_LEFT;
        }

    case ')':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
        case REGEX_STYLE_PERL:
        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_PARENT_RIGHT;
        }

    case '+':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
            return REGEX_REPEAT_ALO;

        case REGEX_STYLE_PERL:
            if (c1 == '?') {
                (*s)++;
                return REGEX_REPEAT_ALO_NG;
            }
            return REGEX_REPEAT_ALO;

        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_REPEAT_ALO;
        }

    case '?':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
            return REGEX_ZERO_ONE;

        case REGEX_STYLE_PERL:
            if (c1 == '?') {
                (*s)++;
                return REGEX_ZERO_ONE_NG;
            }
            return REGEX_ZERO_ONE;

        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_ZERO_ONE;
        }

    case '*':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
            return REGEX_REPEAT;

        case REGEX_STYLE_PERL:
            if (c1 == '?') {
                (*s)++;
                return REGEX_REPEAT_NG;
            }
            return REGEX_REPEAT;

        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_REPEAT;
        }

    case '|':
        switch (option.style) {
        case REGEX_STYLE_POSIX:
        case REGEX_STYLE_POSIX_GNU:
        case REGEX_STYLE_PERL:
        case REGEX_STYLE_VIM_MAGIC:
        case REGEX_STYLE_VIM_VERY_MAGIC:
        case REGEX_STYLE_VIM_NOMAGIC:
        case REGEX_STYLE_VIM_VERY_NOMAGIC:
        case REGEX_STYLE_MIXED:
            return REGEX_OR;
        }
    case '.':
        next_char = '.';
        return REGEX_CHARSET;
    case '\0':
        assert(0);
        return REGEX_REGEX_END;
    default:
        next_char = c0;
        return REGEX_CHAR;
    }
    assert(0);
    return REGEX_NOTHING;
}

static void choose_algorithm(vfrex_t vfrex)
{
    bool is_shift_or_32 = true;
    bool is_shift_or_64 = false;
    bool is_boyer_moore = true;
    bool is_dfa = true;
    bool is_nfa = true;
    size_t num_char = 0;

    symbol_t *exp = vfrex->exp.v;
    for (size_t i = 0; i < vfrex->exp.len; ++i) {
        operator_t op = exp[i].kind;
        if (op == REGEX_CHAR)
            ++num_char;
        if (op != REGEX_CHAR && op != REGEX_CONCATE) {
            is_shift_or_32 = false;
            is_shift_or_64 = false;
            is_boyer_moore = false;
        }
    }
    if (num_char > 64)
        is_shift_or_64 = false;
    else if (num_char > 32)
        is_shift_or_32 = false;

    if (is_shift_or_32)
        vfrex->algorithm = REGEX_SHIFT_OR_32;
    else if (is_shift_or_64)
        vfrex->algorithm = REGEX_SHIFT_OR_64;
    else if (is_boyer_moore)
        vfrex->algorithm = REGEX_BOYER_MOORE;
    else if (is_dfa)
        vfrex->algorithm = REGEX_DFA;
    else if (is_nfa)
        vfrex->algorithm = REGEX_NFA;
    else
        assert(0);
}

/* The main parser routine */
extern void parser_parse(vfrex_t vfrex)
{
    option = vfrex->option;

    uchar *regex      = vfrex->regex;
    typedef array(operator_t) operator_a;

    symbol_a   exp;
    symbol_a   token;
    operator_a stack;

    arr_init(exp);
    arr_init(token);
    arr_init(stack);

    /* first scan, get all the symbol(token) */
    symbol_t sym;
    sym.kind = REGEX_REGEX_START;
    arr_push(token, sym);

    while (*regex) {
        operator_t kind = next_token(&regex);

        sym.kind = kind;
        if (kind == REGEX_CHAR || kind == REGEX_CHARSET) {
            sym.ch = mmalloc(sizeof(range_a));
            arr_init(*sym.ch);

            if (kind == REGEX_CHAR) {
                arr_push(*sym.ch, ((range_t){ next_char, next_char }));
                if (option.ignore_case && isalpha(next_char)) {
                    next_char = (uchar)tooppo(next_char);
                    arr_push(*sym.ch, ((range_t){ next_char, next_char }));
                }
            } else {
                gen_default_charset(sym.ch);
            }
        }
        arr_push(token, sym);
    }
    sym.kind = REGEX_REGEX_END;
    arr_push(token, sym);

#ifdef DEBUG
    for (size_t i = 0; i < token.len; ++i) {
        if (i)
            printf(" ");
        if (token.v[i].kind == REGEX_CHAR)
            printf("(REGEX_CHAR: %c)", token.v[i].ch->v[0].lower);
        else
            printf("(%s)", operator_to_str(token.v[i].kind));
    }
    puts("");
#endif

#define maintain(operator) {                                       \
    uint32_t level = precedence(operator);                         \
    for (operator_t *p = stack.v+stack.len-1; p >= stack.v; --p) { \
        if (precedence(*p) >= level) {                             \
            --stack.len;                                           \
            sym.kind = *p;                                         \
            arr_push(exp, sym);                                    \
        } else                                                     \
        break;                                                     \
    }                                                              \
    arr_push(stack, operator);                                     \
}
    /* second scan, build the reverse polish expression */
    arr_push(stack, REGEX_REGEX_START);
    for (size_t i = 1; i < token.len; ++i) {
        switch (token.v[i].kind) {
        case REGEX_CHAR:
        case REGEX_CHARSET:
            /* Note that [|, (, BOL] is contained in left parent */
            if (!is_left_parent(token.v[i-1].kind)) {
                maintain(REGEX_CONCATE);
            }
            arr_push(exp, token.v[i]);
            break;

        case REGEX_OR:
            /* when we mean situation like (|xxx) */
            if (is_left_parent(token.v[i-1].kind)) {
                sym.kind = REGEX_NOTHING;
                arr_push(exp, sym);
            }
            maintain(REGEX_OR);

            /* when we have situation like (xxx|) */
            if (is_right_parent(token.v[i+1].kind)) {
                sym.kind = REGEX_NOTHING;
                arr_push(exp, sym);
            }
            break;

        case REGEX_ZERO_ONE:
        case REGEX_REPEAT:
        case REGEX_REPEAT_ALO:
        case REGEX_ZERO_ONE_NG:
        case REGEX_REPEAT_NG:
        case REGEX_REPEAT_ALO_NG:
            /* error handle */
            if (is_left_parent(token.v[i-1].kind)) {
                if (token.v[i].kind == REGEX_ZERO_ONE ||
                    token.v[i].kind == REGEX_ZERO_ONE_NG)
                    vfrex->status = VFREX_INVALID_QUESTION_MARK;
                else if (token.v[i].kind == REGEX_REPEAT ||
                         token.v[i].kind == REGEX_REPEAT_NG)
                    vfrex->status = VFREX_INVALID_STAR;
                else
                    vfrex->status = VFREX_INVALID_PLUS;
                longjmp(env, 1);
            }
            maintain(token.v[i].kind);
            break;

        case REGEX_PARENT_LEFT:
            if (!is_left_parent(token.v[i-1].kind)) {
                maintain(REGEX_CONCATE);
            }
            arr_push(stack, REGEX_PARENT_LEFT);
            break;

        case REGEX_PARENT_RIGHT:
            for (operator_t *p = stack.v+stack.len-1; p >= stack.v; --p) {
                --stack.len;
                if (*p == REGEX_PARENT_LEFT)
                    break;
                if (*p == REGEX_REGEX_START) {
                    vfrex->status = VFREX_UNMATCH_PARENTHESES;
                    longjmp(env, vfrex->status);
                }
                sym.kind = *p;
                arr_push(exp, sym);
            }
            break;

        case REGEX_REGEX_END:
            for (operator_t *p = stack.v+stack.len-1; p >= stack.v; --p) {
                --stack.len;
                if (*p == REGEX_REGEX_START)
                    break;
                if (*p == REGEX_PARENT_LEFT) {
                    vfrex->status = VFREX_UNMATCH_PARENTHESES;
                    longjmp(env, vfrex->status);
                }
                sym.kind = *p;
                arr_push(exp, sym);
            }
            break;

        default:
            assert(0);
            break;
        }
#undef maintain
#ifdef DEBUG
        printf("Stack: ");
        for (size_t i = 0; i < stack.len; ++i) {
            if (i)
                printf(" ");
            printf("(%s)", operator_to_str(stack.v[i]));
        }
        printf("\nOutput: ");
        for (size_t i = 0; i < exp.len; ++i) {
            if (i)
                printf(" ");
            if (exp.v[i].kind == REGEX_CHAR)
                printf("(REGEX_CHAR: %c)", exp.v[i].ch->v[0].lower);
            else
                printf("(%s)", operator_to_str(exp.v[i].kind));
        }
        puts("");
        puts("");
#endif
    }

    vfrex->exp = exp;
    choose_algorithm(vfrex);

    arr_free(token);
    arr_free(stack);
}

#ifdef DEBUG_MAIN

#include <stdlib.h>
#include "debug.h"

int main(void)
{
    setbuf(stdout, NULL);
    struct vfrex_t vfrex;
    memset(&vfrex, 0, sizeof(vfrex));
    /* vfrex.regex = (uchar *)"abcdefg\\(a\\)b"; */
    vfrex.regex = (uchar *)"a|(.*|b.*)";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    int jmp = setjmp(env);
    if (0 == jmp)
        parser_parse(&vfrex);
    else {
        printf("Runtime error %d\n", jmp);
    }
}

#endif
