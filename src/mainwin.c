#include "mainwin.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "window.h"
#include "draw.h"
#include "color.h"
#include "position.h"
#include "move.h"
#include "board.h"
#include "engine.h"
#include "field.h"
#include "info.h"

#define MAXPASTINFO 17
#define MAXINFOPV   32
#define MOVESMAXLINES 40

struct uciinfo {
	long long depth;
	long long seldepth;
	long long t;
	long long nodes;
	long long cp;
	int lowerbound;
	int upperbound;
	long long mate;
	char pv[MAXINFOPV][8];
};

static int refreshed = 0;

static int selectedsquare = -1;

int flipped = 0;

/* Displayed position. */
struct position posd;
/* Actual position. */
static struct position posa;

static int smove = 0;
static int shownmove = 0;
static int selectedmove = -1;
static int nmove = 0;

static int fenselected = 0;
static struct field fen;

int hideengineoutput = 0;
int autoflip = 1;
int relativescore = 0;

static int sentdepth = 0;
static int sentseldepth = 0;
static int senttime = 0;
static int sentnodes = 0;
static int sentpv = 0;
static int sentscore = 0;
static int sentnps = 0;
static int senthashfull = 0;
static int senttbhits = 0;
static int sentcpuload = 0;
static int sentcurrmove = 0;
static int sentcurrmovenumber = 0;

static long long nps;
static long long hashfull;
static long long tbhits;
static long long cpuload;
static char currmove[8];
static long long currmovenumber;

static int npastinfo = 0;
static struct uciinfo pastinfo[MAXPASTINFO] = { 0 };

static struct {
	struct move move;
	char name[8];
	int color;
	int fullmove;
} *vmove = NULL;

int gamerunning = 0;

static struct engineconnection *analysisengine = NULL;
struct timecontrol tc[2] = { 0 };
static int sentwhite = 0, sentblack = 0;
static struct engineconnection *whiteengine = NULL;
static struct engineconnection *blackengine = NULL;

void mainwin_draw(void);
void put_move(struct move *move, int at_end);
int backward_move(int dontreset);
void backward_full(int dontreset);
int forward_move(int dontreset);
void forward_full(int dontreset);
int prompt_promotion(int square);
void reset_analysis(void);
int is_threefold(int displayed);
int subtract_timecontrol(struct timecontrol *timecontrol, timepoint_t start, timepoint_t end);

void mainwin_event(chtype ch, MEVENT *event) {
	if (analysisengine && engine_error(analysisengine))
		end_analysis();

	if (ch != KEY_MOUSE && ch != 0)
		selectedsquare = -1;

	if (fenselected && ch != 0 && !gamerunning) {
		refreshed = 0;
		if (ch != '\n' && ch != 0 && ch != KEY_ESC) {
			field_driver(&fen, ch, event);
			goto draw;
		}
		else if (ch == '\n') {
			struct position new;
			const char *buf = field_buffer(&fen, 0);
			if (fen_is_ok(buf)) {
				pos_from_fen(&new, buf);
				set_position(&new);
				fenselected = 0;
				fen.error = 0;
			}
			else {
				fen.error = 1;
			}
		}
		else if (ch == KEY_ESC) {
			fenselected = 0;
			fen.error = 0;
		}
	}

	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case 'u':
		backward_move(0);
		refreshed = 0;
		break;
	case 'f':
		forward_move(0);
		refreshed = 0;
		break;
	case 'q':
		running = 0;
		break;
	case 'k':
	case KEY_UP:
		place_top(&topbar);
		break;
	case KEY_MOUSE:
		if (event->bstate & BUTTON1_PRESSED) {
			/* Board. */
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && (!gamerunning || (selectedmove == nmove - 1 && ((sentwhite && !whiteengine) || (sentblack && !blackengine))))) {
				int new = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (flipped)
					new = 63 - new;
				if (posd.mailbox[new].type != EMPTY && posd.mailbox[new].color == posd.turn)
					selectedsquare = new;
				refreshed = 0;
			}
			/* Forward and backward. */
			else if (event->y == 5 * 8 + 3 && 86 <= event->x && event->x <= 89) {
				backward_full(0);
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 91 <= event->x && event->x <= 93) {
				backward_move(0);
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 96 <= event->x && event->x <= 98) {
				forward_move(0);
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 100 <= event->x && event->x <= 103) {
				forward_full(0);
				refreshed = 0;
			}
			else if (1 <= event->y && event->y <= 40 && 90 <= event->x && event->x != 97 && event->x <= 105 && nmove > 0) {
				int line = event->y - 1;
				int index = event->x > 96;
				int offset = vmove[0].color == BLACK;
				int moveindex = 2 * line + index + shownmove - offset;
				int oldshownmove = shownmove;
				if (0 <= moveindex && moveindex < nmove) {
					backward_full(1);
					for (int i = 0; i < moveindex + 1; i++)
						forward_move(1);
					reset_analysis();
				}
				shownmove = oldshownmove;
				refreshed = 0;
			}
			else if (event->y == 43 && 2 <= event->x && event->x < 2 + 78 && !gamerunning) {
				fenselected = 1;
				field_driver(&fen, ch, event);
				refreshed = 0;
			}
		}
		if (event->bstate & BUTTON1_RELEASED) {
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && selectedsquare != -1 && (!gamerunning || (selectedmove == nmove - 1 && ((sentwhite && !whiteengine) || (sentblack && !blackengine))))) {
				struct move move;
				int to = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (flipped)
					to = 63 - to;
				if (to == selectedsquare)
					break;
				new_move(&move, selectedsquare, to, 0, 0);
				struct move moves[MOVES_MAX];
				movegen(&posd, moves, 0);
				for (int i = 0; !is_null(&moves[i]); i++) {
					if (moves[i].to == move.to && moves[i].from == move.from) {
						if (!moves[i].promotion || (moves[i].promotion = prompt_promotion(move.to))) {
							if (gamerunning) {
								struct timecontrol *timecontrol = &tc[posa.turn];
								if (!timecontrol->infinite)
									subtract_timecontrol(timecontrol, timecontrol->offset, time_now());
							}
							put_move(&moves[i], 0);
						}
						break;
					}
				}
				selectedsquare = -1;
				refreshed = 0;
			}
		}
		break;
	}

draw:
	if (!refreshed)
		mainwin_draw();
}

int is_over(int displayed);

void reset_analysis(void) {
	sentdepth = 0;
	sentseldepth = 0;
	senttime = 0;
	sentnodes = 0;
	sentpv = 0;
	sentscore = 0;
	sentnps = 0;
	senthashfull = 0;
	senttbhits = 0;
	sentcpuload = 0;
	sentcurrmove = 0;
	sentcurrmovenumber = 0;
	npastinfo = 0;
	if (!analysisengine)
		return;
	fprintf(analysisengine->w, "stop\n");
	if (is_over(1))
		return;
	char positionfen[8192];
	fprintf(analysisengine->w, "%s\n", position_fen(positionfen, 1));
	engine_isready(analysisengine);
	fprintf(analysisengine->w, "go infinite\n");
	return;
}

