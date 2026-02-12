/*

	Seal Compiler - Diagnostic

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
#include "diagnostic.h"

void print_lines(const char* source_file, const unsigned int line)
{
	char* file = NULL;
	unsigned int filesize = 0;

	file = open_buffer(source_file, &filesize);

	unsigned int c = 1;

	for (unsigned int i = 0; i != filesize; i++)
	{
		if (c == line - 1 || c == line)
		{
			printf("	");

			for (;file[i] != '\n'; i++)
				printf("%c", file[i]);

			printf("	Line -> ");

			if (c == line - 1)
				printf("%d\n", line - 1);
			else
				printf("%d\n", line);
		}

		if (file[i] == '\n')
			c++;
	}
}

void print_caret(unsigned int column)
{
	if	(column == 1)
	{
		printf("^\n");
		return;
	}

	for (unsigned int i = 1; i < column + 8; i++)
		printf("~");

	printf("^\n");
}

void lexer_error(const char* source_file, const unsigned int line, const unsigned int column, const LEXER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("lexer-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	switch (ERROR_TYPE)
	{
		case INVALID_CHAR:
			printf("| Invalid char\n");
			exit(1);
		case INVALID_ESCAPE:
			printf("| Invalid escape\n");
			exit(1);
		case IS_NOT_HEX:
			printf("| This char is not hexadecimal\n");
			exit(1);
		case IS_NOT_BIN:
			printf("| This char is not binary\n");
			exit(1);
		case IS_NOT_DECIMAL:
			printf("| This chat is not decimal(number)\n");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

void parser_error(const char* source_file, const unsigned int line, const unsigned int column, const PARSER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("parser-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	// ...
}

void semantic_error(const char* source_file, const unsigned int line, const unsigned int column, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("semantic-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	// ...
}

void ir_error(const char* source_file, const unsigned int line, const unsigned int column, const IR_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("ir-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	// ...
}
