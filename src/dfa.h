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
#ifndef __DFA_H
#define __DFA_H

#include "common.h"
#include <setjmp.h>

typedef enum node_kind_t {
    NODE_NULL,
    NODE_CHAR,
    NODE_BRANCH,
    NODE_ACCEPT,
} node_kind_t;

typedef struct nnode_t nnode_t;
typedef struct nnode_t {
    /* char in [lower, upper] is accepted */
    range_a      range;
    node_kind_t  kind;
    nnode_t     *next;
    nnode_t     *next0;
    uint32_t     last;
#ifdef DEBUG
    int32_t      index;
#endif
} nnode_t;

typedef array(nnode_t *) state_a;

typedef struct dnode_t dnode_t;
typedef struct dnode_t {
    state_a  states;
    dnode_t *to[256];
    /* current state contains an accept node */
    bool     is_accept;
} dnode_t;

typedef struct hash_t hash_t;
typedef struct FSM_t {
    nnode_t *NFA;
    dnode_t *DFA;
    hash_t  *hash;
    size_t   DFA_size;   /* TODO */
} FSM_t;

extern jmp_buf env;
extern uint32_t timeline;

extern void DFA_compile(vfrex_t vfrex);
/* The return value just means whether we find a match */
extern bool DFA_match(const uchar *text, size_t len, vfrex_t vfrex);

#endif /* end of include guard: __DFA_H */