void start_analysis(struct uciengine *ue) {
#ifndef _WIN32
	end_analysis();

	analysisengine = malloc(sizeof(*analysisengine));
	engine_open(analysisengine, ue);
	reset_analysis();
#endif
}

void add_analysis(struct uciinfo *a) {
	for (int i = npastinfo; i >= 1; i--)
		if (i < MAXPASTINFO)
			pastinfo[i] = pastinfo[i - 1];
	pastinfo[0] = *a;
	if (npastinfo < MAXPASTINFO)
		npastinfo++;
}

void start_game(const struct uciengine *black, const struct uciengine *white, const struct position *start, const struct timecontrol timecontrol[2]) {
	if (gamerunning)
		end_game();
	gamerunning = 1;

#ifndef _WIN32
	if (black) {
		blackengine = malloc(sizeof(*blackengine));
		engine_open(blackengine, black);
	}
	else {
		blackengine = NULL;
	}

	if (white) {
		whiteengine = malloc(sizeof(*whiteengine));
		engine_open(whiteengine, white);
	}
	else {
		whiteengine = NULL;
	}
#else
	whiteengine = blackengine = NULL;
#endif

	tc[0] = timecontrol[0];
	tc[1] = timecontrol[1];
	tc[0].movestogo = tc[0].totaltogo = 0;
	tc[1].movestogo = tc[1].totaltogo = 0;

	/* Save history if start == &posd? */
	set_position(start);

	sentwhite = sentblack = 0;
}

void end_game(void) {
	if (!gamerunning)
		return;
	gamerunning = 0;
	selectedsquare = -1;

	if (whiteengine) {
		engine_close(whiteengine);
		free(whiteengine);
		whiteengine = NULL;
	}

	if (blackengine) {
		engine_close(blackengine);
		free(blackengine);
		blackengine = NULL;
	}
}

int poscmp(const struct position *pos1, const struct position *pos2) {
	for (int sq = 0; sq < 64; sq++) {
		if (pos1->mailbox[sq].type != pos2->mailbox[sq].type)
			return 1;
		else if (pos1->mailbox[sq].type != EMPTY && pos1->mailbox[sq].color != pos2->mailbox[sq].color)
			return 1;
	}

	if (pos1->turn != pos2->turn)
		return 1;
	if (pos1->K != pos2->K)
		return 1;
	if (pos1->Q != pos2->Q)
		return 1;
	if (pos1->k != pos2->k)
		return 1;
	if (pos1->q != pos2->q)
		return 1;

	return 0;
}

int is_threefold(int displayed) {
	struct position pos = posa;
	for (int i = nmove - 1; i >= 0; i--)
		undo_move(&pos, &vmove[i].move);

	int nmoves = displayed ? selectedmove + 1 : nmove;

	/* Should care about the en passant square, but only
	 * if its actually possible to capture en passant. */
	for (int i = 0; i < nmoves - 7; i++) {
		int count = 1;
		struct position rep = pos;
		for (int j = i; j < nmoves && rep.halfmove == pos.halfmove + j - i; j++) {
			do_move(&rep, &vmove[j].move);

			if (j < i + 3 || (j - i) % 2 == 0)
				continue;

			if (!poscmp(&pos, &rep) && ++count == 3)
				return 1;
		}

		do_move(&pos, &vmove[i].move);
	}

	return 0;
}

int is_over(int displayed) {
	struct position pos = displayed ? posd : posa;
	int r;
	if (pos.halfmove >= 100)
		return STATUS_HALFMOVE;
	else if ((r = is_mate(&pos)))
		return r;
	else if ((is_threefold(displayed)))
		return STATUS_THREEFOLD;
	else if (!displayed && ((pos.turn == WHITE && whiteengine && engine_error(whiteengine) == EE_ILLEGALMOVE) ||
				(pos.turn == BLACK && blackengine && engine_error(blackengine) == EE_ILLEGALMOVE)))
		return STATUS_ILLEGALMOVE;
	else if (!displayed && ((pos.turn == WHITE && whiteengine && engine_error(whiteengine)) ||
				(pos.turn == BLACK && blackengine && engine_error(blackengine))))
		return STATUS_DISCONNECT;

	return STATUS_NOTOVER;
}

int subtract_timecontrol(struct timecontrol *timecontrol, timepoint_t start, timepoint_t end) {
	timecontrol->totaltogo -= end - start;
	return timecontrol->totaltogo <= 0;
}

