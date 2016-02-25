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

#include "substring.h"
#include "macro.h"
#include <ctype.h>

int ignore_case;

#define filter(x) (ignore_case ? tolower(x) : (x))

#define SHIFT_OR_COMPILE_GENERATOR(SIZE) \
void shift_or_compile_##SIZE(vfrex_t vfrex) \
{ \
    ignore_case = vfrex->option.ignore_case; \
    assert(vfrex->regex_len <= SIZE); \
    cleanup(vfrex->shift_or); \
 \
    vfrex->shift_or     = mmalloc(256 * sizeof(uint##SIZE##_t)); \
    vfrex->algorithm    = REGEX_SHIFT_OR_##SIZE; \
    uint##SIZE##_t *has = vfrex->shift_or; \
    memset(has, 0xFF, 256 * sizeof(uint##SIZE##_t)); \
 \
    for (uchar *p = vfrex->regex; *p; ++p) \
        has[filter(*p)] &= ~((uint##SIZE##_t)1 << (p - vfrex->regex)); \
    return; \
}

#define SHIFT_OR_MATCH_GENERATOR(SIZE) \
bool shift_or_match_##SIZE(const uchar *text, size_t len, vfrex_t vfrex) \
{ \
    UNUSED(len); \
    if (vfrex->regex_len == 0) { \
        vfrex->group_number = 1; \
        vfrex->group_left   = mmalloc(sizeof(size_t)); \
        vfrex->group_right  = mmalloc(sizeof(size_t)); \
        *vfrex->group_left  = text; \
        *vfrex->group_right = text; \
        return true; \
    } \
    assert(vfrex->shift_or); \
    assert(vfrex->algorithm == REGEX_SHIFT_OR_##SIZE); \
 \
    uint##SIZE##_t d    = ~(uint##SIZE##_t)0; \
    uint##SIZE##_t mask =  (uint##SIZE##_t)1 << (vfrex->regex_len-1); \
    uint##SIZE##_t *has =   vfrex->shift_or; \
 \
    for (const uchar *t = text; *t; ++t) { \
        d = (d << 1) | has[filter(*t)]; \
        if (0 == (d & mask)) { \
            vfrex->group_number = 1; \
            vfrex->group_left   = mmalloc(sizeof(size_t)); \
            vfrex->group_right  = mmalloc(sizeof(size_t)); \
            *vfrex->group_left  = t + 1 - vfrex->regex_len; \
            *vfrex->group_right = t + 1; \
            return true; \
        } \
    } \
    return false; \
}

SHIFT_OR_COMPILE_GENERATOR(32)
SHIFT_OR_COMPILE_GENERATOR(64)
SHIFT_OR_MATCH_GENERATOR(32)
SHIFT_OR_MATCH_GENERATOR(64)

static size_t match_length(uchar *x, uchar *y)
{
    size_t ret = 0;
    while (*x && *y && *x++ == *y++)
        ++ret;
    return ret;
}

static void calc_z_table(const uchar *regex, size_t len, size_t *z)
{
    if (len == 0)
        return;

    z[0] = len;
    if (len == 1) {
        return;
    }

    uchar *rev = mmalloc((len+1) * sizeof(uchar));
    // reverse regex
    const uchar *p1 = regex;
    uchar       *p2 = rev + len - 1;
    for (; *p1; ++p1, --p2)
        *p2 = *p1;
    rev[len] = 0;

    z[1] = match_length(rev, rev+1);
    size_t i, j;
    for (i = 2; i <= z[1]; ++i)
        z[i] = z[1] - i + 1;

    j = 1;
    for (i = z[1]+1; i < len; ++i) {
        size_t jz = j + z[j];
        if (jz > i) {
            size_t i0 = i-j;
            if (i + z[i0] < jz)
                z[i] = z[i0];
            else if (i + z[i0] == jz)
                z[i] = jz - i;
            else
                z[i] = jz - i + match_length(rev+z[j], rev+jz);
        } else {
            z[i] = match_length(rev, rev+i);
        }
        if (i + z[i] > jz)
            j = i;
    }

    // reverse result;
    for (size_t *p = z, *q = z+len-1; p < q; ++p, --q) {
        size_t t = *p;
        *p = *q;
        *q = t;
    }

    mfree(rev);
}

static void calc_bad_char_table(const uchar *regex, size_t len,
                                size_t *z, int32_t *tab)
{
    UNUSED(z);
    memset(tab, 0x7F, 256 * sizeof(int32_t));
    for (size_t i = 0; i < len; ++i)
        tab[regex[i]] = i;
}

static void calc_good_suffix_table(const uchar *regex, size_t len,
                                   size_t *z, int32_t *tab)
{
    UNUSED(regex);
    memset(tab, -1, len * sizeof(int32_t));
    for (size_t i = 0; (int32_t)i < (int32_t)len-1; ++i) {
        tab[len-z[i]] = len-1-i;
    }
}

static void calc_full_jump_table(const uchar *regex, size_t len,
                                 size_t *z, int32_t *tab)
{
    UNUSED(regex);
    tab[0] = 0;
    int32_t now = -1;
    for (size_t i = 0; (int32_t)i < (int32_t)len-1; ++i) {
        if (z[i] == i+1)
            now = i;
        tab[len-1-i] = len-1-now;
    }
}

void boyer_moore_compile(vfrex_t vfrex)
{
#ifdef DEBUG
    puts("Boyer-Moore-Match");
#endif
    ignore_case = vfrex->option.ignore_case;

    size_t len                 = vfrex->regex_len;
    uchar *regex               = vfrex->regex;
    size_t *z                  = mmalloc((len+1) * sizeof(size_t));
    int32_t *bad_char_table    = mmalloc(256 * sizeof(int32_t));
    int32_t *good_suffix_table = mmalloc((len+1) * sizeof(int32_t));
    int32_t *full_jump_table   = mmalloc((len+1) * sizeof(int32_t));

    vfrex->BM_bad_char_table    = bad_char_table;
    vfrex->BM_full_jump_table   = full_jump_table;
    vfrex->BM_good_suffix_table = good_suffix_table;
    vfrex->algorithm            = REGEX_BOYER_MOORE;

    if (ignore_case)
        for (uchar *p = vfrex->regex; *p; ++p)
            *p = (uchar)tolower(*p);

    calc_z_table(regex, len, z);
    calc_bad_char_table(regex, len, z, bad_char_table);
    calc_good_suffix_table(regex, len, z, good_suffix_table);
    calc_full_jump_table(regex, len, z, full_jump_table);

    mfree(z);
}

bool boyer_moore_match(const uchar *text, size_t len, vfrex_t vfrex)
{
#ifdef DEBUG
    puts("Boyer-Moore-Match");
#endif
    assert(vfrex->algorithm == REGEX_BOYER_MOORE);
    assert(vfrex->BM_bad_char_table);
    assert(vfrex->BM_good_suffix_table);
    assert(vfrex->BM_full_jump_table);

    int32_t     *bad_char_table    = vfrex->BM_bad_char_table;
    int32_t     *good_suffix_table = vfrex->BM_good_suffix_table;
    int32_t     *full_jump_table   = vfrex->BM_full_jump_table;
    const uchar *regex             = vfrex->regex;

    int32_t k = vfrex->regex_len - 1;
    int32_t previous = -1;
    while (k < (int32_t)len) {
        int32_t i = vfrex->regex_len - 1;
        int32_t j = k;
        while (j >= 0 && j > previous && regex[i] == filter(text[j])) {
            --i;
            --j;
        }

        if (i == -1 || j == previous) {
            vfrex->group_number = 1;
            vfrex->group_left   = mmalloc(sizeof(size_t));
            vfrex->group_right  = mmalloc(sizeof(size_t));
            *vfrex->group_left  = text + k + 1 - vfrex->regex_len;
            *vfrex->group_right = text + k + 1;
            return true;

            /* previous = k; */
            /* if (len > 1) */
            /*     k += full_jump_table[1]; */
            /* else */
            /*     k += 1; */
        } else {
            int32_t shift_char = i - bad_char_table[filter(text[j])];
            int32_t shift_suffix;
            if (j == k)
                shift_suffix = 1;
            else if (good_suffix_table[i+1] == -1)
                shift_suffix = full_jump_table[i+1];
            else
                shift_suffix = good_suffix_table[i+1];
            int shift = max(shift_char, shift_suffix);
            if (shift > i)
                previous = k;
            k += shift;
        }
    }
    return false;
}
