#include "guish/parse.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <scallop-lang/lex.h>

#include "guish/process.h"

typedef struct libadt_lptr lptr_t;
typedef struct libadt_const_lptr const_lptr_t;
typedef struct scallop_lang_lex lex_t;

#define c_word scallop_lang_classifier_word
#define c_word_separator scallop_lang_classifier_word_separator
#define c_unexpected scallop_lang_classifier_unexpected
#define c_end scallop_lang_classifier_end
#define c_curly_block scallop_lang_classifier_curly_block
#define c_curly_block_end scallop_lang_classifier_curly_block_end
#define c_square_block scallop_lang_classifier_square_block
#define c_square_block_end scallop_lang_classifier_square_block_end
#define lex_next scallop_lang_lex_next

#define lptr_raw libadt_lptr_raw
#define lptr_index libadt_lptr_index
#define const_lptr libadt_const_lptr

#define LPTR_WITH LIBADT_LPTR_WITH

typedef struct word_list_s {
	lptr_t word;
	struct word_list_s *next;
} word_list_t;

typedef struct {
	int srv;
	int cli;
} connection_t;

static connection_t new_connection(void)
{
	/*
	 * A lot of this is to work-around the fact
	 * that the connection the shell gets to the
	 * Wayland compositor is an already-connected
	 * file descriptor.
	 *
	 * We have to "emulate" that for contexts below
	 * directly connecting to the compositor, such as
	 *
	 * foo { bar; baz; }
	 *       ^^^--^^^- these guys
	 */
	const connection_t error = { -1, -1 };
	int srv = socket(AF_UNIX, SOCK_STREAM, 0);
	if (srv < 0)
		return error;

	struct sockaddr_un addr = { .sun_family = AF_UNIX };
	socklen_t addrlen = sizeof(addr);

	// Depends on autobinding, is this Linux-specific?
	if (bind(srv, (struct sockaddr*)&addr, sizeof(addr.sun_family)) < 0)
		return error;

	if (getsockname(srv, (struct sockaddr*)&addr, &addrlen) < 0) {
		close(srv);
		return error;
	}

	int cli = socket(AF_UNIX, SOCK_STREAM, 0);
	if (cli < 0) {
		close(srv);
		return error;
	}

	if (connect(cli, (struct sockaddr*)&addr, addrlen) < 0) {
		close(srv);
		return error;
	}

	return (connection_t){ .srv = srv, .cli = cli };
}

static lex_t parse_statement_impl(
	lex_t lex,
	int guisrv,
	word_list_t *previous,
	int count
);
static lex_t parse_script_impl(
	lex_t lex,
	int guisrv
);

static lex_t parse_statement_impl(
	lex_t lex,
	int guisrv,
	word_list_t *previous,
	int count
)
{
	int guicli = -1;

	lex = lex_next(lex);

	if (lex.type == c_unexpected) {
		return (lex_t){ 0 };
	} else if (lex.type == c_word_separator) {
		return parse_statement_impl(
			lex,
			guisrv,
			previous,
			count
		);
	} else if (lex.type == c_word) {
		ssize_t size = scallop_lang_lex_normalize_word(
			lex.value,
			(lptr_t){ 0 }
		);
		if (size < 0)
			return (lex_t){ 0 };

		lex_t result = { 0 };
		LPTR_WITH(word, (size_t)size + 1, sizeof(char)) {
			scallop_lang_lex_normalize_word(
				lex.value,
				word
			);

			word_list_t current = {
				word,
				previous,
			};

			result = parse_statement_impl(
				lex,
				guisrv,
				&current,
				++count
			);
		}
		return result;
	} else if (lex.type == c_curly_block) {
		connection_t connection = new_connection();

		if (connection.srv == -1)
			return (lex_t){ 0 };

		// confusing naming: "guicli" is the
		// SERVER's file descriptor for CLIENTs
		guicli = connection.srv;

		// TODO: don't rely on the script always having
		// a statement separator after a closing curly bracket
		lex = parse_script_impl(lex, connection.cli);
		if (lex.type != c_curly_block_end) {
			close(guicli);
			return (lex_t){ 0 };
		}
	} else /* if (lex.type == c_statement_separator) and friends */ {
		int error = -1;
		LPTR_WITH(statement, (size_t)count, sizeof(lptr_t)) {
			for (--count; count >= 0; --count) {
				const_lptr_t *item = lptr_raw(
					lptr_index(statement, count)
				);
				*item = const_lptr(previous->word);
				previous = previous->next;
			}
			error = fork_wrapper(const_lptr(statement), guisrv, guicli);
		}
		if (error < 0)
			return (lex_t){ 0 };
	}

	return lex;
}

static lex_t parse_script_impl(lex_t lex, int guisrv)
{
	lex_t next = lex_next(lex);

	const bool end
		= next.type == c_end
		|| next.type == c_curly_block_end
		|| next.type == c_square_block_end
		|| next.type == c_unexpected;

	if (end)
		return next;

	if (next.type == c_word) {
		next = parse_statement_impl(lex, guisrv, NULL, 0);
		return parse_script_impl(next, guisrv);
	}

	// A curly bracket block at the top level is identical
	// to no curly bracket block
	if (next.type == c_curly_block) {
		lex_t end_block = parse_script_impl(next, guisrv);
		if (end_block.type != c_curly_block_end) {
			end_block.type = c_unexpected;
			return end_block;
		}
		return parse_script_impl(end_block, guisrv);
	}

	// skips comments, word/statement separators etc.
	return parse_script_impl(next, guisrv);
}

int guish_parse_script(const_lptr_t script, int guisrv)
{
	lex_t last = parse_script_impl(
		scallop_lang_lex_init(script),
		guisrv
	);

	return last.type == c_end ? 0 : -1;
}