void update_game(void) {
	if (!gamerunning)
		return;

	struct engineconnection *ec;

	char result[128] = { 0 };
	int r;
	if ((r = is_over(0)) != STATUS_NOTOVER) {
		end_game();
		switch (r) {
		case STATUS_STALEMATE:
			sprintf(result, "Stalemate.");
			break;
		case STATUS_THREEFOLD:
			sprintf(result, "Draw by repetition.");
			break;
		case STATUS_HALFMOVE:
			sprintf(result, "Draw by fifty-move rule.");
			break;
		case STATUS_ILLEGALMOVE:
			sprintf(result, "%s loses by illegal move.", posa.turn == WHITE ? "White" : "Black");
			break;
		case STATUS_CHECKMATE:
			sprintf(result, "%s wins by checkmate.", posa.turn == BLACK ? "White" : "Black");
			break;
		case STATUS_DISCONNECT:
			sprintf(result, "%s loses by disconnection.", posa.turn == WHITE ? "White" : "Black");
			break;
		}
		info("Game Over", result, INFO_MESSAGE, 5, 36);
		return;
	}

	if ((posa.turn == WHITE && !sentwhite) || (posa.turn == BLACK && !sentblack)) {
		sentwhite = posa.turn == WHITE;
		sentblack = posa.turn == BLACK;

		struct timecontrol *timecontrol = &tc[posa.turn];
		timecontrol->offset = time_now();

		timecontrol->totaltogo += timecontrol->inc;

		if (timecontrol->movestogo == 0) {
			timecontrol->movestogo = timecontrol->moves ? timecontrol->moves : -1;
			timecontrol->totaltogo += timecontrol->total;
		}

		if ((posa.turn == WHITE && (ec = whiteengine)) || (posa.turn == BLACK && (ec = blackengine))) {
			fprintf(ec->w, "stop\n");
			engine_isready(ec);
			char fenstr[8192];
			fprintf(ec->w, "%s\n", position_fen(fenstr, 0));
			if (timecontrol->movestogo <= 0)
				fprintf(ec->w, "go wtime %lld winc %lld btime %lld binc %lld\n", tc[WHITE].totaltogo / TPPERMS, tc[WHITE].inc / TPPERMS, tc[BLACK].totaltogo / TPPERMS, tc[BLACK].inc / TPPERMS);
			else
				fprintf(ec->w, "go movestogo %lld wtime %lld winc %lld btime %lld binc %lld\n", timecontrol->movestogo, tc[WHITE].totaltogo / TPPERMS, tc[WHITE].inc / TPPERMS, tc[BLACK].totaltogo / TPPERMS, tc[BLACK].inc / TPPERMS);
		}

		if (timecontrol->movestogo > 0)
			timecontrol->movestogo--;
	}
	else if ((posa.turn == WHITE && (ec = whiteengine) && engine_readyok(ec) && engine_hasbestmove(ec)) || (posa.turn == BLACK && (ec = blackengine) && engine_readyok(ec) && engine_hasbestmove(ec))) {
		char bestmove[128] = { 0 };
		pthread_mutex_lock(&ec->mutex);
		memcpy(bestmove, &ec->bestmove[9], 128 - 9);
		timepoint_t bestmovetime = ec->bestmovetime;
		timepoint_t start = ec->readyok;
		pthread_mutex_unlock(&ec->mutex);
		engine_reset(ec);
		char *c;
		if ((c = strchr(bestmove, ' ')) || (c = strchr(bestmove, '\n')))
			*c = '\0';

		struct move move;
		if (string_to_move(&move, &posa, bestmove)) {
			subtract_timecontrol(&tc[posa.turn], start, bestmovetime);
			if (tc[posa.turn].totaltogo < 0)
				goto lostontime;
			if (!analysisengine)
				reset_analysis();
			put_move(&move, 1);
			refreshed = 0;
		}
		else {
			engine_seterror(ec, EE_ILLEGALMOVE);
		}
	}

	struct timecontrol *timecontrol = &tc[posa.turn];
	if (timecontrol->infinite)
		return;

	timepoint_t t = 0;
	if ((sentwhite && (ec = whiteengine) && engine_readyok(ec)) || (sentblack && (ec = blackengine) && engine_readyok(ec))) {
		t = engine_readyok(ec);
	}
	else if ((sentwhite && !whiteengine) || (sentblack && !blackengine)) {
		t = timecontrol->offset;
	}

	if (t == 0 || time_since(t) < timecontrol->totaltogo)
		return;

lostontime:
	end_game();
	sprintf(result, "%s loses on time.", posa.turn == WHITE ? "White" : "Black");
	info("Game Over", result, INFO_MESSAGE, 5, 26);
}

struct engineconnection *do_analysis(void) {
	if (analysisengine)
		return analysisengine;
	if (hideengineoutput)
		return NULL;
	if (gamerunning && posa.turn == WHITE && whiteengine)
		return whiteengine;
	if (gamerunning && posa.turn == BLACK && blackengine)
		return blackengine;
	return NULL;
}

void parse_analysis(const struct position *current) {
	struct engineconnection *ec = do_analysis();
	if (!ec) {
		npastinfo = 0;
		return;
	}
	struct position pos = { 0 };
	char line[4096], *token = NULL, *endptr;
	int error = 0;
	char engineerror[4096] = { 0 };
	while (errno = 0, engine_readyok(ec) && fgets(line, sizeof(line), ec->r) && !error) {
		struct uciinfo a = { 0 };
		if ((token = strchr(line, '\n')))
			*token = '\0';
		memcpy(engineerror, line, sizeof(line));
		if (!(token = strtok(line, " \n")) || strcmp(token, "info"))
			continue;
		int add = 0, dontadd = 0;
		while ((token = strtok(NULL, " \n")) && !error) {
			if (!strcmp(token, "depth")) {
				add = 1;
				sentdepth = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 1;
					break;
				}
				errno = 0;
				a.depth = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || a.depth < 0)
					a.depth = 0;
			}
			else if (!strcmp(token, "seldepth")) {
				add = 1;
				sentseldepth = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 2;
					break;
				}
				errno = 0;
				a.seldepth = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || a.seldepth < 0)
					a.seldepth = 0;
			}
			else if (!strcmp(token, "time")) {
				add = 1;
				senttime = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 3;
					break;
				}
				errno = 0;
				a.t = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || a.t < 0)
					a.t = 0;
			}
			else if (!strcmp(token, "nodes")) {
				add = 1;
				sentnodes = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 4;
					break;
				}
				errno = 0;
				a.nodes = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || a.nodes < 0)
					a.nodes = 0;
			}
			else if (!strcmp(token, "nps")) {
				add = 1;
				sentnps = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 5;
					break;
				}
				errno = 0;
				nps = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || nps < 0)
					nps = 0;
			}
			else if (!strcmp(token, "score")) {
				sentscore = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 6;
					break;
				}
				if (!strcmp(token, "cp")) {
					if (!(token = strtok(NULL, " \n"))) {
						error = 7;
						break;
					}
					errno = 0;
					a.cp = strtoll(token, &endptr, 10);
					if (errno || *endptr != '\0')
						a.cp = 0;
					if (current && !relativescore && current->turn == BLACK)
						a.cp = -a.cp;
					sentscore = 1;
					add = 1;
				}
				else if (!strcmp(token, "mate")) {
					if (!(token = strtok(NULL, " \n"))) {
						error = 8;
						break;
					}
					errno = 0;
					a.mate = strtoll(token, &endptr, 10);
					if (errno || *endptr != '\0')
						a.mate = 0;
					sentscore = 1;
					add = 1;
				}
			}
			else if (!strcmp(token, "lowerbound")) {
				if (current && !relativescore && current->turn == BLACK)
					a.upperbound = 1;
				else
					a.lowerbound = 1;
			}
			else if (!strcmp(token, "upperbound")) {
				if (current && !relativescore && current->turn == BLACK)
					a.lowerbound = 1;
				else
					a.upperbound = 1;
			}
			else if (!strcmp(token, "pv")) {
				if (current)
					pos = *current;
				int k = 0;
				int illegalpv = 0;
				while ((token = strtok(NULL, " \n")) && !error) {
					if (k >= MAXINFOPV)
						continue;
					if (current) {
						struct move move;
						if (string_to_move(&move, &pos, token) && !illegalpv) {
							move_pgn(a.pv[k++], &pos, &move);
							do_move(&pos, &move);
						}
						else {
							illegalpv = 1;
						}
					}
					else {
						if (strlen(token) > 7)
							token[7] = '\0';
						sprintf(a.pv[k++], "%s", token);
					}
				}
				add = 1;
				sentpv = 1;
			}
			else if (!strcmp(token, "string")) {
				break;
			}
			else if (!strcmp(token, "currmove")) {
				if (!(token = strtok(NULL, " \n"))) {
					error = 9;
					break;
				}
				dontadd = 1;
				sentcurrmove = 1;
				if (current) {
					pos = *current;
					struct move move;
					if (string_to_move(&move, &pos, token)) {
						move_pgn(currmove, &pos, &move);
						do_move(&pos, &move);
					}
				}
				else {
					if (strlen(token) > 7)
						token[7] = '\0';
					sprintf(currmove, "%s", token);
				}
			}
			else if (!strcmp(token, "currmovenumber")) {
				dontadd = 1;
				sentcurrmovenumber = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 10;
					break;
				}
				errno = 0;
				currmovenumber = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || currmovenumber <= 0)
					currmovenumber = 1;
			}
			else if (!strcmp(token, "tbhits")) {
				senttbhits = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 11;
					break;
				}
				errno = 0;
				tbhits = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || tbhits < 0)
					tbhits = 0;
			}
			else if (!strcmp(token, "hashfull")) {
				senthashfull = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 12;
					break;
				}
				errno = 0;
				hashfull = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || hashfull < 0)
					hashfull = 0;
			}
			else if (!strcmp(token, "cpuload")) {
				sentcpuload = 1;
				if (!(token = strtok(NULL, " \n"))) {
					error = 13;
					break;
				}
				errno = 0;
				cpuload = strtoll(token, &endptr, 10);
				if (errno || *endptr != '\0' || cpuload < 0)
					cpuload = 0;
			}
			else if (!strcmp(token, "multipv")) {
				if (!(token = strtok(NULL, " \n"))) {
					error = 14;
					break;
				}
				if (strcmp(token, "1")) {
					dontadd = 1;
					break;
				}
			}
			else {
				error = 15;
				break;
			}
		}
		if (!error && add && !dontadd)
			add_analysis(&a);
	}
	if (error) {
		if (analysisengine) {
			snprintf(line, 500, "Error (%d): Poorly formatted engine output: %s", error, engineerror);
			info("Engine Error", line, INFO_ERROR, 10, 80);
		}
		end_analysis();
	}
}

