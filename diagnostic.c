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
#include "preprocessor/preprocessor.h"

uint tab_counter = 0;

uint print_lines(const char* diag, uint line)
{
	char* file = strdup(root_file);
	uint filesize = rf_counter;

	if (diag != NULL)
	{
		file = strdup(diag);
		filesize = strlen(diag);
	}
	
	uint c = 1;
	for (uint i = 0; i != filesize; i++)
	{
		if (c == line)
		{
			for (uint t = i; file[t] == '\t'; t++)
				tab_counter++;
		}

		if (diag != NULL && c == line - 1)
		{
			for (;file[i] != '\n'; i++)
				printf("%c", file[i]);
			
			return 0;
		}

		if (line == 1) 
		{
		    printf("\t");
		    for (uint i = 0; file[i] != '\n' && file[i] != '\0'; i++)
		        printf("%c", file[i]);
		
		    return 0;
		}
		
		if (c == line - 1 || c == line)
		{			
			printf("\t");
			
			for (;file[i] != '\n'; i++)
				printf("%c", file[i]);

			printf("\n");
		}

		if (file[i] == '\n')
			c++;
	}

	return 0;
}

void print_caret(uint column)
{
	for (uint i = 0; i < tab_counter; i++)
		printf("~~~~~~~");

	if (column == 1)
	{
		printf("~~~~~~~~");
		printf("^\n");
		return;
	}

	for (uint i = 1; i < column + 8; i++)
		printf("~");

	printf("^\n");
}

void prep_error(const char* source_file, const uint line, const uint column, const PREP_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("prep-err~~> %s:%d:%d\n", source_file, line, column);
	printf("\n");
	switch (ERROR_TYPE)
	{
		case FILE_NOT_EXISTS:
			printf("| File not exists or permission error.\n");
			exit(0);
		case END_SYMBOL:
			// for diagnostic line control
			printf("| Expected '@' at end of file. (Only included files)\n");
			exit(1);
		case END_SYMBOL_WRONG:
			printf("| The '@' symbol is placed only at the end of included files.\n");
			exit(1);
		default:
	}
}

void lexer_error(const uint line, const uint column, const LEXER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("lexer-error~>");
	print_lines(diagnostic_mark, line);
	printf(":%d\n", column);

	print_lines(NULL, line);
	if (line == 1)
		printf("\n");

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
		case MULTIPLE_DOTS:
			printf("| Multiple dots\n");
			exit(1);
		case IDENTIFIER_OVERFLOW:
			printf("| Identifier name overflow. Identifier name must be 256(max) char\n");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

void parser_error(const uint line, const uint column, const PARSER_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("parser-error~>");
	print_lines(diagnostic_mark, line);
	printf(":%d\n", column);

	print_lines(NULL, line);
	if (line == 1)
		printf("\n");
		
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
		case WRONG_CHRREQ:
			printf("| '@' is only needed in included files");
			exit(1);
		case UNEXPECTED:
			printf("| Unexpected keyword\n");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}

void semantic_error(const char* source_file, const uint line, const uint column, const char* scope, const uint scpline,
					const uint scpcolumn, const char* argument, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("semantic-error->");
	print_lines(diagnostic_mark, line);
	printf(":%d:%s\n", column, scope);

	if (argument != NULL)
		printf("         ^~~~~~> %s\n\n", argument);
	else
		printf("\n");

	print_lines(NULL, line);
	if (line == 1)
		printf("\n");

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
		case FILE_NOT_OPEN:
			printf("| File not exists or permission error");
			exit(1);
		default:
			printf("| Unexpected error\n");
			exit(1);
	}
}
/*
void ir_error(const char* source_file, const uint line, const uint column, const IR_LAYER_ERROR_TYPE ERROR_TYPE)
{
	printf("ir-err~~> %s:%d:%d\n", source_file, line, column);
	print_lines(source_file, line);
	print_caret(column);

	// ...
}
*/
