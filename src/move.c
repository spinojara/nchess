#include "move.h"

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "position.h"
#include "window.h"

void new_move(struct move *move, int from, int to, int en_passant, int promotion) {
	move->from = from;
	move->to = to;
	move->flag = en_passant;
	move->promotion = promotion;
	move->captured = (struct piece){ 0 };
}

struct move *movegen_pawn(const struct position *pos, struct move *moves, int sq, int c) {
	if (c == WHITE) {
		if (pos->mailbox[sq + 8].type == EMPTY) {
			if (sq < A7) {
				new_move(moves++, sq, sq + 8, 0, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq + 8, 0, piece);
			}
			if (A2 <= sq && sq <= H2 && pos->mailbox[sq + 16].type == EMPTY)
				new_move(moves++, sq, sq + 16, 0, 0);
		}
		if (file_of(sq) != 0 && ((pos->mailbox[sq + 7].type != EMPTY && pos->mailbox[sq + 7].color != c) || (pos->en_passant && pos->en_passant == sq + 7))) {
			if (sq < A7) {
				new_move(moves++, sq, sq + 7, pos->en_passant && pos->en_passant == sq + 7, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq + 7, 0, piece);
			}
		}
		if (file_of(sq) != 7 && ((pos->mailbox[sq + 9].type != EMPTY && pos->mailbox[sq + 9].color != c) || (pos->en_passant && pos->en_passant == sq + 9))) {
			if (sq < A7) {
				new_move(moves++, sq, sq + 9, pos->en_passant && pos->en_passant == sq + 9, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq + 9, 0, piece);
			}
		}
	}
	else {
		if (pos->mailbox[sq - 8].type == EMPTY) {
			if (sq > H2) {
				new_move(moves++, sq, sq - 8, 0, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq - 8, 0, piece);
			}
			if (A7 <= sq && sq <= H7 && pos->mailbox[sq - 16].type == EMPTY)
				new_move(moves++, sq, sq - 16, 0, 0);
		}
		if (file_of(sq) != 7 && ((pos->mailbox[sq - 7].type != EMPTY && pos->mailbox[sq - 7].color != c) || (pos->en_passant && pos->en_passant == sq - 7))) {
			if (sq > H2) {
				new_move(moves++, sq, sq - 7, pos->en_passant && pos->en_passant == sq - 7, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq - 7, 0, piece);
			}
		}
		if (file_of(sq) != 0 && ((pos->mailbox[sq - 9].type != EMPTY && pos->mailbox[sq - 9].color != c) || (pos->en_passant && pos->en_passant == sq - 9))) {
			if (sq > H2) {
				new_move(moves++, sq, sq - 9, pos->en_passant && pos->en_passant == sq - 9, 0);
			}
			else {
				for (int piece = KNIGHT; piece <= QUEEN; piece++)
					new_move(moves++, sq, sq - 9, 0, piece);
			}
		}
	}
	return moves;
}

struct move *movegen_knight(const struct position *pos, struct move *moves, int sq, int c) {
	int dx[8] = { -2, -2, -1, -1, 1, 1, 2, 2};
	int dy[8] = { -1, 1, -2, 2, -2, 2, -1, 1};
	for (int i = 0; i < 8; i++) {
		int x = file_of(sq) + dx[i];
		int y = rank_of(sq) + dy[i];
		if (x < 0 || y < 0 || x >= 8 || y >= 8)
			continue;
		if (pos->mailbox[x + 8 * y].type == EMPTY || pos->mailbox[x + 8 * y].color != c)
			new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	return moves;
}

struct move *movegen_bishop(const struct position *pos, struct move *moves, int sq, int c) {
	for (int x = file_of(sq) - 1, y = rank_of(sq) - 1, flag = 0; x >= 0 && y >= 0 && !flag; x--, y--) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq) + 1, y = rank_of(sq) - 1, flag = 0; x < 8 && y >= 0 && !flag; x++, y--) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq) - 1, y = rank_of(sq) + 1, flag = 0; x >= 0 && y < 8 && !flag; x--, y++) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq) + 1, y = rank_of(sq) + 1, flag = 0; x < 8 && y < 8 && !flag; x++, y++) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	return moves;
}

struct move *movegen_rook(const struct position *pos, struct move *moves, int sq, int c) {
	for (int x = file_of(sq) - 1, y = rank_of(sq), flag = 0; x >= 0 && !flag; x--) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq), y = rank_of(sq) - 1, flag = 0; y >= 0 && !flag; y--) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq) + 1, y = rank_of(sq), flag = 0; x < 8 && !flag; x++) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	for (int x = file_of(sq), y = rank_of(sq) + 1, flag = 0; y < 8 && !flag; y++) {
		if (pos->mailbox[x + 8 * y].type != EMPTY) {
			if (pos->mailbox[x + 8 * y].color == c)
				break;
			else
				flag = 1;
		}
		new_move(moves++, sq, x + 8 * y, 0, 0);
	}
	return moves;
}

