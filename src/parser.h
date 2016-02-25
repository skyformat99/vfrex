#ifndef __PARSER_H
#define __PARSER_H

#include "common.h"
#include <setjmp.h>

#define MAX_INPUT 100000
#define MAX_STACK 100000

extern jmp_buf env;

/* Turn the regular expression into the Reverse Polish.  Notice that input
 * string should be utf-8 compatible */
extern void parser_parse(vfrex_t vfrex);

#endif