void end_analysis(void) {
	if (!analysisengine)
		return;
	if (engine_close(analysisengine))
		info("Engine Error", "The analysis engine is unresponsive.", INFO_ERROR, 5, 42);
	free(analysisengine);
	analysisengine = NULL;
}

int backward_move(int dontreset) {
	if (selectedmove == -1)
		return 1;
	undo_move(&posd, &vmove[selectedmove--].move);
	if (!dontreset)
		reset_analysis();
	int offset = vmove[0].color == BLACK;
	if (selectedmove < shownmove - offset && shownmove >= 2)
		shownmove -= 2;
	return 0;
}

int forward_move(int dontreset) {
	if (selectedmove > nmove - 2)
		return 1;
	do_move(&posd, &vmove[++selectedmove].move);
	if (!dontreset)
		reset_analysis();
	int offset = vmove[0].color == BLACK;
	if (selectedmove >= 2 * MOVESMAXLINES + shownmove - offset)
		shownmove += 2;
	return 0;
}

void backward_full(int dontreset) {
	int n = 0;
	while (!backward_move(1))
		n++;
	if (n && !dontreset)
		reset_analysis();
}

void forward_full(int dontreset) {
	int n = 0;
	while (!forward_move(1))
		n++;
	if (n && !dontreset)
		reset_analysis();
}

void put_move(struct move *move, int at_end) {
	int save_history = 0;
	struct position old;
	if (!at_end) {
		if (nmove > selectedmove + 1) {
			/* Only delete history if we made a different move. */
			if (!movecmp(move, &vmove[selectedmove + 1].move)) {
				save_history = nmove;
				old = posa;
			}
			nmove = selectedmove + 1;
			posa = posd;
		}
	}
	if (nmove >= smove) {
		smove = smove ? 2 * smove : 4;
		vmove = realloc(vmove, smove * sizeof(*vmove));
	}

	vmove[nmove].move = *move;
	vmove[nmove].color = posa.turn;
	vmove[nmove].fullmove = posa.fullmove;
	move_pgn(vmove[nmove].name, &posa, move);
	do_move(&posa, move);

	nmove++;

	if (nmove == selectedmove + 2) {
		if (gamerunning && autoflip) {
			if (posa.turn == WHITE && !whiteengine)
				flipped = 0;
			if (posa.turn == BLACK && !blackengine)
				flipped = 1;
		}
		forward_move(0);
	}

	if (save_history) {
		nmove = save_history;
		posa = old;
	}
}

void moves_draw(void) {
	if (nmove == 0)
		return;
	set_color(mainwin.win, &cs.text);
	int offset = vmove[0].color == BLACK;
	int current = shownmove - offset;
	for (int line = 0; line < MOVESMAXLINES; line++, current += 2) {
		set_color(mainwin.win, &cs.text);
		for (int i = 0; i < 20; i++)
			mvwaddch(mainwin.win, 1 + line, 85 + i, ' ');
		if (current == -1 && nmove > 0) {
			mvwprintw(mainwin.win, 1 + line, 85, "%3d. ...", vmove[0].fullmove <= 999 ? vmove[0].fullmove : 999);
		}
		else if (current < nmove) {
			set_color(mainwin.win, &cs.text);
			mvwprintw(mainwin.win, 1 + line, 85, "%3d.", vmove[current].fullmove <= 999 ? vmove[current].fullmove : 999);
			set_color(mainwin.win, current == selectedmove ? &cs.texthl : &cs.text);
			mvwaddstr(mainwin.win, 1 + line, 85 + 5, vmove[current].name);
		}
		if (current + 1 < nmove) {
			set_color(mainwin.win, current + 1 == selectedmove ? &cs.texthl : &cs.text);
			mvwaddstr(mainwin.win, 1 + line, 85 + 13, vmove[current + 1].name);
		}
		wrefresh(mainwin.win);
	}
}

/* size of line should be resonably big, 4096 should suffice. */
char *position_fen(char *line, int displayed) {
	struct position pos = displayed ? posd : posa;
	int nmoves = displayed ? selectedmove + 1 : nmove;

	char fenstr[128];
	if (pos.halfmove >= 100) {
		pos.halfmove = 0;
		sprintf(line, "position fen %s", pos_to_fen(fenstr, &pos));
		return line;
	}
	int move;
	if (pos.halfmove == 0) {
		move = -1;
	}
	else {
		for (move = nmoves - 1; move >= 0; move--)
			if (vmove[move].move.halfmove == 0)
				break;

		if (move < 0 && nmoves)
			move = 0;
	}

	if (move < 0) {
		sprintf(line, "position fen %s", pos_to_fen(fenstr, &pos));
	}
	else {
		for (int i = nmoves - 1; i >= move; i--)
			undo_move(&pos, &vmove[i].move);
		sprintf(line, "position fen %s moves", pos_to_fen(fenstr, &pos));
		char movestr[8] = " ";
		for (int i = move; i < nmoves; i++) {
			move_algebraic(&movestr[1], &vmove[i].move);
			strcat(line, movestr);
		}
	}

	return line;
}