struct move *movegen_king(const struct position *pos, struct move *moves, int sq, int c) {
	for (int x = file_of(sq) - 1; x <= file_of(sq) + 1; x++) {
		for (int y = rank_of(sq) - 1; y <= rank_of(sq) + 1; y++) {
			if (x == file_of(sq) && y == rank_of(sq))
				continue;
			if (x < 0 || y < 0 || x >= 8 || y >= 8)
				continue;
			if (pos->mailbox[x + 8 * y].type == EMPTY || pos->mailbox[x + 8 * y].color != c)
				new_move(moves++, sq, x + 8 * y, 0, 0);
		}
	}
	if (c == WHITE) {
		if (pos->K && pos->mailbox[F1].type == EMPTY && pos->mailbox[G1].type == EMPTY) {
			struct move m[MOVES_MAX];
			struct position copy = *pos;
			copy.turn = BLACK;
			/* Place piece here so that pawns can capture them. */
			copy.mailbox[F1] = (struct piece){ .type = ROOK, .color = WHITE };
			movegen(&copy, m, 1);
			int flag = 0;
			for (struct move *move = m; !is_null(move); move++)
				if (move->to == E1 || move->to == F1)
					flag = 1;
			if (!flag)
				new_move(moves++, E1, G1, 0, 0);
		}
		if (pos->Q && pos->mailbox[B1].type == EMPTY && pos->mailbox[C1].type == EMPTY && pos->mailbox[D1].type == EMPTY) {
			struct move m[MOVES_MAX];
			struct position copy = *pos;
			copy.turn = BLACK;
			/* Place piece here so that pawns can capture them. */
			copy.mailbox[D1] = (struct piece){ .type = ROOK, .color = WHITE };
			movegen(&copy, m, 1);
			int flag = 0;
			for (struct move *move = m; !is_null(move); move++)
				if (move->to == E1 || move->to == D1)
					flag = 1;
			if (!flag)
				new_move(moves++, E1, C1, 0, 0);
		}
	}
	else {
		if (pos->k && pos->mailbox[F8].type == EMPTY && pos->mailbox[G8].type == EMPTY) {
			struct move m[MOVES_MAX];
			struct position copy = *pos;
			copy.turn = WHITE;
			/* Place piece here so that pawns can capture them. */
			copy.mailbox[F8] = (struct piece){ .type = ROOK, .color = BLACK };
			movegen(&copy, m, 1);
			int flag = 0;
			for (struct move *move = m; !is_null(move); move++)
				if (move->to == E8 || move->to == F8)
					flag = 1;
			if (!flag)
				new_move(moves++, E8, G8, 0, 0);
		}
		if (pos->q && pos->mailbox[B8].type == EMPTY && pos->mailbox[C8].type == EMPTY && pos->mailbox[D8].type == EMPTY) {
			struct move m[MOVES_MAX];
			struct position copy = *pos;
			copy.turn = WHITE;
			/* Place piece here so that pawns can capture them. */
			copy.mailbox[D8] = (struct piece){ .type = ROOK, .color = BLACK };
			movegen(&copy, m, 1);
			int flag = 0;
			for (struct move *move = m; !is_null(move); move++)
				if (move->to == E8 || move->to == D8)
					flag = 1;
			if (!flag)
				new_move(moves++, E8, C8, 0, 0);
		}
	}
	return moves;
}

int legal(const struct position *pos, struct move *move) {
	struct position copy = *pos;
	do_move(&copy, move);
	int king_sq;
	for (king_sq = 0; king_sq < 64; king_sq++)
		if (copy.mailbox[king_sq].type == KING && copy.mailbox[king_sq].color == !copy.turn)
			break;
	struct move moves[MOVES_MAX];
	movegen(&copy, moves, 1);
	for (move = moves; !is_null(move); move++)
		if (move->to == king_sq)
			return 0;

	return 1;
}

