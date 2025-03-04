#include "guish/parse.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <scallop-lang/token.h>

#include "guish/process.h"

typedef struct libadt_lptr lptr_t;
typedef struct libadt_const_lptr const_lptr_t;
typedef struct scallop_lang_token token_t;

#define lex_word scallop_lang_lex_word
#define lex_word_separator scallop_lang_lex_word_separator
#define lex_unexpected scallop_lang_lex_unexpected
#define lex_end scallop_lang_lex_end
#define lex_curly_block scallop_lang_lex_curly_block
#define lex_curly_block_end scallop_lang_lex_curly_block_end
#define lex_square_block scallop_lang_lex_square_block
#define lex_square_block_end scallop_lang_lex_square_block_end
#define token_next scallop_lang_token_next

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

static token_t parse_statement_impl(
	token_t token,
	int guisrv,
	word_list_t *previous,
	int count
);
static token_t parse_script_impl(
	token_t token,
	int guisrv
);

static token_t parse_statement_impl(
	token_t token,
	int guisrv,
	word_list_t *previous,
	int count
)
{
	int guicli = -1;

	token = token_next(token);

	if (token.type == lex_unexpected) {
		return (token_t){ 0 };
	} else if (token.type == lex_word_separator) {
		return parse_statement_impl(
			token,
			guisrv,
			previous,
			count
		);
	} else if (token.type == lex_word) {
		ssize_t size = scallop_lang_token_normalize_word(
			token.value,
			(lptr_t){ 0 }
		);
		if (size < 0)
			return (token_t){ 0 };

		token_t result = { 0 };
		LPTR_WITH(word, (size_t)size + 1, sizeof(char)) {
			scallop_lang_token_normalize_word(
				token.value,
				word
			);

			word_list_t current = {
				word,
				previous,
			};

			result = parse_statement_impl(
				token,
				guisrv,
				&current,
				++count
			);
		}
		return result;
	} else if (token.type == lex_curly_block) {
		connection_t connection = new_connection();

		if (connection.srv == -1)
			return (token_t){ 0 };

		// confusing naming: "guicli" is the
		// SERVER's file descriptor for CLIENTs
		guicli = connection.srv;

		// TODO: don't rely on the script always having
		// a statement separator after a closing curly bracket
		token = parse_script_impl(token, connection.cli);
		if (token.type != lex_curly_block_end) {
			close(guicli);
			return (token_t){ 0 };
		}
	} else /* if (token.type == lex_statement_separator) and friends */ {
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
			return (token_t){ 0 };
	}

	return token;
}

static token_t parse_script_impl(token_t token, int guisrv)
{
	token_t next = token_next(token);

	const bool end
		= next.type == lex_end
		|| next.type == lex_curly_block_end
		|| next.type == lex_square_block_end
		|| next.type == lex_unexpected;

	if (end)
		return next;

	if (next.type == lex_word) {
		next = parse_statement_impl(token, guisrv, NULL, 0);
		return parse_script_impl(next, guisrv);
	}

	// A curly bracket block at the top level is identical
	// to no curly bracket block
	if (next.type == lex_curly_block) {
		token_t end_block = parse_script_impl(next, guisrv);
		if (end_block.type != lex_curly_block_end) {
			end_block.type = lex_unexpected;
			return end_block;
		}
		return parse_script_impl(end_block, guisrv);
	}

	// skips comments, word/statement separators etc.
	return parse_script_impl(next, guisrv);
}

int guish_parse_script(const_lptr_t script, int guisrv)
{
	token_t last = parse_script_impl(
		scallop_lang_token_init(script),
		guisrv
	);

	return last.type == lex_end ? 0 : -1;
}
