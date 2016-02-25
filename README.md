vfrex (a very fast regex engine X)
=====

A fast regex library in pure C.  This project implements a variety of string/regex matching
algorithm.  It currently supports all the POSIX symbol, including
* `+?*()`: the most common symbol of regex;
* `.\s\d\x\o\w\h\a\l\u`: some built-in charset.

It does not support:
* `[]`, `[^]`: custom charset;
* zero-width assertion
* all international characters are treated as ASCII.  Therefore it can support UTF-8 well but not
  Unicode because it contains "\0"

It also contains two grep-like utilities programs so that you can play with the regex engine!

Implemented algorithms
----------------------
* SHIFT-OR-32: a super fast string matching algorithm using bit arithmetic.  Automatically enabled
  when the input regex is pure string and its length is smaller than 32
* SHIFT-OR-64: a super fast string matching algorithm using bit arithmetic (uses int64).  Disabled
  by default.
* Boyer Moore: the state-of-the-art general string matching algorithm.  Sub-linear on random
  string.
* DFA/NFA: construct DFA from NFA on the fly to match the regex.  The position of the matching can
  be returned.

Interesting part
----------------
The generic programming using C programming language!  See `list.h`, `array.h`, `hash.h`, `qsort.h`.

PS:  qsort is hard to optimize! After spending one day optimizing it, I finally got a similar
performance as sort in STL!
