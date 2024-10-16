#include "position.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "move.h"

int file_of(int square) {
	return square % 8;
}

int rank_of(int square) {
	return square / 8;
}

void fen_is_ok(const char *fen) {
	struct position pos = { 0 };
	

}

char *algebraic(char *str, int square) {
	if (square < 0 || square >= 64) {
		str[0] = '-';
		str[1] = '\0';
	}
	else {
		str[0] = file_of(square) + 'a';
		str[1] = rank_of(square) + '1';
		str[2] = '\0';
	}
	return str;
}

int square(const char *algebraic) {
	if (strlen(algebraic) < 2)
		return -1;

	const char file[] = "abcdefgh";
	const char rank[] = "12345678";

	char *f = strchr(file, algebraic[0]);
	char *r = strchr(rank, algebraic[1]);

	return f && r ? f - file + 8 * (r - rank) : -1;
}

void pos_from_fen(struct position *pos, const char *fen) {
	int sq = A8;
	const char *c;
	for (c = fen; *c != ' '; c++) {
		switch (*c) {
		case 'p':
			pos->mailbox[sq++] = (struct piece){ .type = PAWN, .color = BLACK };
			break;
		case 'n':
			pos->mailbox[sq++] = (struct piece){ .type = KNIGHT, .color = BLACK };
			break;
		case 'b':
			pos->mailbox[sq++] = (struct piece){ .type = BISHOP, .color = BLACK };
			break;
		case 'r':
			pos->mailbox[sq++] = (struct piece){ .type = ROOK, .color = BLACK };
			break;
		case 'q':
			pos->mailbox[sq++] = (struct piece){ .type = QUEEN, .color = BLACK };
			break;
		case 'k':
			pos->mailbox[sq++] = (struct piece){ .type = KING, .color = BLACK };
			break;
		case 'P':
			pos->mailbox[sq++] = (struct piece){ .type = PAWN, .color = WHITE };
			break;
		case 'N':
			pos->mailbox[sq++] = (struct piece){ .type = KNIGHT, .color = WHITE };
			break;
		case 'B':
			pos->mailbox[sq++] = (struct piece){ .type = BISHOP, .color = WHITE };
			break;
		case 'R':
			pos->mailbox[sq++] = (struct piece){ .type = ROOK, .color = WHITE };
			break;
		case 'Q':
			pos->mailbox[sq++] = (struct piece){ .type = QUEEN, .color = WHITE };
			break;
		case 'K':
			pos->mailbox[sq++] = (struct piece){ .type = KING, .color = WHITE };
			break;
		case '/':
			sq = 8 * (rank_of(sq) - 2);
			break;
		default:
			sq += *c - '0';
		}
	}

	c++;

	pos->turn = *c == 'w' ? WHITE : BLACK;

	c += 2;

	pos->K = strchr(c, 'K') != NULL;
	pos->Q = strchr(c, 'Q') != NULL;
	pos->k = strchr(c, 'k') != NULL;
	pos->q = strchr(c, 'q') != NULL;

	c = strchr(c, ' ');

	pos->en_passant = square(c + 1);
	if (pos->en_passant == -1)
		pos->en_passant = 0;

	c = strchr(c + 1, ' ');
	if (c == NULL)
		return;

	char *endptr;
	errno = 0;
	pos->halfmove = strtod(c + 1, &endptr);

	c = endptr;
	errno = 0;
	pos->fullmove = strtod(c, &endptr);
}

int en_passant_needed(const struct position *pos) {
	if (!pos->en_passant)
		return 0;

	struct move moves[MOVES_MAX];
	movegen(pos, moves, 0);
	for (int i = 0; !is_null(&moves[i]); i++)
		if (moves[i].flag)
			return 1;
	return 0;
}

char *pos_to_fen(char *fen, const struct position *pos) {
	int k = 0;
	int i = 56, j = 0;
	while (1) {
		if (pos->mailbox[i].type != EMPTY) {
			if (j)
				fen[k++] = " 12345678"[j];
			j = 0;
			fen[k++] = " PNBRQKpnbrqk"[pos->mailbox[i].type + 6 * (pos->mailbox[i].color == BLACK)];
		}
		else {
			j++;
		}
		i++;
		if (i == 8) {
			if (j)
				fen[k++] = " 12345678"[j];
			break;
		}
		if (i % 8 == 0)  {
			if (j)
				fen[k++] = " 12345678"[j];
			j = 0;
			fen[k++] = '/';
			i -= 16;
		}
	}
	fen[k++] = ' ';
	fen[k++] = pos->turn == WHITE ? 'w' : 'b';
	fen[k++] = ' ';
	if (pos->K)
		fen[k++] = 'K';
	if (pos->Q)
		fen[k++] = 'Q';
	if (pos->k)
		fen[k++] = 'k';
	if (pos->q)
		fen[k++] = 'q';
	if (pos->K || pos->Q || pos->k || pos->q)
		fen[k++] = ' ';
	algebraic(fen + k, en_passant_needed(pos) ? pos->en_passant : -1);
	if (fen[k] == '-')
		k++;
	else
		k += 2;
	fen[k++] = ' ';
	k += snprintf(fen + k, 32, "%d %d", pos->halfmove, pos->fullmove);
	fen[k] = '\0';
	return fen;
}