void fen_draw(WINDOW *win, struct position *pos) {
	char fenstr[128];
	memset(fenstr, ' ', 128);
	pos_to_fen(fenstr, pos);
	/* Limit the length of the fen. In very rare cases this would actually
	 * make parts of the fen invisible.
	 */
	if (strlen(fenstr) > 78) {
		fenstr[75] = '.';
		fenstr[76] = '.';
		fenstr[77] = '.';
	}
	else {
		fenstr[strlen(fenstr)] = ' ';
	}
	fenstr[78] = '\0';
	set_color(win, fenselected ? &cs.red : &cs.text);
	mvwprintw(win, 43, 2, "%s", fenstr);
}

int fen_filter(char c) {
	return c != '/' && c != ' ' && c != '-' && (c < '0' || c > '9') &&
		(c < 'a' || c > 'h') && c != 'w' && c != 'K' && c != 'Q' &&
		c != 'R' && c != 'B' && c != 'N' && c != 'P' && c != 'k' &&
		c != 'q' && c != 'r' && c != 'b' && c != 'n' && c != 'p';
}

static long long max(long long a, long long b) {
	return a > b ? a : b;
}

static long long min(long long a, long long b) {
	return a > b ? b : a;
}

void parsedepth(char strs[MAXPASTINFO][8]) {
	if (!sentdepth)
		return;
	for (int i = 0; i < npastinfo; i++) {
		if (sentseldepth)
			sprintf(strs[i], "%lld/%lld", min(pastinfo[i].depth, 999), min(pastinfo[i].seldepth, 999));
		else
			sprintf(strs[i], "%lld", min(pastinfo[i].depth, 9999999));
	}
}

void parsetime(char strs[MAXPASTINFO][7]) {
	if (!senttime)
		return;

	for (int i = 0; i < npastinfo; i++) {
		long long t = pastinfo[i].t;
		if (t < 1000)
			sprintf(strs[i], "%lldms", t);
		else if (t < 60 * 1000)
			sprintf(strs[i], "%lld.%llds", t / 1000, (t % 1000) / 100);
		else if (t < 60ll * 60 * 1000)
			sprintf(strs[i], "%lld.%lldm", t / (60 * 1000), (t % (60 * 1000)) / (60 * 100));
		else if (t < 24ll * 60 * 60 * 1000)
			sprintf(strs[i], "%lld.%lldh", t / (60ll * 60 * 1000), (t % (60ll * 60 * 1000)) / (60 * 60 * 100));
		else if (t < 1000ll * 24 * 60 * 60 * 1000)
			sprintf(strs[i], "%lld.%lldd", t / (1000ll * 24 * 60 * 60 * 1000), (t % (1000ll * 24 * 60 * 60 * 1000)) / (1000ll * 24 * 60 * 60 * 100));
		else
			sprintf(strs[i], ">100d");
	}
}

void parsenodes(char strs[MAXPASTINFO][7]) {
	if (!sentnodes)
		return;

	for (int i = 0; i < npastinfo; i++) {
		long long nodes = pastinfo[i].nodes;
		if (nodes < 100)
			sprintf(strs[i], "%lld", nodes);
		else if (nodes < 100ll * 1000)
			sprintf(strs[i], "%lld.%lldk", nodes / 1000, (nodes % 1000) / 100);
		else if (nodes < 100ll * 1000 * 1000)
			sprintf(strs[i], "%lld.%lldM", nodes / (1000 * 1000), (nodes % (1000 * 1000)) / (1000 * 100));
		else if (nodes < 100ll * 1000 * 1000 * 1000)
			sprintf(strs[i], "%lld.%lldG", nodes / (1000ll * 1000 * 1000), (nodes % (1000ll * 1000 * 1000)) / (1000ll * 1000 * 100));
		else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000)
			sprintf(strs[i], "%lld.%lldT", nodes / (1000ll * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 100));
		else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000 * 1000)
			sprintf(strs[i], "%lld.%lldP", nodes / (1000ll * 1000 * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 1000 * 100));
		else
			sprintf(strs[i], ">100P");
	}
}

void parsescore(char strs[MAXPASTINFO][7]) {
	if (!sentscore)
		return;

	for (int i = 0; i < npastinfo; i++) {
		int j = 0;
		if ((pastinfo[i].lowerbound || pastinfo[i].upperbound) && !pastinfo[j].mate)
			strs[i][j++] = ' ';
		
		if (pastinfo[i].mate)
			sprintf(strs[i], "#%lld", min(llabs(pastinfo[i].mate), 99999));
		else
			sprintf(&strs[i][j], "%+lld", max(min(pastinfo[i].cp, 9999), -9999));
	}
}

void parsepv(char strs[MAXPASTINFO][256]) {
	if (!sentpv)
		return;

	for (int i = 0; i < npastinfo; i++) {
		/* This will always fit. */
		int n = 0;
		for (int j = 0; j < MAXINFOPV; j++) {
			if (pastinfo[i].pv[j][0] == '\0')
				break;
			if (j == 0)
				n += sprintf(&strs[i][n], "%s", pastinfo[i].pv[j]);
			else
				n += sprintf(&strs[i][n], " %s", pastinfo[i].pv[j]);
		}
	}
}