void movegen(const struct position *pos, struct move *moves, int pseudo_legal) {
	struct move pseudo[MOVES_MAX] = { 0 };
	struct move *move = pseudo;
	for (int sq = A1; sq <= H8; sq++) {
		int c = pos->mailbox[sq].color;
		int t = pos->mailbox[sq].type;
		if (t == EMPTY || c != pos->turn)
			continue;
		switch (t) {
		case EMPTY:
			break;
		case PAWN:
			move = movegen_pawn(pos, move, sq, c);
			break;
		case KNIGHT:
			move = movegen_knight(pos, move, sq, c);
			break;
		case BISHOP:
			move = movegen_bishop(pos, move, sq, c);
			break;
		case ROOK:
			move = movegen_rook(pos, move, sq, c);
			break;
		case QUEEN:
			move = movegen_bishop(pos, move, sq, c);
			move = movegen_rook(pos, move, sq, c);
			break;
		case KING:
			move = movegen_king(pos, move, sq, c);
			break;
		}
	}
	new_move(move, 0, 0, 0, 0);

	for (move = pseudo; !is_null(move); move++)
		if (pseudo_legal || legal(pos, move))
			*(moves++) = *move;

	new_move(moves, 0, 0, 0, 0);
}

int movecount(const struct move *moves) {
	int count;
	for (count = 0; !is_null(&moves[count]); count++);
	return count;
}

void do_move(struct position *pos, struct move *move) {
	/* Store information before making move. */
	move->en_passant = pos->en_passant;
	move->K = pos->K;
	move->Q = pos->Q;
	move->k = pos->k;
	move->q = pos->q;
	move->halfmove = pos->halfmove;

	move->captured = pos->mailbox[move->to];

	pos->halfmove++;

	if (pos->mailbox[move->to].type != EMPTY)
		pos->halfmove = 0;

	if (move->flag) {
		/* Can clear both squares, because one of them will be empty anyway. */
		pos->mailbox[pos->en_passant - 8].type = EMPTY;
		pos->mailbox[pos->en_passant + 8].type = EMPTY;
	}
	pos->en_passant = 0;
	if (pos->mailbox[move->from].type == PAWN) {
		pos->halfmove = 0;
		if (move->from + 16 == move->to)
			pos->en_passant = move->from + 8;
		if (move->from - 16 == move->to)
			pos->en_passant = move->from - 8;
	}

	pos->mailbox[move->to] = pos->mailbox[move->from];
	pos->mailbox[move->from].type = EMPTY;

	if (move->promotion)
		pos->mailbox[move->to].type = move->promotion;

	if (pos->K && move->from == E1 && move->to == G1) {
		pos->mailbox[F1] = pos->mailbox[H1];
		pos->mailbox[H1].type = EMPTY;
	}
	if (pos->Q && move->from == E1 && move->to == C1) {
		pos->mailbox[D1] = pos->mailbox[A1];
		pos->mailbox[A1].type = EMPTY;
	}
	if (pos->k && move->from == E8 && move->to == G8) {
		pos->mailbox[F8] = pos->mailbox[H8];
		pos->mailbox[H8].type = EMPTY;
	}
	if (pos->q && move->from == E8 && move->to == C8) {
		pos->mailbox[D8] = pos->mailbox[A8];
		pos->mailbox[A8].type = EMPTY;
	}

	if (move->from == E1 || move->from == H1 || move->to == H1)
		pos->K = 0;
	if (move->from == E1 || move->from == A1 || move->to == A1)
		pos->Q = 0;
	if (move->from == E8 || move->from == H8 || move->to == H8)
		pos->k = 0;
	if (move->from == E8 || move->from == A8 || move->to == A8)
		pos->q = 0;

	if (!pos->turn)
		pos->fullmove++;
	pos->turn = !pos->turn;
}

void undo_move(struct position *pos, const struct move *move) {
	pos->en_passant = move->en_passant;
	pos->K = move->K;
	pos->Q = move->Q;
	pos->k = move->k;
	pos->q = move->q;
	pos->halfmove = move->halfmove;

	pos->mailbox[move->from] = pos->mailbox[move->to];
	pos->mailbox[move->to] = move->captured;

	if (move->flag)
		pos->mailbox[pos->en_passant + (pos->turn == WHITE ? 8 : -8)] = (struct piece){ .type = PAWN, .color = pos->turn };

	if (move->promotion)
		pos->mailbox[move->from].type = PAWN;

	if (pos->K && move->from == E1 && move->to == G1) {
		pos->mailbox[H1] = pos->mailbox[F1];
		pos->mailbox[F1].type = EMPTY;
	}
	if (pos->Q && move->from == E1 && move->to == C1) {
		pos->mailbox[A1] = pos->mailbox[D1];
		pos->mailbox[D1].type = EMPTY;
	}
	if (pos->k && move->from == E8 && move->to == G8) {
		pos->mailbox[H8] = pos->mailbox[F8];
		pos->mailbox[F8].type = EMPTY;
	}
	if (pos->q && move->from == E8 && move->to == C8) {
		pos->mailbox[A8] = pos->mailbox[D8];
		pos->mailbox[D8].type = EMPTY;
	}

	if (pos->turn)
		pos->fullmove--;
	pos->turn = !pos->turn;
}

