/* The MIT License (MITs)
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

#include "common.h"

/* this routine is mainly used for debug */
char *operator_to_str(operator_t x)
{
#ifndef NDEBUG
    switch (x) {
    case REGEX_CHAR:
        return "REGEX_CHAR";
    case REGEX_BOL:
        return "REGEX_BOL";
    case REGEX_EOL:
        return "REGEX_EOL";
    case REGEX_EOF:
        return "REGEX_EOF";
    case REGEX_NOTHING:
        return "REGEX_NOTHING";
    case REGEX_CHARSET:
        return "REGEX_CHARSET";
    case REGEX_CONCATE:
        return "REGEX_CONCATE";
    case REGEX_OR:
        return "REGEX_OR";
    case REGEX_ZERO_ONE:
        return "REGEX_ZERO_ONE";
    case REGEX_REPEAT:
        return "REGEX_REPEAT";
    case REGEX_REPEAT_ALO:
        return "REGEX_REPEAT_ALO";
    case REGEX_ZERO_ONE_NG:
        return "REGEX_ZERO_ONE_NG";
    case REGEX_REPEAT_NG:
        return "REGEX_REPEAT_NG";
    case REGEX_REPEAT_ALO_NG:
        return "REGEX_REPEAT_ALO_NG";
    case REGEX_START:
        return "REGEX_START";
    case REGEX_END:
        return "REGEX_END";
    case REGEX_REGEX_START:
        return "REGEX_REGEX_START";
    case REGEX_REGEX_END:
        return "REGEX_REGEX_END";
    case REGEX_PARENT_LEFT:
        return "REGEX_PARENT_LEFT";
    case REGEX_PARENT_RIGHT:
        return "REGEX_PARENT_RIGHT";
    case REGEX_WORD_BOUNDARY:
        return "REGEX_WORD_BOUNDARY";
    case REGEX_WORD_BOUNDARY_LEFT:
        return "REGEX_WORD_BOUNDARY_LEFT";
    case REGEX_WORD_BOUNDARY_RIGHT:
        return "REGEX_WORD_BOUNDARY_RIGHT";
    }
#endif
    return "";
}

/* this routine is mainly used for debug */
char *algorithm_to_str(algorithm_t x)
{
#ifndef NDEBUG
    switch (x) {
    case REGEX_SHIFT_OR_32:
        return "REGEX_SHIFT_OR_32";
    case REGEX_SHIFT_OR_64:
        return "REGEX_SHIFT_OR_64";
    case REGEX_BOYER_MOORE:
        return "REGEX_BOYER_MOORE";
    case REGEX_DFA:
        return "REGEX_DFA";
    case REGEX_NFA:
        return "REGEX_NFA";
    }
#endif
    return "";
}
