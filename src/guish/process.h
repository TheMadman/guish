/*
 * Project Name - Project Description
 * Copyright (C) 2024
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

#ifndef GUISH_PROCESS
#define GUISH_PROCESS

#include "parse-statement.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 */

int fork_wrapper(
	struct parse_statement_command command,
	int guiin,
	int guiout
);
int exec_command(struct parse_statement_command command);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GUISH_PROCESS
