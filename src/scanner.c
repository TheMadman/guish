#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include <sys/mman.h>
#include <libadt.h>
#include <descent-xml.h>

typedef struct libadt_lptr ptr_t;
typedef struct libadt_const_lptr cptr_t;
#define allocated libadt_const_lptr_allocated
#define str libadt_str
#define lit libadt_str_literal
#define cptr libadt_const_lptr
#define streq libadt_const_lptr_equal
#define inbounds libadt_const_lptr_in_bounds
#define index libadt_const_lptr_index
#define raw libadt_const_lptr_raw

typedef struct descent_xml_lex token_t;
#define parse descent_xml_parse
#define element_end descent_xml_classifier_element_close
#define unexpected descent_xml_classifier_unexpected
#define eof descent_xml_classifier_eof
#define init descent_xml_lex_init
#define valid descent_xml_validate_document

#define perror_exit(msg) perror(msg), exit(EXIT_FAILURE)

#define FILE_PREFIX \
	"/*\n" \
	" * GuiSH - A Shell for Wayland Programs\n" \
	" * This file was generated automatically by guish-scanner\n" \
	" */\n" \
	"\n" \
	"#ifndef GUISH_PROTO\n" \
	"#define GUISH_PROTO\n" \
	"\n" \
	"#ifdef __cplusplus\n" \
	"extern \"C\" {\n" \
	"#endif\n" \
	"\n"

#define FILE_SUFFIX \
	"\n" \
	"#ifdef __cplusplus\n" \
	"} // extern \"C\"\n" \
	"#endif\n" \
	"\n" \
	"#endif // GUISH_PROTO\n"


// TODO: this entire program breaks if the XML is UTF-16

ssize_t interface_count = 0;

token_t interface_member_handler(
	token_t token,
	cptr_t name,
	cptr_t attributes,
	bool empty,
	void *context
)
{
	// TODO: handle <description>, <enum>, <request>, <event>
	if (empty)
		return token;
	while (token.type != element_end) {
		if (token.type == unexpected)
			return token;
		token = parse(token, NULL, NULL, NULL);
	}
	return token;
}

token_t interface_handler(
	token_t token,
	cptr_t name,
	cptr_t attributes,
	bool empty,
	void *context
)
{
	token_t result = token;
	if (empty || !streq(name, lit("interface")))
		return result;

	for (; inbounds(attributes); attributes = index(attributes, 2)) {
		const cptr_t *a_name = raw(attributes);
		const cptr_t *a_value = raw(index(attributes, 1));

		if (!streq(*a_name, lit("name")))
			continue;

		printf(
			"#define %.*s %d\n",
			a_value->length,
			raw(*a_value),
			interface_count++
		);
		while (result.type != element_end) {
			if (result.type == unexpected)
				return result;
			result = parse(
				result,
				interface_member_handler,
				NULL,
				NULL
			);
		}
	}
	return result;
}

ptr_t map_file(const char *const path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return (ptr_t) { 0 };

	size_t file_size = lseek(fd, 0, SEEK_END);
	return (ptr_t) {
		.buffer = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0),
		.size = sizeof(char),
		.length = file_size,
	};
}

void print_context(token_t token)
{
	char
		*before = token.value.buffer,
		*after = token.value.buffer;

	ssize_t
		before_lines = 0,
		after_lines = 0;
	for (; before > token.script.buffer + 1; before--) {
		if (before_lines > 2)
			break;
		if (before[-1] == '\n')
			before_lines++;
	}

	for (; after < token.script.buffer + (token.script.length - 1); after++) {
		if (after_lines > 2)
			break;
		if (after[1] == '\n')
			after_lines++;
	}

	fprintf(stderr, "%.*s\n", after - before, before);
}

int main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	argv++, argc--;
	puts(FILE_PREFIX);
	for (; *argv; argv++) {
		ptr_t file = map_file(*argv);
		cptr_t ptr = cptr(file);

		if (!allocated(ptr))
			perror_exit("map_file");

		token_t token = init(ptr);

		if (!valid(token)) {
			fprintf(stderr, "script invalid\n");
			exit(EXIT_FAILURE);
		}

		while (token.type != eof) {
			token = parse(token, interface_handler, NULL, NULL);
			if (token.type == unexpected) {
				print_context(token);
				fprintf(stderr, "Parsing XML failed\n");
				exit(EXIT_FAILURE);
			}
		}

		munmap(raw(ptr), file.size);
	}
	puts(FILE_SUFFIX);
}
