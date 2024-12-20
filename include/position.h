#ifndef POSITION_H
#define POSITION_H

enum {
	BLACK, WHITE,
};

enum {
	EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
};

enum {
	STATUS_NOTOVER,
	STATUS_CHECKMATE,
	STATUS_STALEMATE,
	STATUS_HALFMOVE,
	STATUS_THREEFOLD,
	STATUS_ILLEGALMOVE,
	STATUS_DISCONNECT,
	STATUS_TIME,
};

struct piece {
	int type;
	int color;
};

struct position {
	struct piece mailbox[64];
	int turn;
	int K, Q, k, q;

	int en_passant;

	int halfmove, fullmove;
};

enum {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
};

int file_of(int square);

int rank_of(int square);

int fen_is_ok(const char *fen);

struct position *pos_from_fen(struct position *pos, const char *fen);

char *pos_to_fen(char *fen, const struct position *pos);

char *algebraic(char *str, int square);

int square(const char *algebraic); 

int make_legal(struct position *pos);

int is_mate(const struct position *pos);

#endif
