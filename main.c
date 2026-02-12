/*

	Seal Compiler

	Copyright (C) 2026 Habil Yıldırım

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <https://www.gnu.org/licenses/>.

*/

/* main.c Was written for testing purposes. Will be replaced later. */

#include "common.h"
#include "lexer.h"
#include "test.c"

#define VERSION "Seal Version - Under"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Seal Error: \033[31mNo input files\033[0m\n");
		return -1;
	}

	if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
	{
		puts(VERSION);
		return 0;
	}

	char out[255];

	strncpy(out, "a.out", 5);
	out[5] = '\0';

	if (strcmp(argv[argc - 2], "-o") == 0 || strcmp(argv[argc - 2], "-output") == 0)
	{
		strncpy(out, argv[argc - 1], strlen(argv[argc - 1]));
		out[strlen(argv[argc - 1])] = '\0';
	}

	_ar sources_list = NULL;
	_arsize sources_list_size = 0;

	/* control the extensions */
	int sources_i_counter = 0;

	for (unsigned int i = 1; i < (unsigned int)argc; i++)
	{
		if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-output") == 0)
			break;

		if (read_f(argv[i]) < 0)
		{
			fprintf(stderr, "Seal Error: \033[31m%s -> file cannot open\033[0m\n", argv[i]);
			return -1;
		}

		if (strlen(argv[i]) < 6 || strcmp(argv[i] + strlen(argv[i]) - 5, ".seal") != 0)
		{
			fprintf(stderr, "Seal Error: \033[31m%s -> wrong input\033[0m\n", argv[i]);
			return -1;
		}

		ar_at(&sources_list, argv[i], sources_i_counter, &sources_list_size);
		sources_i_counter++;
	}

	run_lexer_machine(sources_list);
	print_tokens(1);

	return 0;
}
