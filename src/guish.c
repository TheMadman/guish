#include "guish/guish.h"

#include "guish/parse-statement.h"
#include "guish/process.h"

#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <locale.h>

#include <libadt/lptr.h>
#include <scallop-lang/token.h>

// gettext placeholder
#define _(str) str

typedef struct scallop_lang_token token_t;
typedef struct libadt_lptr lptr_t;
typedef struct libadt_const_lptr const_lptr_t;
typedef struct parse_statement_command command_t;
typedef struct parse_statement statement_t;

int main(int argc, char **argv, char **envp)
{
	setlocale(LC_ALL, "");
	if (argc < 2) {
		fprintf(stderr, _("Usage: %s <script-file>\n"), basename(argv[0]));
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror(_("Failed to open file"));
		return 1;
	}

	off_t length = lseek(fd, 0, SEEK_END);

	void *raw_file = mmap(
		NULL,
		(size_t)length,
		PROT_READ,
		MAP_PRIVATE,
		fd,
		0
	);

	if (!raw_file) {
		perror(_("Failed to map file"));
		return 1;
	}

	close(fd);

	const_lptr_t file = {
		.buffer = raw_file,
		.size = 1,
		.length = (ssize_t)length,
	};

	statement_t statement = parse_statement(file);
	command_t command = statement.command;

	for (ssize_t i = 0; i < command.statement.length; i++) {
		const_lptr_t strptr = parse_statement_get_arg(command, i);
		printf("Arg: %*s\n", (int)strptr.length, (const char*)strptr.buffer);
	}

	return fork_wrapper(command, -1, -1);
}
