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

#ifndef __COMMON_H
#define __COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "array.h"
#include "vfrex-share.h"

typedef uint8_t uchar;

typedef enum operator_t {
    REGEX_CHAR = 0,  /* a uint8_t */
    REGEX_BOL, /* '^': begin of line */
    REGEX_EOL, /* '$': end of line */
    REGEX_EOF,
    REGEX_NOTHING,  /* a zero length char */
    REGEX_CHARSET,
    REGEX_CONCATE,
    REGEX_OR,
    REGEX_ZERO_ONE,
    REGEX_REPEAT, /* '*': repeat any times */
    REGEX_REPEAT_ALO, /* '+': repeat at least once*/
    REGEX_ZERO_ONE_NG, /* Not-Greedy version */
    REGEX_REPEAT_NG, /* Not-Greedy version */
    REGEX_REPEAT_ALO_NG, /* Not-Greedy version */
    REGEX_START, /* \zs, for replacing only current */
    REGEX_END,   /* \ze, for replacing only current */
    REGEX_REGEX_START,
    REGEX_REGEX_END,
    REGEX_PARENT_LEFT,
    REGEX_PARENT_RIGHT,
    REGEX_WORD_BOUNDARY,
    REGEX_WORD_BOUNDARY_LEFT,
    REGEX_WORD_BOUNDARY_RIGHT,
} operator_t;

#define is_symbol(x) \
    (((x) == REGEX_CHAR) |\
     ((x) == REGEX_NOTHING) |\
     ((x) == REGEX_CHARSET))

#define is_assertion(x) \
    (((x) == REGEX_BOL) || \
     ((x) == REGEX_EOL) || \
     ((x) == REGEX_EOF) || \
     ((x) == REGEX_WORD_BOUNDARY) || \
     ((x) == REGEX_WORD_BOUNDARY_LEFT) || \
     ((x) == REGEX_WORD_BOUNDARY_RIGHT))

#define is_operator(x) \
    (((x) == REGEX_CONCATE) || \
     ((x) == REGEX_OR) || \
     ((x) == REGEX_ZERO_ONE) || \
     ((x) == REGEX_REPEAT) || \
     ((x) == REGEX_REPEAT_ALO))

#define is_left_parent(x) \
    (((x) == REGEX_PARENT_LEFT) || \
     ((x) == REGEX_REGEX_START) || \
     ((x) == REGEX_OR))

#define is_right_parent(x) \
    (((x) == REGEX_PARENT_RIGHT) || \
     ((x) == REGEX_REGEX_END) || \
     ((x) == REGEX_OR))

#define is_parent(x) \
    ((is_left_parent(x)) || \
     (is_right_parent(x)))

typedef struct range_t {
    uchar lower;
    uchar upper;
} range_t;

typedef array(range_t) range_a;

typedef struct symbol_t {
    range_a   *ch;
    operator_t kind;
} symbol_t;

typedef enum algorithm_t {
    REGEX_SHIFT_OR_32,
    REGEX_SHIFT_OR_64,
    REGEX_BOYER_MOORE,
    REGEX_DFA,
    REGEX_NFA,
} algorithm_t;

char *operator_to_str(operator_t);
char *algorithm_to_str(algorithm_t);

typedef struct FSM_t    FSM_t;
typedef array(symbol_t) symbol_a;

typedef struct vfrex_t {
    /* original string */
    uchar       *regex;
    size_t       regex_len;

    /* reverse polish expression */
    symbol_a     exp;

    algorithm_t  algorithm;
    void        *shift_or;
    /* TODO:  Clean up the namespace */
    int32_t     *BM_bad_char_table;
    int32_t     *BM_good_suffix_table;
    int32_t     *BM_full_jump_table;

    /* FSM[0] is the forward direction FSM
     * FSM[1] is the backward direction FSM */
    FSM_t       *FSM[2];

    size_t        group_number;
    const uchar **group_left;
    const uchar **group_right;

    vfrex_error_t  status;
    vfrex_option_t option;
} *vfrex_t;

extern void *(*mmalloc)(size_t);
extern void  (*mfree)(void *);
extern void *(*mrealloc)(void *, size_t);
extern void *(*mcalloc)(size_t, size_t);

#ifndef NDEBUG
#  define logff(...) fprint(stderr, __VAR_ARGS__)
#else
#  define logff(...) ((void)0)
#endif

#endif
