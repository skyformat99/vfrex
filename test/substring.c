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
#include "substring.h"
#include "unit-test.h"

struct vfrex_t vfrex;

const char *pattern[] = {
    "",
    "A",
    "AB",
    "BA",
    "ABC",
    "BAB",
    "ABA",
    ",a",
    "f ",
    "ababc",
    "ABCDA",
    "BA",
    "CA",
    "BABAB",
    "world",
    "BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
    "\x01\x02\x03\x04",
    "\xC1\xC2\xC3\xC4",
};

const char *text[] = {
    "",
    "A",
    "AA",
    "AAA",
    "ABAB",
    "BABA",
    "BABAB",
    "world",
    "BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
    "BABAB",
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
    "CASDFASDFASDFASDFAAasdfasdfasdflj,zvnkhafkjlhsadflzxcvn"
        "ABABABABBABABABABABABABABABABABABABABABAC",
    "BB",
    "BC",
    "ABABAB",
    "BAB",
    "ABA",
    "ABABA",
    "he f llo world,",
    "hello world,",
    "babbya,"
    "\x01\x02\x03\x04",
    "\xC1\xC2\xC3\xC4",
    "\x13\xF1\x01\x02\x03\x04\xF5",
    "\xC1\xC2\xC3\xC4",
};

#define N_PATTERN (sizeof(pattern)/sizeof(void *))
#define N_TEXT    (sizeof(text)/sizeof(void *))

typedef void (*fcomp)(vfrex_t);
typedef bool (*func)(const uchar *, size_t, vfrex_t);

void judge(const char *text, const char *patt, fcomp compile, func run)
{
    size_t tlen = strlen(text);
    size_t plen = strlen(patt);

    /* result */
    char *pch = strstr(text, patt);

    /* test for cu */
    vfrex.regex = (uchar *)patt;
    vfrex.regex_len = plen;
    vfrex.group_number = 0;
    compile(&vfrex);
    CU_ASSERT(run((uchar *)text, tlen, &vfrex) == (pch != NULL));
    CU_ASSERT(vfrex.group_number == (pch != NULL));
    CU_ASSERT(vfrex.group_number == !!vfrex.group_number);
    if (vfrex.group_number)
        CU_ASSERT(*vfrex.group_left == (uchar *)pch);
}

void shift_or_32_1(void)
{
    judge("A", "A", shift_or_compile_32, shift_or_match_32);
}

void shift_or_32_2(void)
{
    judge("A", "BA", shift_or_compile_32, shift_or_match_32);
}

void shift_or_32_3(void)
{
    judge("AB", "BA", shift_or_compile_32, shift_or_match_32);
}

void shift_or_32_4(void)
{
    judge("AA", "BA", shift_or_compile_32, shift_or_match_32);
}

void shift_or_32_5(void)
{
    judge("AA", "AAA", shift_or_compile_32, shift_or_match_32);
}

void shift_or_32_fuzzy(void)
{
    for (size_t i = 0; i < N_PATTERN; ++i) {
        if (strlen(pattern[i]) <= 32) {
            for (size_t j = 0; j < N_TEXT; ++j) {
                judge(text[j], pattern[i],
                      shift_or_compile_32,
                      shift_or_match_32);
            }
        }
    }
}

void shift_or_64_fuzzy(void)
{
    for (size_t i = 0; i < N_PATTERN; ++i) {
        for (size_t j = 0; j < N_TEXT; ++j) {
            if (strlen(pattern[i]) <= 64) {
                judge(text[j], pattern[i],
                      shift_or_compile_64,
                      shift_or_match_64);
            }
        }
    }
}

void shift_or_BM_1(void)
{
    judge("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          "BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
          boyer_moore_compile, boyer_moore_match);
}

void shift_or_BM_2(void)
{
    judge(", ", "A", boyer_moore_compile, boyer_moore_match);
}

void shift_or_BM_fuzzy(void)
{
    for (size_t i = 0; i < N_PATTERN; ++i) {
        for (size_t j = 0; j < N_TEXT; ++j) {
            /* printf("t: %s, p: %s\n", text[j], pattern[i]); */
            judge(text[j], pattern[i],
                  boyer_moore_compile,
                  boyer_moore_match);
        }
    }
}

int main()
{
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("shift_or_32", NULL, NULL);
    CU_ADD_TEST(pSuite, shift_or_32_1);
    CU_ADD_TEST(pSuite, shift_or_32_2);
    CU_ADD_TEST(pSuite, shift_or_32_3);
    CU_ADD_TEST(pSuite, shift_or_32_4);
    CU_ADD_TEST(pSuite, shift_or_32_5);
    CU_ADD_TEST(pSuite, shift_or_32_fuzzy);
    pSuite = CU_add_suite("shift_or_64", NULL, NULL);
    CU_ADD_TEST(pSuite, shift_or_64_fuzzy);
    pSuite = CU_add_suite("BM", NULL, NULL);
    CU_ADD_TEST(pSuite, shift_or_BM_1);
    CU_ADD_TEST(pSuite, shift_or_BM_2);
    CU_ADD_TEST(pSuite, shift_or_BM_fuzzy);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

/* }}} */
