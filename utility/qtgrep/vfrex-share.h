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
#ifndef __VFREX_OPTION_H
#define __VFREX_OPTION_H

typedef enum vfrex_style_t {
    REGEX_STYLE_POSIX,
    REGEX_STYLE_POSIX_GNU,
    REGEX_STYLE_PERL,
    REGEX_STYLE_VIM_MAGIC,
    REGEX_STYLE_VIM_VERY_MAGIC,
    REGEX_STYLE_VIM_NOMAGIC,
    REGEX_STYLE_VIM_VERY_NOMAGIC,
    REGEX_STYLE_MIXED,
} vfrex_style_t;

typedef enum vfrex_match_t {
    REGEX_MATCH_FULL_BOOL,
    REGEX_MATCH_FULL_SUBMATCH,
    REGEX_MATCH_PARTIAL_BOOL,
    REGEX_MATCH_PARTIAL_BOUNDARY,
    REGEX_MATCH_PARTIAL_SUBMATCH,
} vfrex_match_t;

typedef struct vfrex_option_t {
    vfrex_style_t style;
    vfrex_match_t match;
    int ignore_case;
} vfrex_option_t;

typedef enum vfrex_error_t {
    VFREX_SUCCESS = 0,
    VFREX_NOT_FOUND,
    VFREX_UNMATCH_PARENTHESES,
    VFREX_INVALID_STAR,
    VFREX_INVALID_PLUS,
    VFREX_INVALID_QUESTION_MARK,
    VFREX_INVALID_UTF8,
    VFREX_INVALID_COMPLIATION,
} vfrex_error_t;

#endif