void print_move(struct move *move) {
	char str1[3], str2[3];
	printf("%s%s", algebraic(str1, move->from), algebraic(str2, move->to));
	if (move->promotion)
		printf("%c", "nbrq"[move->promotion - KNIGHT]);
}

int is_null(const struct move *move) {
	return move->from == move->to;
}

long perft(struct position *pos, int depth, int verbose) {
	if (depth <= 0)
		return 0;

	struct move moves[MOVES_MAX];
	movegen(pos, moves, 0);

	uint64_t nodes = 0, count;
	for (struct move *move = moves; !is_null(move); move++) {
		if (depth == 1) {
			count = 1;
			nodes++;
		}
		else {
			do_move(pos, move);
			count = perft(pos, depth - 1, 0);
			undo_move(pos, move);
			nodes += count;
		}
		if (verbose) {
			print_move(move);
			printf(": %ld\n", count);
		}
	}
	return nodes;
}

char *move_pgn(char *str, const struct position *pos, const struct move *move) {
	int i = 0;
	int f = file_of(move->from);
	int r = rank_of(move->from);
	int piece = pos->mailbox[move->from].type;
	switch (piece) {
	case PAWN:
		break;
	case KNIGHT:
		str[i++] = 'N';
		break;
	case BISHOP:
		str[i++] = 'B';
		break;
	case ROOK:
		str[i++] = 'R';
		break;
	case QUEEN:
		str[i++] = 'Q';
		break;
	case KING:
		if (f == 4 && file_of(move->to) == 6) {
			sprintf(str, "O-O");
			i = 3;
		}
		else if (f == 4 && file_of(move->to) == 2) {
			sprintf(str, "O-O-O");
			i = 5;
		}
		else {
			str[i++] = 'K';
		}
		break;
	}
	struct move moves[MOVES_MAX];
	movegen(pos, moves, 0);
	int need_file = 0;
	int need_rank = 0;
	int need = 0;
	for (int j = 0; !is_null(&moves[j]); j++) {
		if (moves[j].from == move->from || moves[j].to != move->to || pos->mailbox[moves[j].from].type != piece)
			continue;

		if (file_of(moves[j].from) == f)
			need_rank = 1;
		else if (rank_of(moves[j].to) == r)
			need_file = 1;
		need = 1;
	}
	if (need && !need_rank)
		need_file = 1;
	if (need_file)
		str[i++] = 'a' + f;
	if (need_rank)
		str[i++] = '1' + r;

	/* Capture, or en passant. */
	if (pos->mailbox[move->to].type != EMPTY || move->flag) {
		if (piece == PAWN)
			str[i++] = 'a' + f;
		str[i++] = 'x';
	}
	if (str[0] != 'O') {
		algebraic(str + i, move->to);
		i += 2;
	}
	if (move->promotion) {
		str[i++] = '=';
		str[i++] = "??NBRQ"[move->promotion];
	}

	struct position posc = *pos;
	struct move movec = *move;
	do_move(&posc, &movec);
	movegen(&posc, moves, 0);
	int mate = is_null(moves);
	posc.turn = !posc.turn;
	movegen(&posc, moves, 1);
	int check = 0;
	for (int j = 0; !is_null(&moves[j]) && !check; j++)
		if (posc.mailbox[moves[j].to].type == KING)
			check = 1;

	if (check && mate)
		str[i++] = '#';
	else if (check)
		str[i++] = '+';
	str[i] = '\0';
	return str;
}

char *move_algebraic(char *str, const struct move *m) {
	algebraic(str, m->from);
	algebraic(str + 2, m->to);

	if (m->promotion) {
		str[4] = "??NBRQ"[m->promotion];
		str[5] = '\0';
	}

	return str;
}

int movecmp(const struct move *a, const struct move *b) {
	return a->from != b->from || a->to != b->to || a->flag != b->flag || a->promotion != b->promotion;
}

struct move *string_to_move(struct move *move, struct position *pos, const char *str) {
	if (!str)
		return NULL;
	struct move moves[MOVES_MAX];
	movegen(pos, moves, 0);
	char s[6];
	for (int i = 0; !is_null(&moves[i]); i++) {
		if (!strcmp(move_algebraic(s, &moves[i]), str)) {
			*move = moves[i];
			return move;
		}
	}

	return NULL;
}
