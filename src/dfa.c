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

#include "dfa.h"
#include "macro.h"
#include "qsort.h"
#include "hash-map.h"

uint32_t timeline = 0;
#ifdef DEBUG
int32_t total_index = 0;
#endif

typedef array(nnode_t**) edge_a;

typedef struct stack_t {
    nnode_t *node;
    edge_a   edges;
} stack_t;
typedef array(stack_t) stack_a;

static void push(stack_a *stack, nnode_t *node, edge_a edges)
{
    stack->v[stack->len].node  = node;
    stack->v[stack->len].edges = edges;
    ++stack->len;
}

static stack_t *pop(stack_a *stack)
{
    --stack->len;
    return stack->v + stack->len;
}

static void NUNUSED debug_print_node(nnode_t *node)
{
    UNUSED(node);
#ifdef DEBUG
    ++total_index;
    node->index = total_index;
    switch (node->kind) {
    case NODE_NULL:
        printf("Node %d: NULL Node", total_index);
        break;

    case NODE_CHAR:
        printf("Node %d: Char Node ASCII %d~%d", total_index,
               node->range.v[0].lower, node->range.v[0].upper);
        break;

    case NODE_BRANCH:
        printf("Node %d: Branch Node", total_index);
        break;

    case NODE_ACCEPT:
        printf("Node %d: Accept Node", total_index);
        break;
    }
    printf(" %p %p\n", &node->next, &node->next0);
#endif
}

static void NUNUSED debug_print_dnode(dnode_t *node)
{
    UNUSED(node);
#ifdef DEBUG
    printf("dnode:");
    arr_for(state, node->states) {
        printf(" %d", (*state)->index);
    }
    puts("");
#endif
}

static void NUNUSED debug_print_graph(nnode_t *node)
{
    UNUSED(node);
#ifdef DEBUG
    assert(node);

    if (node->last == timeline)
        return;
    node->last = timeline;
    if (node->kind == NODE_BRANCH) {
        printf("Branch Edge1 %d to %d\n", node->index, node->next->index);
        printf("Branch Edge2 %d to %d\n", node->index, node->next0->index);
        debug_print_graph(node->next);
        debug_print_graph(node->next0);
    } else if (node->kind == NODE_CHAR) {
        printf("Char   Edge %d to %d with %d~%d\n",
               node->index, node->next->index,
               node->range.v[0].lower, node->range.v[0].upper);
        debug_print_graph(node->next);
    }
#endif
}

static nnode_t *new_null_node()
{
    nnode_t *ret = mmalloc(sizeof(nnode_t));
    ret->kind    = NODE_NULL;
    ret->last    = 0;
    debug_print_node(ret);
    return ret;
}

static nnode_t *new_char_node(range_a *ch)
{
    nnode_t *ret = mmalloc(sizeof(nnode_t));
    ret->kind    = NODE_CHAR;
    ret->range   = *ch;
    ret->last    = 0;
    debug_print_node(ret);
    return ret;
}

static nnode_t *new_branch_node(nnode_t *next, nnode_t *next0)
{
    nnode_t *ret = mmalloc(sizeof(nnode_t));
    ret->kind    = NODE_BRANCH;
    ret->next    = next;
    ret->next0   = next0;
    ret->last    = 0;
    debug_print_node(ret);
    return ret;
}

static nnode_t *new_accept_node()
{
    nnode_t *ret = mmalloc(sizeof(nnode_t));
    ret->kind    = NODE_ACCEPT;
    ret->last    = 0;
    debug_print_node(ret);
    return ret;
}

static void connect_edges(edge_a *edges, nnode_t *node)
{
    for (size_t i = 0; i < edges->len; ++i)
        *(edges->v[i]) = node;
    arr_free(*edges);
}

static edge_a new_edges(nnode_t **edges)
{
    edge_a ret;
    ret.v    = mmalloc(sizeof(void *));
    ret.v[0] = edges;
    ret.len  = 1;
    return ret;
}

/* TODO: We may change edge_a to edge_l.  List is more suitable here!!! */
static void combine_edges(edge_a *edges, edge_a *edges0)
{
    edges->v = mrealloc(edges->v, sizeof(void *) * (edges->len + edges0->len));
    for (size_t i = 0; i < edges0->len; ++i)
        edges->v[edges->len + i] = edges0->v[i];
    edges->len += edges0->len;
    arr_free(*edges0);
}