void parsetbhits(char str[8]) {
	if (!senttbhits || tbhits < 0)
		return;

	long long nodes = tbhits;
	if (nodes < 100)
		sprintf(str, "%lld", nodes);
	else if (nodes < 100ll * 1000)
		sprintf(str, "%lld.%lldk", nodes / 1000, (nodes % 1000) / 100);
	else if (nodes < 100ll * 1000 * 1000)
		sprintf(str, "%lld.%lldM", nodes / (1000 * 1000), (nodes % (1000 * 1000)) / (1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldG", nodes / (1000ll * 1000 * 1000), (nodes % (1000ll * 1000 * 1000)) / (1000ll * 1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldT", nodes / (1000ll * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldP", nodes / (1000ll * 1000 * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 1000 * 100));
	else
		sprintf(str, ">100P");
}

void parsepermill(char str[7], long long n, int sent) {
	if (!sent || n < 0)
		return;

	if (n < 100000)
		sprintf(str, "%lld%%", n / 10);
	else
		sprintf(str, "99999%%");
}

void parsenps(char str[7]) {
	if (!sentnps || nps < 0)
		return;

	long long nodes = nps;
	if (nodes < 100)
		sprintf(str, "%lld", nodes);
	else if (nodes < 100ll * 1000)
		sprintf(str, "%lld.%lldk", nodes / 1000, (nodes % 1000) / 100);
	else if (nodes < 100ll * 1000 * 1000)
		sprintf(str, "%lld.%lldM", nodes / (1000 * 1000), (nodes % (1000 * 1000)) / (1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldG", nodes / (1000ll * 1000 * 1000), (nodes % (1000ll * 1000 * 1000)) / (1000ll * 1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldT", nodes / (1000ll * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 100));
	else if (nodes < 100ll * 1000 * 1000 * 1000 * 1000 * 1000)
		sprintf(str, "%lld.%lldP", nodes / (1000ll * 1000 * 1000 * 1000 * 1000), (nodes % (1000ll * 1000 * 1000 * 1000 * 1000)) / (1000ll * 1000 * 1000 * 1000 * 100));
	else
		sprintf(str, ">100P");
}

void parsecurrmove(char str[256]) {
	if (!sentcurrmove)
		return;

	if (sentcurrmovenumber)
		sprintf(str, "%s/%lld", currmove, min(currmovenumber, 256));
	else
		sprintf(str, "%s", currmove);
}

static void analysisframe_draw(void) {
	if (!do_analysis())
		return;
	set_color(mainwin.win, &cs.text);
	if (COLS >= 127) {
		mvwaddstr(mainwin.win, 6, 109, "Depth");
		mvwaddch(mainwin.win, 5, 108, ACS_ULCORNER);
		mvwvline(mainwin.win, 6, 108, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, 108, ACS_LLCORNER);
		mvwaddch(mainwin.win, 5, 116, ACS_URCORNER);
		mvwvline(mainwin.win, 6, 116, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, 116, ACS_LRCORNER);
		for (int i = 0; i < 19; i++) {
			mvwhline(mainwin.win, 5 + 2 * i, 109, ACS_HLINE, 7);
			if (i != 0 && i != 18) {
				mvwaddch(mainwin.win, 5 + 2 * i, 108, ACS_LTEE);
				mvwaddch(mainwin.win, 5 + 2 * i, 116, ACS_RTEE);
			}
		}

		mvwaddstr(mainwin.win, 1, 109, "Tbhits");
		mvwaddch(mainwin.win, 0, 108, ACS_ULCORNER);
		mvwvline(mainwin.win, 1, 108, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, 108, ACS_LLCORNER);
		mvwaddch(mainwin.win, 0, 116, ACS_URCORNER);
		mvwvline(mainwin.win, 1, 116, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, 116, ACS_LRCORNER);
		for (int i = 0; i < 3; i++) {
			mvwhline(mainwin.win, 2 * i, 109, ACS_HLINE, 7);
			if (i == 1) {
				mvwaddch(mainwin.win, 2 * i, 108, ACS_LTEE);
				mvwaddch(mainwin.win, 2 * i, 116, ACS_RTEE);
			}
		}
	}
	if (COLS >= 134) {
		mvwaddstr(mainwin.win, 6, 117, "Time");
		mvwaddch(mainwin.win, 5, 116, ACS_TTEE);
		mvwaddch(mainwin.win, 41, 116, ACS_BTEE);
		mvwaddch(mainwin.win, 5, 123, ACS_URCORNER);
		mvwvline(mainwin.win, 6, 123, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, 123, ACS_LRCORNER);
		for (int i = 0; i < 19; i++) {
			mvwhline(mainwin.win, 5 + 2 * i, 117, ACS_HLINE, 6);
			if (i != 0 && i != 18) {
				mvwaddch(mainwin.win, 5 + 2 * i, 116, ACS_PLUS);
				mvwaddch(mainwin.win, 5 + 2 * i, 123, ACS_RTEE);
			}
		}

		mvwaddstr(mainwin.win, 1, 117, "Hash");
		mvwaddch(mainwin.win, 0, 116, ACS_TTEE);
		mvwaddch(mainwin.win, 4, 116, ACS_BTEE);
		mvwaddch(mainwin.win, 0, 123, ACS_URCORNER);
		mvwvline(mainwin.win, 1, 123, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, 123, ACS_LRCORNER);
		for (int i = 0; i < 3; i++) {
			mvwhline(mainwin.win, 2 * i, 117, ACS_HLINE, 6);
			if (i == 1) {
				mvwaddch(mainwin.win, 2 * i, 116, ACS_PLUS);
				mvwaddch(mainwin.win, 2 * i, 123, ACS_RTEE);
			}
		}
	}
	if (COLS >= 141) {
		mvwaddstr(mainwin.win, 6, 124, "Score");
		mvwaddch(mainwin.win, 5, 123, ACS_TTEE);
		mvwaddch(mainwin.win, 41, 123, ACS_BTEE);
		mvwaddch(mainwin.win, 5, 130, ACS_URCORNER);
		mvwvline(mainwin.win, 6, 130, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, 130, ACS_LRCORNER);
		for (int i = 0; i < 19; i++) {
			mvwhline(mainwin.win, 5 + 2 * i, 124, ACS_HLINE, 6);
			if (i != 0 && i != 18) {
				mvwaddch(mainwin.win, 5 + 2 * i, 123, ACS_PLUS);
				mvwaddch(mainwin.win, 5 + 2 * i, 130, ACS_RTEE);
			}
		}

		mvwaddstr(mainwin.win, 1, 124, "CPU");
		mvwaddch(mainwin.win, 0, 123, ACS_TTEE);
		mvwaddch(mainwin.win, 4, 123, ACS_BTEE);
		mvwaddch(mainwin.win, 0, 130, ACS_URCORNER);
		mvwvline(mainwin.win, 1, 130, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, 130, ACS_LRCORNER);
		for (int i = 0; i < 3; i++) {
			mvwhline(mainwin.win, 2 * i, 124, ACS_HLINE, 6);
			if (i == 1) {
				mvwaddch(mainwin.win, 2 * i, 123, ACS_PLUS);
				mvwaddch(mainwin.win, 2 * i, 130, ACS_RTEE);
			}
		}
	}
	if (COLS >= 148) {
		mvwaddstr(mainwin.win, 6, 131, "Nodes");
		mvwaddch(mainwin.win, 5, 130, ACS_TTEE);
		mvwaddch(mainwin.win, 41, 130, ACS_BTEE);
		mvwaddch(mainwin.win, 5, 137, ACS_URCORNER);
		mvwvline(mainwin.win, 6, 137, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, 137, ACS_LRCORNER);
		for (int i = 0; i < 19; i++) {
			mvwhline(mainwin.win, 5 + 2 * i, 131, ACS_HLINE, 6);
			if (i != 0 && i != 18) {
				mvwaddch(mainwin.win, 5 + 2 * i, 130, ACS_PLUS);
				mvwaddch(mainwin.win, 5 + 2 * i, 137, ACS_RTEE);
			}
		}

		mvwaddstr(mainwin.win, 1, 131, "NPS");
		mvwaddch(mainwin.win, 0, 130, ACS_TTEE);
		mvwaddch(mainwin.win, 4, 130, ACS_BTEE);
		mvwaddch(mainwin.win, 0, 137, ACS_URCORNER);
		mvwvline(mainwin.win, 1, 137, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, 137, ACS_LRCORNER);
		for (int i = 0; i < 3; i++) {
			mvwhline(mainwin.win, 2 * i, 131, ACS_HLINE, 6);
			if (i == 1) {
				mvwaddch(mainwin.win, 2 * i, 130, ACS_PLUS);
				mvwaddch(mainwin.win, 2 * i, 137, ACS_RTEE);
			}
		}
	}
	if (COLS >= 157) {
		int x = COLS - 11;
		mvwaddstr(mainwin.win, 6, 138, "PV");
		mvwaddch(mainwin.win, 5, 137, ACS_TTEE);
		mvwaddch(mainwin.win, 41, 137, ACS_BTEE);
		mvwaddch(mainwin.win, 5, x, ACS_URCORNER);
		mvwvline(mainwin.win, 6, x, ACS_VLINE, 35);
		mvwaddch(mainwin.win, 41, x, ACS_LRCORNER);
		for (int i = 0; i < 19; i++) {
			mvwhline(mainwin.win, 5 + 2 * i, 138, ACS_HLINE, x - 138);
			if (i != 0 && i != 18) {
				mvwaddch(mainwin.win, 5 + 2 * i, 137, ACS_PLUS);
				mvwaddch(mainwin.win, 5 + 2 * i, x, ACS_RTEE);
			}
		}

		mvwaddstr(mainwin.win, 1, 138, "Currmove");
		mvwaddch(mainwin.win, 0, 137, ACS_TTEE);
		mvwaddch(mainwin.win, 4, 137, ACS_BTEE);
		mvwaddch(mainwin.win, 0, x, ACS_URCORNER);
		mvwvline(mainwin.win, 1, x, ACS_VLINE, 3);
		mvwaddch(mainwin.win, 4, x, ACS_LRCORNER);
		for (int i = 0; i < 3; i++) {
			mvwhline(mainwin.win, 2 * i, 138, ACS_HLINE, x - 138);
			if (i == 1) {
				mvwaddch(mainwin.win, 2 * i, 137, ACS_PLUS);
				mvwaddch(mainwin.win, 2 * i, x, ACS_RTEE);
			}
		}
	}
}

static void analysis_draw(void) {
	if (!do_analysis())
		return;

	parse_analysis(&posd);
	/* Maybe the engine was terminated and analysis cancelled. */
	if (!do_analysis())
		return;

	char depthstrs[MAXPASTINFO][8] = { 0 };
	char timestrs[MAXPASTINFO][7] = { 0 };
	char nodesstrs[MAXPASTINFO][7] = { 0 };
	char scorestrs[MAXPASTINFO][7] = { 0 };
	char pvstrs[MAXPASTINFO][256] = { 0 };
	char tbhitsstr[8] = { 0 };
	char hashfullstr[7] = { 0 };
	char cpuloadstr[7] = { 0 };
	char npsstr[7] = { 0 };
	char currmovestr[256] = { 0 };
	parsedepth(depthstrs);
	parsetime(timestrs);
	parsenodes(nodesstrs);
	parsescore(scorestrs);
	parsepv(pvstrs);
	parsetbhits(tbhitsstr);
	parsepermill(hashfullstr, hashfull, senthashfull);
	parsepermill(cpuloadstr, cpuload, sentcpuload);
	parsenps(npsstr);
	parsecurrmove(currmovestr);

	if (COLS >= 127)
		mvwaddstr(mainwin.win, 3, 109, tbhitsstr);
	if (COLS >= 134)
		mvwaddstr(mainwin.win, 3, 117, hashfullstr);
	if (COLS >= 141)
		mvwaddstr(mainwin.win, 3, 124, cpuloadstr);
	if (COLS >= 148)
		mvwaddstr(mainwin.win, 3, 131, npsstr);
	if (COLS >= 157) {
		if ((int)strlen(currmovestr) >= COLS - 149) {
			char *c = strchr(currmovestr, '/');
			if (c)
				*c = '\0';
			currmovestr[8] = '\0';
		}
		mvwaddstr(mainwin.win, 3, 138, currmovestr);
	}
	for (int i = 0; i < MAXPASTINFO; i++) {
		if (COLS >= 127) {
			mvwhline(mainwin.win, 8 + 2 * i, 109, ' ', 7);
			if (i < npastinfo)
				mvwaddstr(mainwin.win, 8 + 2 * i, 109, depthstrs[i]);
		}
		if (COLS >= 134) {
			mvwhline(mainwin.win, 8 + 2 * i, 117, ' ', 6);
			if (i < npastinfo)
				mvwaddstr(mainwin.win, 8 + 2 * i, 117, timestrs[i]);
		}
		if (COLS >= 141) {
			mvwhline(mainwin.win, 8 + 2 * i, 124, ' ', 6);
			if (i < npastinfo) {
				mvwaddstr(mainwin.win, 8 + 2 * i, 124, scorestrs[i]);
				if (pastinfo[i].lowerbound || pastinfo[i].upperbound)
					mvwaddch(mainwin.win, 8 + 2 * i, 124, pastinfo[i].lowerbound ? ACS_GEQUAL : ACS_LEQUAL);
			}
		}
		if (COLS >= 148) {
			mvwhline(mainwin.win, 8 + 2 * i, 131, ' ', 6);
			if (i < npastinfo)
				mvwaddstr(mainwin.win, 8 + 2 * i, 131, nodesstrs[i]);
		}
		if (COLS >= 157) {
			mvwhline(mainwin.win, 8 + 2 * i, 138, ' ', COLS - 149);
			if (i < npastinfo) {
				if ((int)strlen(pvstrs[i]) >= COLS - 149) {
					pvstrs[i][COLS - 152] = '.';
					pvstrs[i][COLS - 151] = '.';
					pvstrs[i][COLS - 150] = '.';
					pvstrs[i][COLS - 149] = '\0';
				}
				mvwaddstr(mainwin.win, 8 + 2 * i, 138, pvstrs[i]);
			}
		}
		/* Have to refresh here for otherwise the terminal flickers... */
		wrefresh(mainwin.win);
	}
}

void game_draw(void) {
	if (!gamerunning)
		return;

	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 1, 45, 0, 3, 19);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 1, 45, 20, 3, 10);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 1, 45, 52, 3, 10);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 1, 45, 63, 3, 19);

	set_color(mainwin.win, &cs.text);
	char tcstr[7];
	if (!tc[WHITE].infinite)
		timepoint_str(tcstr, 7, tc[WHITE].totaltogo > 0 ? tc[WHITE].totaltogo - (posa.turn == WHITE && (!whiteengine || engine_readyok(whiteengine)) ? time_since(tc[WHITE].offset) : 0) : tc[WHITE].total);
	else
		sprintf(tcstr, "oo");
	mvwaddstr(mainwin.win, 46, 28 - strlen(tcstr), tcstr);

	if (!tc[BLACK].infinite)
		timepoint_str(tcstr, 7, tc[BLACK].totaltogo > 0 ? tc[BLACK].totaltogo - (posa.turn == BLACK && (!blackengine || engine_readyok(blackengine)) ? time_since(tc[BLACK].offset) : 0) : tc[BLACK].total);
	else
		sprintf(tcstr, "oo");
	mvwaddstr(mainwin.win, 46, 60 - strlen(tcstr), tcstr);

	char name[17];
	if (whiteengine) {
		snprintf(name, 17, "%s", whiteengine->name);
		if (strlen(name) == 16) {
			name[12] = '.';
			name[13] = '.';
			name[14] = '.';
			name[15] = '\0';
		}
	}
	else
		sprintf(name, "Human");
	mvwaddstr(mainwin.win, 46, 10 - (strlen(name) + 1) / 2, name);
	if (blackengine) {
		snprintf(name, 17, "%s", blackengine->name);
		if (strlen(name) == 16) {
			name[12] = '.';
			name[13] = '.';
			name[14] = '.';
			name[15] = '\0';
		}
	}
	else
		sprintf(name, "Human");
	mvwaddstr(mainwin.win, 46, 72 - strlen(name) / 2, name);
}

