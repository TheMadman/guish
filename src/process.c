#include "guish/process.h"

#include <stdlib.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <libadt/util.h>

#include <sys/wait.h>

#define arrlength libadt_util_arrlength
#define arrend libadt_util_arrend

typedef struct {
	int client, server, error;
} sockpair_t;

typedef char buf_t[4096];

static sockpair_t create_socketpair(int domain, int type, int protocol)
{
	int sockpair[2] = { 0 };
	int error = socketpair(domain, type, protocol, sockpair) < 0;
	return (sockpair_t) {
		.client = sockpair[0],
		.server = sockpair[1],
		.error = error,
	};
}

int fork_wrapper(
	struct parse_statement_command command,
	int guiin,
	int guiout
)
{
	sockpair_t sockets = create_socketpair(AF_UNIX, SOCK_STREAM, 0);
	if (sockets.error)
		return -1;

	int pid = fork();
	switch (pid) {
		case -1:
			return -1;
		case 0: {
			close(sockets.server);
			char fdbuf[11] = { 0 };
			snprintf(fdbuf, sizeof(fdbuf), "%d", sockets.client);
			setenv("WAYLAND_SOCKET", fdbuf, 1);
			return exec_command(command);
		}
		default: {
			close(sockets.client);
			char buf[4096] = { 0 };
			ssize_t amount = read(sockets.server, buf, sizeof(buf));
			printf("client sent %ld bytes\n", amount);
			kill(pid, SIGTERM);
			// set up a client socket to the parent
			wait(NULL);
			return 0;
		}
	}
}

int exec_command(struct parse_statement_command command)
{
	// Maybe I should put this logic in libadt somewhere
	char **args = calloc((size_t)command.statement.length + 1, sizeof(char*));
	for (
		char **out = args;
		libadt_const_lptr_in_bounds(command.statement);
		command.statement = libadt_const_lptr_index(command.statement, 1),
		out++
	) {
		const struct libadt_const_lptr
			*current = libadt_const_lptr_raw(command.statement);
		*out = strndup(libadt_const_lptr_raw(*current), (size_t)current->length);
	}

	// I don't care if we leak memory if execvp() fails
	return execvp(*args, args);
}
