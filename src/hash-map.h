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

/* This is a hash-table library using macro template, which is not very
 * beautiful ... */
#ifndef __HASH_H
#define __HASH_H

#include <math.h>
#include <stdint.h>

#ifdef __GNUC__
#  define NUNUSED __attribute__ ((unused))
#else
#  define NUNUSED
#endif

static inline uint32_t next_prime(uint32_t value)
{
    for (;;) {
        uint32_t lmt = (uint32_t)sqrt(value);
        for (uint32_t i = 2; i < lmt; ++i) {
            bool flag = true;
            if (!(value % i)) {
                flag = false;
                break;
            }
            if (flag)
                return value;
        }
        ++value;
    }
}

#define HASH_INIT_SIZE    991
#define HASH_MULT_FACTPR  10

/* a macro to create all the hash function we need ... */
#define HASH_MAP_INIT(key_t, value_t, cmp, hash, prefix) \
\
typedef struct prefix##_node_t { \
    key_t key; \
    value_t value; \
    struct prefix##_node_t *next; \
} prefix##_node_t; \
\
typedef struct prefix##_t { \
    prefix##_node_t **hlist; \
    size_t hsize; \
    size_t size; \
} prefix##_t; \
\
static void prefix##_init(prefix##_t *htable) \
{ \
    htable->size = 0; \
    htable->hsize = HASH_INIT_SIZE; \
    htable->hlist = mcalloc(HASH_INIT_SIZE, sizeof(void *)); \
} \
\
static void prefix##_retable(prefix##_t *htable) \
{ \
    uint32_t nhsize = htable->hsize * HASH_MULT_FACTPR; \
    prefix##_node_t **nlist = mcalloc(sizeof(void *), nhsize); \
    for (size_t i = 0; i < htable->hsize; ++i) { \
        prefix##_node_t *p = htable->hlist[i]; \
        while (p) { \
            uint32_t hvalue = hash(p->key) % nhsize; \
            prefix##_node_t *q = p->next; \
            p->next = nlist[hvalue]; \
            nlist[hvalue] = p; \
            p = q; \
        } \
    } \
    mfree(htable->hlist); \
    htable->hlist = nlist; \
    htable->hsize = nhsize; \
} \
\
static void prefix##_insert(prefix##_t *htable, key_t key, value_t value) \
{ \
    if (htable->size >= htable->hsize) \
        prefix##_retable(htable); \
    \
    uint32_t hvalue = hash(key) % htable->hsize; \
    prefix##_node_t **p = &htable->hlist[hvalue]; \
    while (*p && !cmp((*p)->key, key)) \
        p = &(*p)->next; \
    if (*p) { \
        (*p)->value = value; \
        return; \
    } else { \
        *p = mmalloc(sizeof(prefix##_node_t)); \
        (*p)->key = key; \
        (*p)->value = value; \
        (*p)->next = NULL; \
        ++htable->size; \
    } \
} \
\
static value_t *prefix##_find(prefix##_t *htable, key_t key) \
{ \
    uint32_t hvalue = hash(key) % htable->hsize; \
    prefix##_node_t *p = htable->hlist[hvalue]; \
    while (p) { \
        if (cmp(p->key, key)) { \
            return &p->value; \
        } \
        p = p->next; \
    } \
    return NULL; \
} \
\
NUNUSED static value_t *prefix##_find_default(prefix##_t *htable, \
                                              key_t key, value_t value) \
{ \
    uint32_t hvalue = hash(key) % htable->hsize; \
    prefix##_node_t *p = htable->hlist[hvalue]; \
    while (p) { \
        if (cmp(p->key, key)) { \
            return &p->value; \
        } \
        p = p->next; \
    } \
    p = mmalloc(sizeof(prefix##_node_t)); \
    p->key = key; \
    p->value = value; \
    p->next = htable->hlist[hvalue]; \
    htable->hlist[hvalue] = p; \
    return &p->value; \
} \
\
NUNUSED static void prefix##_delete(prefix##_t *htable, key_t key) \
{ \
    uint32_t hvalue = hash(key) % htable->hsize; \
    prefix##_node_t *p = htable->hlist[hvalue]; \
    if (!p) \
        return; \
    if (cmp(p->key, key)) { \
        htable->hlist[hvalue] = p->next; \
        return; \
    } \
    while (p->next) { \
        if (cmp(p->next->key, key)) { \
            p->next = p->next->next; \
            return ; \
        } \
        p = p->next; \
    } \
} \
_Pragma("GCC diagnostic pop") \
\
static void prefix##_free(prefix##_t *htable) \
{ \
    if (htable == NULL) \
        return; \
    for (size_t i = 0; i < htable->hsize; ++i) { \
        prefix##_node_t *p = htable->hlist[i]; \
        while (p) { \
            prefix##_node_t *q = p->next; \
            mfree(p); \
            p = q; \
        } \
    } \
    htable->size = 0; \
    mfree(htable->hlist); \
} \

#endif /* end of include guard: __HASH_H */