int exclude(int y, int x) {
	if (do_analysis()) {
		int xmax = 0;
		if (COLS >= 127)
			xmax = 116;
		if (COLS >= 134)
			xmax = 123;
		if (COLS >= 141)
			xmax = 130;
		if (COLS >= 148)
			xmax = 137;
		if (COLS >= 157)
			xmax = COLS - 11;
		if (8 <= y && y <= 41 && 109 <= x && x <= xmax)
			return 1;
	}

	return nmove && 1 <= y && y <= 40 && 85 <= x && x <= 84 + 20;
}

void mainwin_draw(void) {
	draw_fill(mainwin.win, &cs.border, 0, 0, LINES, COLS, &exclude);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 0, 5 * 8 + 2, 10 * 8 + 2);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 0, 3, 82);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 83, 42, 24);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 83, 3, 24);
	set_color(mainwin.win, &cs.text);
	mvwaddch(mainwin.win, 5 * 8 + 3, 87, ACS_LARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 88, ACS_LARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 92, ACS_LARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 97, ACS_RARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 101, ACS_RARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 102, ACS_RARROW);

	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 45, 31, 3, 9);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 45, 42, 3, 9);
	set_color(mainwin.win, posd.turn == WHITE ? &cs.texthl : &cs.text);
	mvwaddstr(mainwin.win, 46, 33, "White");
	set_color(mainwin.win, posd.turn == BLACK ? &cs.texthl : &cs.text);
	mvwaddstr(mainwin.win, 46, 44, "Black");

	board_draw(mainwin.win, 1, 1, &posd, selectedsquare, flipped);
	char fenstr[128];
	if (!fenselected)
		field_set(&fen, pos_to_fen(fenstr, &posd));
	field_draw(&fen, fenselected ? A_UNDERLINE : 0, fenselected, 0);
	game_draw();

	/* Should be done at the end because of refreshes. */
	analysisframe_draw();
	analysis_draw();
	moves_draw();
	wrefresh(mainwin.win);
	refreshed = 1;
}

