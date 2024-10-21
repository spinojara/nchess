MKDIR_P   = mkdir -p
RM        = rm
INSTALL   = install

CC        = cc
CSTANDARD = -std=c99 -DPOSIX_C_SOURCE=200112L
CWARNINGS = -Wall -Wextra -Wshadow -pedantic -Wvla
COPTIMIZE = -O2

ifeq ($(DEBUG), yes)
	CDEBUG = -g3 -ggdb
else ifeq ($(DEBUG), thread)
	CDEBUG = -g3 -ggdb -fsanitize=thread,undefined
else ifeq ($(DEBUG), address)
	CDEBUG = -g3 -ggdb -fsanitize=address,undefined
else ifeq ($(DEBUG), )
	CDEBUG = -DNDEBUG
endif

CFLAGS  ::= $(CSTANDARD) $(CWARNINGS) $(COPTIMIZE) $(CDEBUG) $(shell pkg-config --cflags ncurses) -Iinclude
LDLIBS  ::= $(shell pkg-config --libs ncurses)
LDFLAGS ::= $(CFLAGS) $(LDLIBS)

SRC       = analysis.c board.c color.c draw.c editengine.c editwin.c \
	    engine.c engines.c field.c info.c mainwin.c move.c nchess.c \
	    position.c settings.c topbar.c window.c

OBJ       = $(patsubst %.c,obj/%.o,$(SRC))

PREFIX    = /usr/local
BINDIR    = $(PREFIX)/bin

ifneq ($(STATIC), )
	LDFLAGS += -static
endif

all: nchess

nchess: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

obj/%.o: src/%.c Makefile
	@$(MKDIR_P) obj
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	$(MKDIR_P) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 nchess $(DESTDIR)$(BINDIR)

uninstall:
	$(RM) -f $(DESTDIR)$(BINDIR)/nchess

clean:
	$(RM) -rf obj nchess

.SUFFIXES: .c .h
.PHONY: all clean install uninstall