static void append_edges(edge_a *edges, nnode_t **edge)
{
    edges->v = mrealloc(edges->v, sizeof(void *) * (edges->len + 1));
    edges->v[edges->len] = edge;
    ++edges->len;
}

static void build_NFA(vfrex_t vfrex, bool flip, bool prepend, FSM_t *FSM)
{
    stack_a stack;
    symbol_t *exp = vfrex->exp.v;
    size_t    len = vfrex->exp.len;

    stack.len = 0;
    stack.v   = mmalloc(sizeof(stack_t) * len);

    for (size_t i = 0; i < len; ++i) {
        nnode_t *node;
        stack_t *s1, *s2;

        switch (exp[i].kind) {
        case REGEX_CHAR:
        case REGEX_CHARSET:
            node = new_char_node(exp[i].ch);
            push(&stack, node, new_edges(&node->next));
            break;

        case REGEX_NOTHING:
            node = new_null_node();
            push(&stack, node, new_edges(&node->next));
            break;

            node = new_char_node(exp[i].ch);
            push(&stack, node, new_edges(&node->next));
            break;

        case REGEX_CONCATE:
            assert(stack.len >= 2);
            /* REGEX_CONCATE is the only place we need to flip when we reverse the
             * regular expression */
            if (!flip) {
                s2 = pop(&stack);
                s1 = pop(&stack);
            } else {
                s1 = pop(&stack);
                s2 = pop(&stack);
            }

            connect_edges(&s1->edges, s2->node);
            push(&stack, s1->node, s2->edges);
            break;

        case REGEX_OR:
            assert(stack.len >= 2);
            /* We don't flip REGEX_OR or we may change the length of matched
             * string */
            s2 = pop(&stack);
            s1 = pop(&stack);

            combine_edges(&s1->edges, &s2->edges);
            push(&stack, new_branch_node(s1->node, s2->node), s1->edges);
            break;

        case REGEX_ZERO_ONE:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(s1->node, NULL);
            append_edges(&s1->edges, &node->next0);
            push(&stack, node, s1->edges);
            break;

        case REGEX_REPEAT:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(s1->node, NULL);
            connect_edges(&s1->edges, node);
            push(&stack, node, new_edges(&node->next0));
            break;

        case REGEX_REPEAT_ALO:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(s1->node, NULL);
            connect_edges(&s1->edges, node);
            push(&stack, s1->node, new_edges(&node->next0));
            break;

        case REGEX_ZERO_ONE_NG:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(NULL, s1->node);
            append_edges(&s1->edges, &node->next);
            push(&stack, node, s1->edges);
            break;

        case REGEX_REPEAT_NG:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(NULL, s1->node);
            connect_edges(&s1->edges, node);
            push(&stack, node, new_edges(&node->next));
            break;

        case REGEX_REPEAT_ALO_NG:
            assert(stack.len >= 1);
            s1 = pop(&stack);
            node = new_branch_node(NULL, s1->node);
            connect_edges(&s1->edges, node);
            push(&stack, s1->node, new_edges(&node->next));
            break;

        default:
            assert(0);
            break;
        }
    }
    assert(stack.len == 1);
    connect_edges(&stack.v[0].edges, new_accept_node());

    if (prepend) {
        range_a range;
        arr_init(range);
        arr_push(range, ((range_t){ 32, 127 }));
        arr_push(range, ((range_t){ '\t', '\t' }));
        nnode_t *node = new_char_node(&range);

        nnode_t *branch;
        /* NON-Greedy */
        branch = new_branch_node(stack.v[0].node, node);
        node->next = branch;
        FSM->NFA = branch;
    } else {
        FSM->NFA = stack.v[0].node;
    }

#ifdef DEBUG
    debug_print_graph(FSM->NFA);
    puts("=============");
#endif
    arr_free(stack);
}

#define ptr_cmp(x, y)   (*(x) < *(y))
#define state_cmp(x, y) ((x).len == (y).len && \
                         memcmp((x).v, (y).v, (x).len * sizeof(void *)) == 0)
static uint32_t state_hash(state_a x)
{
    uint32_t ret = 0;
    arr_for(i, x) {
        ret *= 133331;
        ret ^= (uint32_t) *i;
        ret += (uint32_t) *i;
    }
    return ret;
}

QSORT_INIT(nnode_t *, ptr_cmp, node);
HASH_MAP_INIT(state_a, dnode_t *, state_cmp, state_hash, hash);

