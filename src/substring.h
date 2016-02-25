#ifndef __SUBSTRING__H
#define __SUBSTRING__H

#include "common.h"

void shift_or_compile_32(vfrex_t vfrex);
void shift_or_compile_64(vfrex_t vfrex);

bool shift_or_match_32(const uchar *text, size_t len, vfrex_t vfrex);
bool shift_or_match_64(const uchar *text, size_t len, vfrex_t vfrex);

void boyer_moore_compile(vfrex_t vfrex);
bool boyer_moore_match(const uchar *text, size_t len, vfrex_t vfrex);

#endif
