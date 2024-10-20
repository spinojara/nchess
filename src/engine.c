#include "engine.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#include "window.h"

void engine_open(struct engineconnection *ec, struct uciengine *ue) {
	int parentchild[2];
	int childparent[2];
	if (pipe(parentchild) || pipe(childparent))
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
			chdir(ue->workingdir);

		execlp(ue->command, ue->command, (char *)NULL);
		exit(-1);
	}

	close(parentchild[0]);
	close(childparent[1]);
	ec->w = fdopen(parentchild[1], "w");
	setbuf(ec->w, NULL);
	int flags = fcntl(childparent[0], F_GETFL, 0);
	fcntl(childparent[0], F_SETFL, flags | O_NONBLOCK);
	ec->r = fdopen(childparent[0], "r");
	setbuf(ec->r, NULL);
}

void engine_close(struct engineconnection *ec) {
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 1000000 };

	fprintf(ec->w, "stop\nquit\n");
	fclose(ec->w);
	fclose(ec->r);
	nanosleep(&ts, NULL);
	if (waitpid(ec->pid, NULL, WNOHANG) == ec->pid)
		return;

	kill(ec->pid, SIGINT);
	nanosleep(&ts, NULL);
	if (waitpid(ec->pid, NULL, WNOHANG) == ec->pid)
		return;

	kill(ec->pid, SIGKILL);
	waitpid(ec->pid, NULL, 0);
}
