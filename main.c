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

#include "common.h"
#include "preprocessor/preprocessor.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "test.c"

#define VERSION "Seal Version - Under\n"

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

	char* sourcefile_path = argv[1];

	pp_main(&sourcefile_path);
	lexer_main(root_file, rf_counter, sourcefile_path);		//print_tokens(1);
	parser_main();										//print_ast(0);
	semantic_main();

	return 0;
}
