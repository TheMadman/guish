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

#define lex_word scallop_lang_lex_word
#define lex_word_separator scallop_lang_lex_word_separator
#define lex_statement_separator scallop_lang_lex_statement_separator
#define lex_unexpected scallop_lang_lex_unexpected
#define token_next scallop_lang_token_next
#define const_lptr libadt_const_lptr

typedef struct command_s {
	const_lptr_t command;
	const_lptr_t args;
} command_t;

typedef struct word_list_s {
	lptr_t word;
	struct word_list_s *next;
} word_list_t;

const_lptr_t get_arg(const_lptr_t args, ssize_t index)
{
	return *(const_lptr_t*)
		libadt_const_lptr_raw(
		libadt_const_lptr_index(
		args, index
	));
}

command_t handle_statement(token_t token, word_list_t *previous, int count)
{
	token = token_next(token);
	if (token.type == lex_unexpected) {
		return (command_t) { 0 };
	} else if (token.type == lex_word) {
		ssize_t size = scallop_lang_token_normalize_word(token.value, (lptr_t){ 0 });
		if (size < 0)
			return (command_t) { 0 };
		LIBADT_LPTR_WITH(word, (size_t)size, sizeof(char)) {
			scallop_lang_token_normalize_word(token.value, word);
			word_list_t current = {
				word,
				previous,
			};
			return handle_statement(token, &current, ++count);
		}
	} else if (token.type == lex_word_separator) {
		return handle_statement(token, previous, count);
	} else /* if (token.type == lex_statement_separator) and others */ {
		/*
		 * The word_list_t *next is built back-to-front, with the
		 * _last_ word in the statement
		 */
		LIBADT_LPTR_WITH(args, (size_t)count, sizeof(const_lptr_t)) {
			// All words but the first go on the argument list
			// The first word is treated as the command, hence
			// count >= 1
			for (--count; count >= 0; --count) {
				const_lptr_t *item =
					libadt_lptr_raw(
					libadt_lptr_index(
					args, count
				));
				*item = const_lptr(previous->word);
				previous = previous->next;
			}
			const_lptr_t command = *(const_lptr_t*)libadt_lptr_raw(args);
			return (command_t) {
				.command = command,
				.args = const_lptr(args),
			};
		}
	}
}

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

	token_t token = scallop_lang_token_init(file);

	command_t command = handle_statement(token, NULL, 0);

	printf("Command: %*s\n", (int)command.command.length, (const char*)command.command.buffer);

	for (ssize_t i = 0; i < command.args.length; i++) {
		const_lptr_t strptr = get_arg(command.args, i);
		printf("Arg: %*s\n", (int)strptr.length, (const char*)strptr.buffer);
	}
}
