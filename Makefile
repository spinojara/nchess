all:
	gcc -Wall -Wextra -pedantic -Iinclude -fsanitize=address,undefined -DPOSIX_C_SOURCE=200112L -std=c99 $(shell pkg-config --cflags ncurses) -O2 src/*.c $(shell pkg-config --libs ncurses)
