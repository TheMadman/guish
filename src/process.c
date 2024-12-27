#include "guish/process.h"

#include <stdlib.h>
#include <unistd.h>

int exec_command(struct parse_statement_command command)
{
	char **args = calloc((size_t)command.args.length + 1, sizeof(char*));
	for (
		char **out = args;
		libadt_const_lptr_in_bounds(command.args);
		command.args = libadt_const_lptr_index(command.args, 1),
		out++
	) {
		const struct libadt_const_lptr *current = libadt_const_lptr_raw(command.args);
		*out = libadt_const_lptr_raw(*current);
	}

	return execvp(libadt_const_lptr_raw(command.command), args);
}