/* BFS to get through all the branch node to get an initial set of states */
static void append_nnode(nnode_t *node, state_a *ret)
{
    typedef pair(nnode_t *, bool) pair_t;
    array(pair_t) stack;

    if (node->last != timeline) {
        node->last = timeline;
        arr_init(stack);
        arr_push(stack, ((pair_t){node, true}));
    }

    while (stack.len) {
        nnode_t *cnode = arr_back(stack).a;
        if (arr_back(stack).b) {
            /* first time */
            arr_back(stack).b = false;

            if (cnode->kind != NODE_BRANCH) {
                arr_push(*ret, cnode);
                arr_pop(stack);
            } else {
                if (cnode->next->last != timeline) {
                    cnode->next->last = timeline;
                    arr_push(stack, ((pair_t){cnode->next, true}));
                }
            }
        } else {
            /* second time */
            arr_pop(stack);
            if (cnode->next0->last != timeline) {
                cnode->next0->last = timeline;
                arr_push(stack, ((pair_t){cnode->next0, true}));
            }
        }
    }
    arr_free(stack);
}

static void handle_dnode(dnode_t *node, FSM_t *FSM)
{
    hash_insert(FSM->hash, node->states, node);
    arr_for(state, node->states)
        if ((*state)->kind == NODE_ACCEPT) {
            node->is_accept = true;
            break;
        }
}

static dnode_t *next_dnode(dnode_t *node, uchar c, FSM_t *FSM)
{
    if (node->to[c])
        return node->to[c];

    state_a nstates;
    arr_init(nstates);

    /* clean the hash */
    ++timeline;
    arr_for(state, node->states)
        if ((*state)->kind == NODE_CHAR)
            arr_for(range, (*state)->range)
                if (range->lower <= c && c <= range->upper) {
                    append_nnode((*state)->next, &nstates);
                    break;
                }

    if (nstates.len == 0)
        return NULL;

    /* qsort_node(nstates.v, nstates.v + nstates.len); */

    dnode_t **target = hash_find(FSM->hash, nstates);
    if (target) {
        node->to[c] = *target;
        arr_free(nstates);
        return *target;
    }

    dnode_t *p = mcalloc(1, sizeof(dnode_t));
    node->to[c] = p;
    p->states = nstates;
    handle_dnode(p, FSM);
    return p;
}

static dnode_t *strip_dnode(dnode_t *node, FSM_t *FSM)
{
    arr_for(state, node->states)
        if ((*state)->kind == NODE_ACCEPT) {
            dnode_t *p = mcalloc(1, sizeof(dnode_t));
            p->states.len = (size_t)(state - node->states.v);
            p->states.v = mmalloc(sizeof(void *) * p->states.len);
            memcpy(p->states.v, node->states.v, sizeof(void *) * p->states.len);

            dnode_t **target = hash_find(FSM->hash, p->states);
            if (target) {
                arr_free(p->states);
                return *target;
            }
            handle_dnode(p, FSM);
            return p;
        }
    assert(0);
    return NULL;
}

static void init_match(FSM_t *FSM)
{
    if (!FSM->hash) {
        FSM->hash = mmalloc(sizeof(hash_t));
        hash_init(FSM->hash);
    }
    if (!FSM->DFA) {
        FSM->DFA_size = 1;

        FSM->DFA = mcalloc(1, sizeof(dnode_t));
        arr_init(FSM->DFA->states);

        ++timeline;
        append_nnode(FSM->NFA, &FSM->DFA->states);
        /* qsort_node(FSM->DFA->states.v, */
        /*            FSM->DFA->states.v + FSM->DFA->states.len); */
        handle_dnode(FSM->DFA, FSM);
    }
}

/* compile current regular expression into a NFA graph */
extern void DFA_compile(vfrex_t vfrex)
{
    assert(vfrex->algorithm == REGEX_DFA);
    assert(vfrex->exp.len);

    switch (vfrex->option.match) {
    case REGEX_MATCH_FULL_BOOL:
        vfrex->FSM[0] = mcalloc(1, sizeof(FSM_t));
        build_NFA(vfrex, false, false, vfrex->FSM[0]);
        break;

    case REGEX_MATCH_PARTIAL_BOOL:
        vfrex->FSM[0] = mcalloc(1, sizeof(FSM_t));
        build_NFA(vfrex, false, true, vfrex->FSM[0]);
        break;

    case REGEX_MATCH_PARTIAL_BOUNDARY:
        vfrex->FSM[0] = mcalloc(1, sizeof(FSM_t));
        vfrex->FSM[1] = mcalloc(1, sizeof(FSM_t));
        build_NFA(vfrex, false, true, vfrex->FSM[0]);
        build_NFA(vfrex, true, false, vfrex->FSM[1]);
        break;

    case REGEX_MATCH_FULL_SUBMATCH:
    case REGEX_MATCH_PARTIAL_SUBMATCH:
        assert(0);
        break;
    }
    ++timeline;
}

