#include "guish/process.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <libadt/util.h>

#include <sys/wait.h>

#define arrlength libadt_util_arrlength
#define arrend libadt_util_arrend

#define GUISRV_FILENO 3
#define GUICLI_FILENO 4

int fork_wrapper(
	struct libadt_const_lptr statement,
	int guisrv,
	int guicli
)
{
	int pid = fork();
	switch (pid) {
		case -1:
			return -1;
		case 0: {
			// TODO: Maybe we shouldn't do stdin/stdout-like
			// things here, but I'm not smart enough for
			// a better solution
			if (dup2(guisrv, GUISRV_FILENO) == -1)
				exit(EXIT_FAILURE);

			// There isn't always a client to connect
			// to, and I think it's better if programs
			// trying just check if GUICLI_FILENO is
			// valid and handle it themselves
			if (
				guicli >= 0
				&& dup2(guicli, GUICLI_FILENO) == -1
			)
				exit(EXIT_FAILURE);
			const char wayland_socket_value[] = { '0' + guisrv, '\0' };
			setenv("WAYLAND_SOCKET", wayland_socket_value, 1);
			exec_command(statement);
			// we only get here if execvp() errors
			exit(EXIT_FAILURE);
		}
		default: {
			// do we need anything else here?
			return pid;
		}
	}
}

int exec_command(struct libadt_const_lptr statement)
{
	// Maybe I should put this logic in libadt somewhere
	char **args = calloc((size_t)statement.length + 1, sizeof(char*));
	for (
		char **out = args;
		libadt_const_lptr_in_bounds(statement);
		statement = libadt_const_lptr_index(statement, 1),
		out++
	) {
		const struct libadt_const_lptr
			*current = libadt_const_lptr_raw(statement);
		*out = strndup(libadt_const_lptr_raw(*current), (size_t)current->length);
	}

	// I don't care if we leak memory if execvp() fails
	return execvp(*args, args);
}
