/* The MIT License (MIT)
 *
 * Copyright (c) 2012, 2013, Yichao Zhou
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

#include "common.h"
#include "macro.h"
#include "parser.h"
#include "substring.h"
#include "dfa.h"
#include "vfrex.h"
#include <stdlib.h>

jmp_buf env;
void *(*mmalloc)(size_t)          = malloc;
void  (*mfree)(void *)            = free;
void *(*mrealloc)(void *, size_t) = realloc;
void *(*mcalloc)(size_t, size_t)  = calloc;

void set_alloc(
        void *(*my_malloc)(size_t),
        void  (*my_free)(void *),
        void *(*my_realloc)(void *, size_t),
        void *(*my_calloc)(size_t, size_t)
)
{
    mmalloc = my_malloc;
    mfree = my_free;
    mrealloc = my_realloc;
    mcalloc = my_calloc;
}

vfrex_option_t default_option()
{
    return (vfrex_option_t) {
        REGEX_STYLE_POSIX,
        REGEX_MATCH_PARTIAL_BOOL,
        0,
    };
}

int vfrex_replace(char *text, const char *regex, const char *target,
                  vfrex_option_t option)
{
    /* TODO */
    assert(0);
    UNUSED(text);
    UNUSED(regex);
    UNUSED(target);
    UNUSED(option);
    return VFREX_SUCCESS;
}

vfrex_t vfrex_match(const char *text, const char *regex, vfrex_option_t option)
{
    vfrex_t vfrex;

    if (VFREX_SUCCESS != vfrex_compile(&vfrex, regex, option)) {
        vfrex_free(&vfrex);
        return NULL;
    }
    if (VFREX_SUCCESS != vfrex_object_match(vfrex, text)) {
        vfrex_free(&vfrex);
        return NULL;
    }
    return vfrex;
}

/* We will put our result into vfrex */
int vfrex_compile(vfrex_t *vfrex, const char *_regex, vfrex_option_t option)
{
    *vfrex = mcalloc(1, sizeof(struct vfrex_t));

    size_t       len   = strlen(_regex);
    const uchar *regex = (const uchar *)_regex;

    (*vfrex)->regex     = mmalloc((len+1) * sizeof(uchar));
    (*vfrex)->regex_len = len;
    (*vfrex)->option    = option;
    (*vfrex)->status    = VFREX_SUCCESS;
    strcpy((char *)(*vfrex)->regex, (const char *)regex);

    if (!setjmp(env)) {
        parser_parse(*vfrex);

        switch ((*vfrex)->algorithm) {
        case REGEX_SHIFT_OR_32:
            shift_or_compile_32(*vfrex);
            break;

        case REGEX_SHIFT_OR_64:
            shift_or_compile_64(*vfrex);
            break;

        case REGEX_BOYER_MOORE:
            boyer_moore_compile(*vfrex);
            break;

        case REGEX_DFA:
            DFA_compile(*vfrex);
            break;

        case REGEX_NFA:
            assert(0);
        }
    }

    int ret = (*vfrex)->status;
    if (VFREX_SUCCESS != ret)
        vfrex_free(vfrex);
    return ret;
}

/* Using the object we compiled to do full matching */
int vfrex_object_match(vfrex_t vfrex, const char *_text)
{
    if (!vfrex)
        return VFREX_INVALID_COMPLIATION;

    cleanup(vfrex->group_left);
    cleanup(vfrex->group_right);
    vfrex->group_number = 0;
    vfrex->status = VFREX_SUCCESS;

    size_t tlen = strlen(_text);
    const uchar *text = (const uchar *)_text;
    
    bool found;

    if (!setjmp(env)) {
        parser_parse(vfrex);

        switch (vfrex->algorithm) {
        case REGEX_SHIFT_OR_32:
            found = shift_or_match_32(text, tlen, vfrex);
            break;

        case REGEX_SHIFT_OR_64:
            found = shift_or_match_64(text, tlen, vfrex);
            break;

        case REGEX_BOYER_MOORE:
            found = boyer_moore_match(text, tlen, vfrex);
            break;

        case REGEX_DFA:
            found = DFA_match(text, tlen, vfrex);
            break;

        case REGEX_NFA:
            assert(0);
            break;
        }
    }

    if (vfrex->status != VFREX_SUCCESS)
        return vfrex->status;

    if (found)
        return VFREX_SUCCESS;
    else
        return VFREX_NOT_FOUND;
}

int vfrex_scanf(vfrex_t vfrex, char *pat, ...)
{
    UNUSED(vfrex);
    UNUSED(pat);
    /* TODO */
    return 0;
}

size_t vfrex_group_number(vfrex_t vfrex)
{
    if (vfrex->status != VFREX_SUCCESS)
        return 0;
    return vfrex->group_number;
}

int vfrex_group(size_t idx, const char **left, const char **right, vfrex_t vfrex)
{
    if (vfrex->status != VFREX_SUCCESS)
        return vfrex->status;
    assert(idx < vfrex->group_number);
    *left  = (const char *)vfrex->group_left[idx];
    *right = (const char *)vfrex->group_right[idx];
    return VFREX_SUCCESS;
}

void vfrex_free(vfrex_t *vfrex)
{
    /* TODO */
    cleanup((*vfrex)->shift_or);
    cleanup((*vfrex)->BM_bad_char_table);
    cleanup((*vfrex)->BM_good_suffix_table);
    cleanup((*vfrex)->BM_full_jump_table);
    cleanup((*vfrex)->group_left);
    cleanup((*vfrex)->group_right);
    cleanup((*vfrex));
}

#ifdef DEBUG_MAIN

void judge(const char *regex, const char *text,
           bool match, int st, int ed)
{
    printf("\nTest case: %s <match> %s\n", regex, text);
    vfrex_option_t option = {
        REGEX_STYLE_POSIX,
        REGEX_MATCH_PARTIAL_BOUNDARY,
        0,
    };
    vfrex_t result = vfrex_match(text, regex, option);
    if (match == false)
        assert(NULL == result);
    else {
        assert(result);
        const char *left, *right;
        assert(1 == vfrex_group_number(result));
        assert(0 == vfrex_group(0, &left, &right, result));
        assert(left  == text + st - 1);
        assert(right == text + ed);
    }
}

int main(void)
{
    judge("abc", "abc", true, 1, 3);
    judge("a", "abc", true, 1, 1);
    judge("hello", "ahealleoahhelolhello", true, 16, 20);
    judge("hello|world", "hello", true, 1, 5);
    judge("hello|world", "hello world", true, 1, 5);
    judge("world|hello", "hello world", true, 1, 5);
    judge("hel|hello", "hello", true, 1, 3);
    judge("hello|hel", "hello", true, 1, 5);
}
#endif
