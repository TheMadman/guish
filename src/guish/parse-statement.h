/*
 * GuiSH - A Shell for Wayland Programs
 * Copyright (C) 2024  Marcus Harrison
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GUISH_PARSE_STATEMENT
#define GUISH_PARSE_STATEMENT

#include <libadt/lptr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 */

/**
 * \brief A type encoding a single, simple command
 *
 * A simple statement is one or more words, where
 * the first word is a command to run and the remaining words
 * are passed as arguments.
 */
struct parse_statement_command {

	/**
	 * \brief A value storing the command.
	 */
	struct libadt_const_lptr command;

	/**
	 * \brief A pointer to an array of arguments.
	 *
	 * By convention, the zeroth argument is the name of
	 * the command to be run.
	 */
	struct libadt_const_lptr args;
};

/**
 * \brief The return type of parse_statement, containing
 * 	a parse_statement_command and the remaining, unparsed
 * 	script.
 */
struct parse_statement {
	struct parse_statement_command command;
	struct libadt_const_lptr remaining;
};

/**
 * \brief Parses the script, returning a command and the remaining
 * 	script.
 *
 * \param script A pointer to the script to begin parsing.
 *
 * \returns A parse_statement struct, containing a command and
 * 	the remaining script. If a parse error occurs, an object with
 * 	zeroed values is returned. If the script does not start with
 * 	a command, the behaviour is undefined.
 */
struct parse_statement parse_statement(struct libadt_const_lptr script);

/**
 * \brief Given a command from parse_statement, retrieve a pointer
 * 	to the string containing the argument at index.
 *
 * \param command A parsed command result.
 * \param index The index of the argument to get.
 *
 * \returns A pointer to the argument at index.
 */
struct libadt_const_lptr parse_statement_get_arg(
	struct parse_statement_command command,
	ssize_t index
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUISH_PARSE_STATEMENT