void mainwin_resize(void) {
	wresize(mainwin.win, LINES - 8, COLS - 10);
	mvwin(mainwin.win, 5, 4);

	mainwin_draw();
}

void mainwin_init(void) {
	pos_from_fen(&posa, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	field_init(&fen, mainwin.win, 43, 2, 78, &fen_filter, NULL);
	posd = posa;
}

void set_position(const struct position *pos) {
	posd = posa = *pos;
	nmove = 0;
	selectedmove = -1;
	shownmove = 0;
	fenselected = 0;
	fen.error = 0;
	selectedsquare = -1;
	reset_analysis();
}

int prompt_promotion(int square) {
	if (flipped)
		square = 63 - square;
	int file = square % 8;
	int rank = square / 8;
	int up = rank > 4;
	WINDOW *win = newwin(22, 12, 5 + !up * 20, 4 + 10 * file);
	keypad(win, TRUE);
	draw_fill(win, &cs.border, 1, 1, 20, 10, NULL);
	set_color(win, &cs.bordershadow);
	mvwhline(win, 0, 1, ACS_HLINE, 10);
	mvwvline(win, 1, 0, ACS_VLINE, 20);
	mvwaddch(win, 0, 0, up && file == 0 ? ACS_ULCORNER : file == 0 ? ACS_LTEE : up ? ACS_TTEE : ACS_ULCORNER);

	set_color(win, &cs.border);
	mvwhline(win, 21, 1, ACS_HLINE, 10);
	mvwvline(win, 1, 11, ACS_VLINE, 20);
	mvwaddch(win, 21, 11, !up && file == 7 ? ACS_LRCORNER : file == 7 ? ACS_RTEE : !up ? ACS_BTEE : ACS_LRCORNER);

	set_color(win, up && file != 7 ? &cs.bordershadow : &cs.border);
	mvwaddch(win, 0, 11, up && file == 7 ? ACS_URCORNER : file == 7 ? ACS_RTEE : up ? ACS_TTEE : ACS_URCORNER);

	set_color(win, !up && file != 0 ? &cs.border : &cs.bordershadow);
	mvwaddch(win, 21, 0, !up && file == 0 ? ACS_LLCORNER : file == 0 ? ACS_LTEE : !up ? ACS_BTEE : ACS_LLCORNER);

	struct piece p = { 0 };
	p.type = QUEEN;
	piece_draw(win, 1 + 15 * !up, 1, &p, &cs.text);
	p.type = KNIGHT;
	piece_draw(win, 1 + 5 + 5 * !up, 1, &p, &cs.text);
	p.type = ROOK;
	piece_draw(win, 1 + 10 - 5 * !up, 1, &p, &cs.text);
	p.type = BISHOP;
	piece_draw(win, 1 + 15 - 15 * !up, 1, &p, &cs.text);

	MEVENT event;
	int promotion = 0;
	if (wgetch(win) == KEY_MOUSE && getmouse(&event) == OK && (event.bstate & BUTTON1_PRESSED) && wenclose(win, event.y, event.x) && wmouse_trafo(win, &event.y, &event.x, FALSE)) {
		if (event.y <= 5)
			promotion = up ? QUEEN : BISHOP;
		else if (event.y <= 10)
			promotion = up ? KNIGHT : ROOK;
		else if (event.y <= 15)
			promotion = up ? ROOK : KNIGHT;
		else
			promotion = up ? BISHOP : QUEEN;
	}

	delwin(win);

	return promotion;
}
