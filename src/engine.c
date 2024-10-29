#include "engine.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/wait.h>
#include <poll.h>
#endif

#include "window.h"
#include "timepoint.h"

#ifndef _WIN32
struct arg {
	FILE *w;
	FILE *r;
	struct engineconnection *ec;
};

void *engine_listen(void *arg) {
	FILE *w = ((struct arg *)arg)->w;
	FILE *r = ((struct arg *)arg)->r;
	struct engineconnection *ec = ((struct arg *)arg)->ec;
	free(arg);

	char line[8192];
	int n;
	struct pollfd fd = { .fd = fileno(r), .events = POLLIN };
	while (!engine_error(ec) && (n = poll(&fd, 1, 250)) >= 0) {
		if (n > 0) {
			while (errno = 0, fgets(line, sizeof(line), r)) {
				if (!strncmp(line, "bestmove ", 9)) {
					timepoint_t t = time_now();
					pthread_mutex_lock(&ec->mutex);
					ec->bestmovetime = t;
					snprintf(ec->bestmove, 128, "%s", line);
					pthread_mutex_unlock(&ec->mutex);
				}
				else if (!strcmp(line, "readyok\n")) {
					timepoint_t t = time_now();
					pthread_mutex_lock(&ec->mutex);
					ec->isready = 0;
					ec->readyok = t;
					pthread_mutex_unlock(&ec->mutex);
					continue;
				}
				pthread_mutex_lock(&ec->mutex);
				if (!ec->isready)
					fprintf(w, "%s", line);
				pthread_mutex_unlock(&ec->mutex);
			}
			if (errno && errno != EWOULDBLOCK)
				engine_seterror(ec, EE_CRASHED);
		}
		pthread_mutex_lock(&ec->mutex);
		if (ec->isready && time_since(ec->isready) >= TPPERSEC * 2)
			ec->error = EE_READYOK;
		pthread_mutex_unlock(&ec->mutex);
	}
	fclose(w);
	fclose(r);

	if (!engine_error(ec))
		engine_seterror(ec, EE_TERMINATED);

	return NULL;
}
#endif

void engine_open(struct engineconnection *ec, const struct uciengine *ue) {
#ifndef _WIN32
	memset(ec->name, 0, 17);
	snprintf(ec->name, 17, "%s", ue->name);
	int parentchild[2];
	int childparent[2];
	int parentparent[2];
	if (pipe(parentchild) || pipe(childparent) || pipe(parentparent))
		die("error: failed to open pipe");

	if ((ec->pid = fork()) == -1)
		die("error: failed to fork");

	if (ec->pid == 0) {
		close(parentchild[1]);
		close(childparent[0]);
		dup2(parentchild[0], STDIN_FILENO);
		dup2(childparent[1], STDOUT_FILENO);
		int fd = open("/dev/null", O_WRONLY);
		dup2(fd, STDERR_FILENO);

		if (ue->workingdir[0])
			if (chdir(ue->workingdir))
				exit(-1);

		execlp(ue->command, ue->command, (char *)NULL);
		exit(-1);
	}

	close(parentchild[0]);
	close(childparent[1]);
	ec->r = fdopen(parentparent[0], "r");
	ec->w = fdopen(parentchild[1], "w");
	setbuf(ec->w, NULL);
	struct arg *arg = malloc(sizeof(*arg));
	arg->w = fdopen(parentparent[1], "w");
	arg->r = fdopen(childparent[0], "r");
	arg->ec = ec;
	int flags;
	flags = fcntl(parentparent[0], F_GETFL, 0);
	fcntl(parentparent[0], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(childparent[0], F_GETFL, 0);
	fcntl(childparent[0], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(parentparent[1], F_GETFL, 0);
	fcntl(parentparent[1], F_SETFL, flags | O_NONBLOCK);

	ec->error = ec->isready = ec->readyok = ec->bestmovetime = 0;

	setbuf(arg->w, NULL);
	pthread_mutex_init(&ec->mutex, 0);
	pthread_create(&ec->tid, NULL, &engine_listen, arg);
#endif
}

int engine_close(struct engineconnection *ec) {
#ifndef _WIN32
	int error = engine_error(ec);

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000000 };

	fprintf(ec->w, "stop\nquit\n");
	fclose(ec->w);
	fclose(ec->r);
	nanosleep(&ts, NULL);
	if (waitpid(ec->pid, NULL, WNOHANG) == ec->pid)
		goto join;

	kill(ec->pid, SIGINT);
	nanosleep(&ts, NULL);
	if (waitpid(ec->pid, NULL, WNOHANG) == ec->pid)
		goto join;

	kill(ec->pid, SIGKILL);
	waitpid(ec->pid, NULL, 0);

join:;
	if (!engine_error(ec))
		engine_seterror(ec, EE_TERMINATED);
	pthread_join(ec->tid, NULL);
	pthread_mutex_destroy(&ec->mutex);
	return error;
#else
	return 0;
#endif
}

timepoint_t engine_readyok(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	timepoint_t readyok = ec->readyok;
	pthread_mutex_unlock(&ec->mutex);
	return readyok;
#else
	return 0;
#endif
}

int engine_isready(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	ec->readyok = 0;
	ec->bestmovetime = 0;
	ec->isready = time_now();
	pthread_mutex_unlock(&ec->mutex);
	return fprintf(ec->w, "isready\n") < 0;
#else
	return 1;
#endif
}

int engine_hasbestmove(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	int ret = ec->bestmovetime != 0;
	pthread_mutex_unlock(&ec->mutex);

	return ret;
#else
	return 0;
#endif
}

int engine_awaiting(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	int ret = ec->isready || ec->readyok;
	pthread_mutex_unlock(&ec->mutex);
	return ret;
#else
	return 0;
#endif
}

void engine_reset(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	ec->readyok = ec->isready = 0;
	pthread_mutex_unlock(&ec->mutex);
#endif
}

void engine_seterror(struct engineconnection *ec, int err) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	ec->error = err;
	pthread_mutex_unlock(&ec->mutex);
#endif
}

int engine_error(struct engineconnection *ec) {
#ifndef _WIN32
	pthread_mutex_lock(&ec->mutex);
	int error = ec->error;
	pthread_mutex_unlock(&ec->mutex);
	return error;
#else
	return 0;
#endif
}
