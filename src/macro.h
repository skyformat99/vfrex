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

#ifndef __MACRO_H
#define __MACRO_H

#define UNUSED(x) (void)(x)

#ifdef __GNUC__
#  define NUNUSED __attribute__ ((unused))
#else
#  define NUNUSED
#endif

#define cleanup(x) {                                                    \
    if (NULL != x) {                                                    \
        mfree(x);                                                       \
        x = NULL;                                                       \
    }                                                                   \
}

#define swap(x, y) {                                                    \
    typeof(x) z;                                                        \
    z = x;                                                              \
    x = y;                                                              \
    y = z;                                                              \
}

#define min(x, y) ({                                                    \
        typeof(x) _min1 = (x);                                          \
        typeof(y) _min2 = (y);                                          \
        (void) (&_min1 == &_min2);                                      \
        _min1 < _min2 ? _min1 : _min2; })

#define max(x, y) ({                                                    \
        typeof(x) _max1 = (x);                                          \
        typeof(y) _max2 = (y);                                          \
        (void) (&_max1 == &_max2);                                      \
        _max1 > _max2 ? _max1 : _max2; })

#endif

#define pair(x, y) struct { x a; y b; }
