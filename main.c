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
#include "codegen.h"
#include "test.c"
#include "diagnostic.h"

#define VERSION "Seal Version - Under\n"

arg_flags arg_flagref;

void parse_arg(uint argc, char** argv, char* *source, char* *output_name)
{
	for (uint i = 0; i < argc - 1; i++)
	{
		if (strcmp(argv[i], "--Compile") == 0 || strcmp(argv[i], "-c") == 0)
			(*source) = argv[i + 1];
		if (strcmp(argv[i], "--Output") == 0 || strcmp(argv[i], "-o") == 0)
			(*output_name) = argv[i + 1];

		if (strcmp(argv[i], "--Save") == 0 || strcmp(argv[i], "-s") == 0) 
		{
			if (strcmp(argv[i + 1], "asm") == 0)
			{
				arg_flagref.asm_flag = 1;
				continue;
			}
			
			if (strcmp(argv[i + 1], "obj") == 0)
			{
				arg_flagref.obj = 1;
				continue;
			}

			if (strcmp(argv[i + 1], "llvm") == 0)
			{
				arg_flagref.llvm = 1;
				continue;
			}

			if (strcmp(argv[i + 1], "ir") == 0)
			{
				arg_flagref.ir = 1;
				continue;	
			}

			if (strcmp(argv[i + 1], "all") == 0)
			{
				arg_flagref.ir = 1;
				arg_flagref.llvm = 1;
				arg_flagref.obj = 1;
				arg_flagref.asm_flag = 1;
				continue;	
			}

			cli_error("Wrong or missing save argument");
		}
	}

	if ((*source) == NULL)
		cli_error("Missing source file");
}

int main(int argc, char *argv[])
{
	arg_flagref.asm_flag = 0;
	arg_flagref.obj = 0;
	arg_flagref.llvm = 0;
	arg_flagref.ir = 0;

	char* output_name = NULL;
	char* sourcefile_path = NULL;

	parse_arg(argc, argv, &sourcefile_path, &output_name);
	pp_main(&sourcefile_path);
	lexer_main(root_file, rf_counter, sourcefile_path); 
	parser_main();
	semantic_main();
	ir_main(sourcefile_path);
	codegen_main(output_name);

	return 0;
}
