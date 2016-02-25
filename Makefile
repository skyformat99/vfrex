SRCDIR   = src
TESTDIR  = test

BUILDIR  = build
BINDIR   = bin
DEPDIR   = dep
DIRS     = $(BUILDIR) $(BINDIR) $(DEPDIR)

SRCS     = common.c dfa.c parser.c vfrex.c substring.c
OBJS     = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
DEPS     = $(SRCS:$(SRCDIR)/%.c=$(DEPDIR)/%.d)
TESTSRCS = $(wildcard $(TESTDIR)/*.c)
TESTEXES = $(TESTSRCS:$(TESTDIR)/%.c=$(BINDIR)/%.exe)

CC       = gcc
CPP      = g++
LD       = ld
INCLUDE  = -Isrc
CFLAGS   = $(INCLUDE) -g -O0 -Wall -Wextra -Wconversion -Wno-sign-conversion -std=gnu99

.PHONY: all test clean
.DELETE_ON_ERROR:
%: makefile

all: $(TESTEXES) lib

clean:
	-rm -fR $(DIRS)

test: $(TESTEXES)
	@cd $(BINDIR) && \
	for %%d in ( $(TESTEXES:$(BINDIR)/%.exe=%.exe) ) do %%d

lib: $(SRCS:%.c=$(BINDIR)/%.o) $(SRCDIR)/vfrex.h
	cp $(SRCDIR)/vfrex.h $(BINDIR)
	cp $(SRCDIR)/vfrex-share.h $(BINDIR)
	cd $(BINDIR) && ar rcs libvfrex.a $(SRCS:%.c=%.o)

$(TESTEXES): $(TESTDIR)/unit-test.h | $(BINDIR)

$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) $(SRCDIR)/$*.c -c -o $@

$(BINDIR)/%.exe: $(SRCDIR)/%.c $(TESTDIR)/%.c
	$(CC) -I$(TESTDIR) $(CFLAGS) $(SRCDIR)/$*.c $(TESTDIR)/$*.c -lcunit -o $@

$(DIRS):
	mkdir $@

# include $(DEPS)
