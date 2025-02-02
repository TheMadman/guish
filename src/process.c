#include "guish/process.h"

#include "guish/guish.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libadt/util.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h> // perror
#include <errno.h>

#define arrlength libadt_util_arrlength
#define arrend libadt_util_arrend

#define _STR(PARAM) #PARAM
#define STR(PARAM) _STR(PARAM)

// temp gettext wrapper
#define _(str) str

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
			// Wayland expects one connection <-> one process,
			// so we replace guisrv with a new socket
			// connected to the same peer
			struct sockaddr_un addr = { 0 };
			socklen_t addrlen = sizeof(addr);
			if (getpeername(guisrv, (struct sockaddr*)&addr, &addrlen) < 0) {
				perror(_("getpeername failure"));
				exit(EXIT_FAILURE);
			}

			guisrv = socket(AF_UNIX, SOCK_STREAM, 0);
			if (guisrv < 0) {
				perror(_("Creating new socket"));
				exit(EXIT_FAILURE);
			}
			if (connect(guisrv, (struct sockaddr*)&addr, addrlen) < 0) {
				perror(_("Connecting to wayland"));
				exit(EXIT_FAILURE);
			}

			// TODO: Maybe we shouldn't do stdin/stdout-like
			// things here, but I'm not smart enough for
			// a better solution
			if (dup2(guisrv, GUISRV_FILENO) == -1) {
				perror(_("guisrv dup2"));
				exit(EXIT_FAILURE);
			}

			// There isn't always a client to connect
			// to, and I think it's better if programs
			// trying just check if GUICLI_FILENO is
			// valid and handle it themselves
			if (
				guicli >= 0
				&& dup2(guicli, GUICLI_FILENO) == -1
			) {
				perror(_("guicli dup2"));
				exit(EXIT_FAILURE);
			}
			setenv("WAYLAND_SOCKET", STR(GUISRV_FILENO), 1);
			exec_command(statement);
			// we only get here if execvp() errors
			perror(_("Failed to execute command"));
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
