#include "guish/parse-statement.h"

#include <scallop-lang/token.h>

typedef struct parse_statement_command command_t;
typedef struct parse_statement parse_t;
typedef struct libadt_lptr lptr_t;
typedef struct libadt_const_lptr const_lptr_t;
typedef struct scallop_lang_token token_t;

#define lex_word scallop_lang_lex_word
#define lex_word_separator scallop_lang_lex_word_separator
#define lex_statement_separator scallop_lang_lex_statement_separator
#define lex_unexpected scallop_lang_lex_unexpected
#define token_next scallop_lang_token_next
#define const_lptr libadt_const_lptr

typedef struct word_list_s {
	lptr_t word;
	struct word_list_s *next;
} word_list_t;

static void free_word_list(word_list_t *list)
{
	for (; list; list = list->next)
		libadt_lptr_free(list->word);
}

const_lptr_t parse_statement_get_arg(command_t command, ssize_t index)
{
	return *(const_lptr_t*)
		libadt_const_lptr_raw(
		libadt_const_lptr_index(
		command.statement, index
	));
}

static parse_t handle_statement(token_t token, word_list_t *previous, int count)
{
// Not sure if I prefer this over "goto error;"
#define HANDLE_ERROR() do { \
	free_word_list(previous); \
	return (parse_t) { 0 }; \
} while (0)

	token = token_next(token);
	if (token.type == lex_unexpected) {
		HANDLE_ERROR();
	} else if (token.type == lex_word) {
		ssize_t size = scallop_lang_token_normalize_word(
			token.value,
			(lptr_t){ 0 }
		);
		if (size < 0)
			HANDLE_ERROR();

		// + 1 for the null terminator
		// lptr allocation uses calloc
		LIBADT_LPTR_WITH(word, (size_t)size + 1, sizeof(char)) {
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
		LIBADT_LPTR_WITH(statement, (size_t)count, sizeof(const_lptr_t)) {
			for (--count; count >= 0; --count) {
				const_lptr_t *item =
					libadt_lptr_raw(
					libadt_lptr_index(
					statement, count
				));
				*item = const_lptr(previous->word);
				previous = previous->next;
			}
			const_lptr_t remaining = libadt_const_lptr_after(
				token.script,
				token.value
			);
			return (parse_t) {
				.command = {
					.statement = const_lptr(statement),
				},
				.remaining = remaining,
			};
		}
	}

#undef HANDLE_ERROR
}

struct parse_statement parse_statement(struct libadt_const_lptr script)
{
	return handle_statement(scallop_lang_token_init(script), NULL, 0);
}

