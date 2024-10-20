#ifndef MOVE_H
#define MOVE_H

#include "position.h"

#define MOVES_MAX 256

struct move {
	int from;
	int to;

	int flag;
	int promotion;

	struct piece captured;
	int en_passant;
	int halfmove;
	int K, Q, k, q;
};

void new_move(struct move *move, int from, int to, int en_passant, int promotion);

void do_move(struct position *pos, struct move *move);

void undo_move(struct position *pos, const struct move *move);

int is_null(const struct move *move);

int movecount(const struct move *moves);

void movegen(const struct position *pos, struct move *moves, int pseudo_legal);

long perft(struct position *pos, int depth, int verbose);

char *move_pgn(char *str, const struct position *pos, const struct move *move);

int movecmp(const struct move *a, const struct move *b);

struct move *string_to_move(struct move *move, struct position *pos, const char *str);

#endif
