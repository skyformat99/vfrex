gcc -std=gnu99 -DDEBUG -Wall -Wextra -Wconversion -Wno-sign-conversion -g -c common.c dfa.c parser.c substring.c
gcc -std=gnu99 -DDEBUG -DDEBUG_MAIN -Wall -Wextra -Wconversion -Wno-sign-conversion -g vfrex.c common.o dfa.o parser.o substring.o
