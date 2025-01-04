#include "guish/process.h"

#include <stdlib.h>
#include <unistd.h>
#include <wayland-client-core.h>

#include <stdio.h>
#include <sys/wait.h>

int fork_wrapper(
	struct parse_statement_command command,
	int guiin,
	int guiout
)
{
	int pid = fork();
	switch (pid) {
		case -1:
			perror("fork");
			return -1;
		case 0:
			// make command run with custom wayland thingy somehow
			return exec_command(command);
		default:
			// set up a client socket to the parent
			wait(NULL);
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
		*out = libadt_const_lptr_raw(*current);
	}

	return execvp(*args, args);
}
