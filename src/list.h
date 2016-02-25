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

/* This is a macro library (for speed to avoid void*) to implement a
 * double-linked list */
#ifndef __LIST_H
#define __LIST_H

/* for typedef */
#define list(x) struct {         \
    struct x##_iter {          \
        x v;                     \
        struct x##_iter *next; \
        struct x##_iter *prev; \
    } *head, *tail;              \
}

/* iterator */
#define lst_iter(x) typeof((x).head)
#define lst_size(x) sizeof(*(x).head)

#define lst_init(x) {              \
    x.head = mmalloc(lst_size(x)); \
    x.tail = mmalloc(lst_size(x)); \
}

#define lst_link(x, y) { (x)->next = (y); (y)->prev = (x); }

#define lstit_insert(c, a, b) { lst_link(a, c); lst_link(c, b); }
#define lstit_delete(c) { lst_link(c->prev, c->next); mfree(c); }

#define lst_push(x, val) {                     \
    lst_iter(x) t = mmalloc(lst_size(x));      \
    t->v = (val);                              \
    lstit_insert(t, (x).tail->prev, (x).tail); \
}

#define lst_append(x, val) {                   \
    lst_iter(x) t = mmalloc(lst_size(x));      \
    t->v = (val);                              \
    lstit_insert(t, (x).head, (x).head->next); \
}

/* p is a new variable defined by this for */
#define lstit_for(p, x, y) for(typeof(x) p = (x); p != (y); p = p->next)
#define lst_for(p, x) lstit_for((p), (x).head, (x).tail)

/* Insert lst *after* iterator it.  After this operation, lst is set to empty.
 */
#define lst_insert(lst, it) {               \
    lst_link((it), (lst).head->next);       \
    lst_link((lst).tail->prev, (it)->next); \
}
#define lst_delete(x, y) {     \
    lst_link((x)->prev, y);    \
    typeof(x) p = x;           \
    while (p != y) {           \
        typeof(x) q = p->next; \
        mfree(p);              \
        p = q;                 \
    }                          \
}

/* make the list empty */
#define lst_clear(x) {              \
    lst_iter(x) p = (x).head->next; \
    while (p != (x).tail) {         \
        lst_iter(x) q = p->next;    \
        mfree(p);                   \
        p = q;                      \
    }                               \
}

/* delete and free the list totally.  If you want to use it again, you must
 * init it. */
#define lst_free(x) {            \
    lst_iter(x) p = (x).head;    \
    while (p != (x).tail) {      \
        lst_iter(x) q = p->next; \
        mfree(p);                \
        p = q;                   \
    }                            \
    mfree(p)                     \
    (x).head = NULL;             \
    (x).tail = NULL;             \
}

#endif /* end of include guard: __LIST_H */