extern bool DFA_match(const uchar *text, size_t len, vfrex_t vfrex)
{
    assert(vfrex->algorithm == REGEX_DFA);
    UNUSED(len);

    dnode_t *node;
    switch (vfrex->option.match) {
    case REGEX_MATCH_FULL_BOOL:
        init_match(vfrex->FSM[0]);
        node = vfrex->FSM[0]->DFA;
        debug_print_dnode(node);
        for (const uchar *c = text; *c; ++c) {
            node = next_dnode(node, *c, vfrex->FSM[0]);
            if (!node)
                return false;
            debug_print_dnode(node);
        }
        return node->is_accept;

    case REGEX_MATCH_PARTIAL_BOOL:
        init_match(vfrex->FSM[0]);
        node = vfrex->FSM[0]->DFA;
        if (node->is_accept)
            return true;
        debug_print_dnode(node);
        for (const uchar *c = text; *c; ++c) {
            node = next_dnode(node, *c, vfrex->FSM[0]);
            debug_print_dnode(node);
            if (node->is_accept)
                return true;
        }
        return false;

    case REGEX_MATCH_PARTIAL_BOUNDARY:
        init_match(vfrex->FSM[0]);
        node = vfrex->FSM[0]->DFA;

        const uchar *left, *right;
        bool found = false;

        debug_print_dnode(node);
        if (node->is_accept) {
            found = true;
            right = text;
            if (node->states.v[0]->kind == NODE_ACCEPT) {
                vfrex->group_number = 1;
                vfrex->group_left   = mmalloc(sizeof(void *));
                vfrex->group_right  = mmalloc(sizeof(void *));
                *vfrex->group_left  = text;
                *vfrex->group_right = text;
                return true;
            }
            node = strip_dnode(node, vfrex->FSM[0]);
            debug_print_dnode(node);
        }
        for (const uchar *c = text; *c; ++c) {
            node = next_dnode(node, *c, vfrex->FSM[0]);
            if (!node)
                break;
            debug_print_dnode(node);
            if (node->is_accept) {
                found = true;
                right = c+1;
                if (node->states.v[0]->kind == NODE_ACCEPT)
                    break;
                node = strip_dnode(node, vfrex->FSM[0]);
                debug_print_dnode(node);
            }
        }
        if (!found)
            return false;

#ifdef DEBUG
        puts("<><><><><><><>");
#endif
        init_match(vfrex->FSM[1]);
        node = vfrex->FSM[1]->DFA;
        found = false;

        if (node->is_accept) {
            found = true;
            left = right;
        }
        debug_print_dnode(node);
        for (const uchar *c = right-1; c >= text; --c) {
            node = next_dnode(node, *c, vfrex->FSM[1]);
            if (!node)
                break;
            debug_print_dnode(node);
            if (node->is_accept) {
                found = true;
                left = c;
            }
        }
        assert(found);

        vfrex->group_number = 1;
        vfrex->group_left   = mmalloc(sizeof(void *));
        vfrex->group_right  = mmalloc(sizeof(void *));
        *vfrex->group_left  = left;
        *vfrex->group_right = right;
        return true;

    case REGEX_MATCH_FULL_SUBMATCH:
    case REGEX_MATCH_PARTIAL_SUBMATCH:
        assert(0);
        break;
    }
    return false;
}

/* TODO */
extern void DFA_free(vfrex_t vfrex)
{
    /* TODO TOTALLy Wrong free to eliminate warning */
    if (vfrex->FSM[0]) {
        hash_free(vfrex->FSM[0]->hash);
        vfrex->FSM[0]->hash = NULL;
        cleanup(vfrex->FSM[0]);
    }
    if (vfrex->FSM[1]) {
        hash_free(vfrex->FSM[1]->hash);
        vfrex->FSM[1]->hash = NULL;
        cleanup(vfrex->FSM[1]);
    }
}

