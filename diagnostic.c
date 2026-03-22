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

unsigned int tab_counter = 0;

void print_lines(const char* source_file, const unsigned int line)
{
	char* file = NULL;
	unsigned int filesize = 0;

	file = open_buffer(source_file, &filesize);

	unsigned int c = 1;

	for (unsigned int i = 0; i != filesize; i++)
	{
		if (c == line)
		{
			for (unsigned int t = i; file[t] == '\t'; t++)
				tab_counter++;
		}

		if (c == line - 1 || c == line)
		{
			printf("\t");

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
	for (unsigned int i = 0; i < tab_counter; i++)
		printf("~~~~~~~");

	if (column == 1)
	{
		printf("~~~~~~~~");
		printf("^\n");
		return;
	}

	for (unsigned int i = 1; i < column + 8; i++)
		printf("~");

	printf("^\n");
}

void lexer_error(const char* source_file, const unsigned int line, const unsigned int column, const LEXER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("lexer-err~~> %s:%d:%d\n\n", source_file, line, column);
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
			printf("| This char is not decimal(number)\n");
			exit(1);
		case IDENTIFIER_OVERFLOW:
			printf("| Identifier name overflow. Identifier name must be 256(max) char\n");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

void parser_error(const char* source_file, const unsigned int line, const unsigned int column, const PARSER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("parser-err~~> %s:%d:%d\n\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	switch (ERROR_TYPE)
	{
		case MISSING_ARG:
			printf("| Missing argument\n");
			exit (1);
		case UNEXPECTED_INCLUDE:
			printf("| Syntax error; INCLUDE Useage ~> INCLUDE <STRING_LITERAL>\n");
			exit (1);
		case UNEXPECTED_MACRO:
			printf("| Syntax error; MACRO Useage ~> MACRO <IDENTIFIER> <*>\n");
			exit(1);
		case UNEXPECTED_FUNCTION:
			printf("| Syntax error; FUNCTION Useage ~>\nff <DATA_TYPE> <FUNCTION_NAME>(<VAR_DEFINATION>, ...)\n{\n	...\n}\n");
			exit(1);
		case NESTED_FUNCTIONS:
			printf("| Nested error; Functions shoulden not be use nested");
			exit(1);
		case UNEXPECTED_CALL:
			printf("| Syntax error; CALL Useage ~> <FUNCTION_NAME>(<PARAMETER>, PARAMETER>....)");
			exit(1);
		case UNEXPECTED_UVAR:
			printf("| Syntax error; UNSIGNED VARIABLE Useage ~> unsigned <DATA_TYPE> <VAR_NAME> ....\n");
			exit(1);
		case UNEXPECTED_VAR:
			printf("| Syntax error; VARIABLE Useage ~> <DATA_TYPE> <VAR_NAME> ....\n");
			exit(1);
		case UNEXPECTED_JUMPER:
			printf("| Syntax error; JUMPER Useage ~> jump (<CONDITION>) <IDENTIFIER>\n");
			exit(1);
		case UNEXPECTED_LABEL:
			printf("| Syntax error; LABEL Useage ~> #<IDENTIFIER>\n");
			exit(1);
		case MISSING_SEMICOLON:
			printf("| Syntax error; Missing semicolon (;)\n");
			exit(1);
		case MISSING_DOLLAR:
			printf("| Syntax error; Missing dollar ($)\n");
			exit(1);
		case WRONG_EXPRESSION:
			printf("| Incorrect expression.\n");
			exit(1);
		case UNEXPECTED:
			printf("| Unexpected keyword\n");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

void semantic_error(const char* source_file, const unsigned int line, const unsigned int column, const char* scope, const unsigned int scpline,
												const unsigned int scpcolumn, const char* argument, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("semantic-err~~> %s:%d:%d\n", source_file, line, column);
	printf("         ^~in~~> %s:%d:%d\n", scope, scpline, scpcolumn);

	if (argument != NULL)
		printf("         ^~~~~~> %s\n\n", argument);
	else
		printf("\n");	

	print_lines(source_file, line);
	print_caret(column);

	switch (ERROR_TYPE)
	{
		case REDEFINITION:
			printf("| of redefinition\n");
			exit(1);
		case UNDEFINED:
			printf("| of undefined\n");
			exit(1);
		case GLOBAL_CANNOTRET:
			printf("| Global cannot be return \n");
			exit(1);
		case CANNOT_RETNULLVAL:
			printf("| Cannot be return to null value \n");
			exit(1);
		case TYPE_ERROR:
			printf("| Invalid expression type \n");
			exit(1);
		case ARGC_MISSMATCH:
			printf("| Argument counter miss match to function defination. \n");
			exit(1);
		case WITHOUT_FUNCTION:
			printf("| State without function");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

/*
void ir_error(const char* source_file, const unsigned int line, const unsigned int column, const IR_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("ir-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	// ...
}
*/
