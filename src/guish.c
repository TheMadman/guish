#include "guish/guish.h"

#include "guish/parse.h"
#include "guish/process.h"

#include <stdio.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <locale.h>
#include <wait.h>

#include <libadt/lptr.h>
#include <libadt/util.h>
#include <scallop-lang/token.h>

// gettext placeholder
#define _(str) str

#define perror_exit(str) perror(str), exit(EXIT_FAILURE)

#define MAX libadt_util_max

typedef struct scallop_lang_token token_t;
typedef struct libadt_lptr lptr_t;
typedef struct libadt_const_lptr const_lptr_t;
typedef struct parse_statement_command command_t;
typedef struct parse_statement statement_t;

#if 0
int parse_statements(const_lptr_t file)
{
	statement_t statement = parse_statement(file);
	command_t command = statement.command;

	if (command.statement.length == 0)
		return 0;

	for (ssize_t i = 0; i < command.statement.length; i++) {
		const_lptr_t strptr = parse_statement_get_arg(command, i);
		printf("Arg: %*s\n", (int)strptr.length, (const char*)strptr.buffer);
	}

	int pid = fork();
	switch (pid) {
		case -1:
			return EXIT_FAILURE;
		case 0: {
			struct wl_display *display = wl_display_connect(NULL);
			if (!display)
				perror_exit(_("Failed to initialize Wayland display"));
			int display_fd = wl_display_get_fd(display);

			return fork_wrapper(command, display_fd, display_fd);
		}
		default: {
			int futures = parse_statements(statement.remaining);
			int wstatus = 0;
			waitpid(pid, &wstatus, 0);
			return MAX(0, MAX(futures, WEXITSTATUS(wstatus)));
		}
	}
}
#endif

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	if (argc < 2) {
		fprintf(stderr, _("Usage: %s <script-file>\n"), basename(argv[0]));
		return EXIT_FAILURE;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0)
		perror_exit(_("Failed to open file"));

	off_t length = lseek(fd, 0, SEEK_END);

	void *raw_file = mmap(
		NULL,
		(size_t)length,
		PROT_READ,
		MAP_PRIVATE,
		fd,
		0
	);

	if (!raw_file)
		perror_exit(_("Failed to map file"));

	close(fd);

	const_lptr_t file = {
		.buffer = raw_file,
		.size = 1,
		.length = (ssize_t)length,
	};

	if (guish_parse_script(file) < 0)
		perror_exit(_("Failed to execute script"));
}