#ifdef DEBUG_MAIN

#include "parser.h"
#include "debug.h"

void test_partial(vfrex_t vfrex, char *str, bool ret, int left, int right)
{
    printf("\nTest case: %s\n", str);
    assert(DFA_match((uchar *)str, strlen(str), vfrex) == ret);
    if (ret) {
        assert(vfrex->group_number == 1);
        printf("%d\n", *vfrex->group_left - (uchar *)str + 1);
        assert(*vfrex->group_left  == (uchar *)str + left - 1);
        printf("%d\n", *vfrex->group_right - (uchar *)str);
        assert(*vfrex->group_right == (uchar *)str + right);
    }
}

int main(void)
{
    setbuf(stdout, NULL);
    struct vfrex_t vfrex;
    memset(&vfrex, 0, sizeof(vfrex));

    vfrex.regex = (uchar *)"a|abcd";
    vfrex.regex_len = strlen((char *)vfrex.regex);
    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_FULL_BOOL;

    int jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        assert( DFA_match((const uchar *)"a", 1, &vfrex));
        assert(!DFA_match((const uchar *)"abcc", 4, &vfrex));
        assert( DFA_match((const uchar *)"abcd", 4, &vfrex));
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }


    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_FULL_BOOL;
    vfrex.regex = (uchar *)"ab*|c";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        assert( DFA_match((const uchar *)"ab", 2, &vfrex));
        assert( DFA_match((const uchar *)"a", 1, &vfrex));
        assert( DFA_match((const uchar *)"c", 1, &vfrex));
        assert( DFA_match((const uchar *)"abbbb", 5, &vfrex));
        assert( DFA_match((const uchar *)"abbbbbbbbbb", 11, &vfrex));
        assert( DFA_match((const uchar *)"abbbbbbbbb", 10, &vfrex));
        assert( DFA_match((const uchar *)"abbbbbbbb", 9, &vfrex));
        assert(!DFA_match((const uchar *)"ca", 2, &vfrex));
        puts("==");
        assert(!DFA_match((const uchar *)"bbbbbbbb", 8, &vfrex));
        assert(!DFA_match((const uchar *)"ac", 2, &vfrex));
        assert(!DFA_match((const uchar *)"abbc", 4, &vfrex));
        assert(!DFA_match((const uchar *)"bbc", 3, &vfrex));
        assert(!DFA_match((const uchar *)"cb", 2, &vfrex));
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.regex = (uchar *)"a*b*";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "zhouyichao", true, 1, 0);
        test_partial(&vfrex, "zhouyichabo", true, 1, 0);
        test_partial(&vfrex, "fabbb", true, 1, 0);
        test_partial(&vfrex, "aaaaabbbbb", true, 1, 10);
        test_partial(&vfrex, "aaaaabxbbb", true, 1, 6);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.regex = (uchar *)"c*ab+c";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "ac", false, 1, 2);
        test_partial(&vfrex, "abcc", true, 1, 3);
        test_partial(&vfrex, "bac", false, 1, 0);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.regex = (uchar *)"(cabde)+|a.*";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "ffffcabdecabdekkkkkkkkk", true, 5, 14);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.regex = (uchar *)"(cabde)+|c.*";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "ffffcabdfcabdekkkkkkkkk", true, 5, 23);
        test_partial(&vfrex, "ffffCabdfCabdekkkkkkkkk", false, 5, 23);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.option.ignore_case = true;
    vfrex.regex = (uchar *)"(caBDe)+|C.*";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "FffFcaBdfCabDekKkkKkkKk", true, 5, 23);
        test_partial(&vfrex, "fFFFcABDFcABDEKKKKkkkkk", true, 5, 23);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }

    vfrex.option.style = REGEX_STYLE_POSIX;
    vfrex.option.match = REGEX_MATCH_PARTIAL_BOUNDARY;
    vfrex.option.ignore_case = true;
    vfrex.regex = (uchar *)"\\d*";
    vfrex.regex_len = strlen((char *)vfrex.regex);

    jmp = setjmp(env);
    if (0 == jmp) {
        parser_parse(&vfrex);
        DFA_compile(&vfrex);
        test_partial(&vfrex, "999", true, 1, 3);
        DFA_free(&vfrex);
    } else {
        printf("Runtime error %d\n", jmp);
    }
    return 0;
}
#endif
