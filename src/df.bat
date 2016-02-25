gcc -std=gnu99 -DDEBUG -Wall -Wextra -Wconversion -g -c parser.c common.c
gcc -std=gnu99 -DDEBUG -DDEBUG_MAIN -Wall -Wextra -Wconversion -g -o dfa parser.o common.o dfa.c
