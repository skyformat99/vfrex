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
/* This file is for debug proposal.  Don't compile it! */
#ifdef DEBUG

#include "debug.h"
#include "hash-map.h"

static int cmp(int x, int y)
{
    return x == y;
}

static uint32_t hash(int x)
{
    UNUSED(x);
    return (uint32_t)x;
}

HASH_MAP_INIT(int, int, cmp, hash, ihash);

ihash_t ht;

int main(void)
{
    ihash_init(&ht);
    ihash_insert(&ht, 1, 10);
    assert(!ihash_find(&ht, 2));
    assert(!ihash_find(&ht, 3));
    printf("gets: %d\n", *ihash_find(&ht, 1));
    ihash_insert(&ht, 1, 20);
    printf("gets: %d\n", *ihash_find(&ht, 1));


    for (int i = 0; i < 90000; ++i)
        ihash_insert(&ht, i, i);

    ihash_free(&ht);
    return 0;
}

#endif
