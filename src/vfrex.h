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

#ifndef __VFREX_H
#define __VFREX_H

#include <stddef.h>
#include "vfrex-share.h"

#define VFREX_SUCCESS 0
#define VFREX_INVALID 1
#define VFREX_UNKNOWN 2

#ifdef __cplusplus
typedef void *vfrex_t;
#else
typedef struct vfrex_t *vfrex_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /* Generate a default option */
    vfrex_option_t default_option();
    /* Grammar sugar:  If the matching is OK, it will return vfrex.  You can
     * use vfrex_group to extract the boundary and submatch.  Otherwise it will
     * return NULL. */
    vfrex_t vfrex_match(const char *text, const char *regex, vfrex_option_t option);

    /* TODO Grammar sugar:  replace regex with target in text.  The return
     * valus is the error code */
    int vfrex_replace(char *text, const char *regex, const char *target,
                      vfrex_option_t option);

    /* Compile the regex into a vfrex engine for future use.  If regex is not
     * valid, it will vfrex will become NULL.  The return value is the error
     * code */
    int vfrex_compile(vfrex_t *vfrex, const char *regex, vfrex_option_t option);

    /* match text with vfrex.  The result is store in vfrex.  You can read the
     * result of last matching by vfrex_result or vfrex_scanf. The return value
     * is the error code */
    int vfrex_object_match(vfrex_t vfrex, const char *text);

    /* TODO: the scanf style to view the result */
    int vfrex_scanf(vfrex_t vfrex, char *pat, ...);

    /* Get the number of groups.  The group 0 is the whole match */
    size_t vfrex_group_number(vfrex_t vfrex);
    int vfrex_group(size_t idx,         /* index of group number */
                    const char **left,  /* place to save the left boundary */
                    const char **right, /* place to save the right boundary */
                    vfrex_t vfrex);     /* regex engine */

    /* release the resource */
    void vfrex_free(vfrex_t *vfrex);

#ifdef __cplusplus
}
#endif

#endif
