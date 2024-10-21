#include "engine.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#include "window.h"

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
	while (fgets(line, sizeof(line), r)) {
		if (!strncmp(line, "bestmove ", 9)) {
			timepoint_t t = time_now();
			pthread_mutex_lock(&ec->mutex);
			ec->bestmovetime = t;
			pthread_mutex_unlock(&ec->mutex);
		}
		fprintf(w, "%s", line);
	}
	fclose(w);
	fclose(r);

	return NULL;
}

void engine_open(struct engineconnection *ec, struct uciengine *ue) {
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
	int flags = fcntl(parentparent[0], F_GETFL, 0);
	fcntl(parentparent[0], F_SETFL, flags | O_NONBLOCK);

	struct arg *arg = malloc(sizeof(*arg));
	arg->w = fdopen(parentparent[1], "w");
	arg->r = fdopen(childparent[0], "r");
	arg->ec = ec;
	setbuf(arg->w, NULL);
	pthread_mutex_init(&ec->mutex, 0);
	pthread_create(&ec->tid, NULL, &engine_listen, arg);
}

void engine_close(struct engineconnection *ec) {
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

join:
	pthread_join(ec->tid, NULL);
	pthread_mutex_destroy(&ec->mutex);
}
