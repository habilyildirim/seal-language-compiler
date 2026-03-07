/*

	Seal Compiler - Semantic Layer

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

#include "lexer.h"
#include "common.h"
#include "parser.h"
#include "diagnostic.h"

AST* var_buffer;
unsigned int var_counter = 0;

AST* function_buffer;
unsigned int function_counter = 0;

unsigned int definiton_control(const char* type, const AST current)
{
	if (strcmp(type, "var") == 0)
	{
		for (unsigned int i = 0; i < var_counter; i++)
		{
			if (strcmp(current.var.name, var_buffer[i].var.name) == 0 && 
				(strcmp(current.scope, var_buffer[i].scope) == 0 || 
				strcmp(var_buffer[i].scope, "global") == 0))
				return 1;
		}
	}

	if (strcmp(type, "function") == 0)
	{
		for (unsigned int i = 0; i < function_counter; i++)
		{
			if (strcmp(current.function.function_name, 
				function_buffer[i].function.function_name) == 0)
				return 1;
		}
	}

	return 0;
}

/*
unsigned int expr_control(const char* type, EXPR* e)
{
	switch(e->type)
	{
    	case NODE_INT_LITERAL:
    		if (strcmp(type, "int") != 0)
    			return 0;

    		break;
        case NODE_IDENTIFIER:
        	

        	return 0;
        case NODE_BINARY:
        	if (strcmp(type, "int") != 0)
        		return 0;

            expr_control("int", e->binary.left);
            expr_control("int", e->binary.right);
            break;
        case NODE_UNARY:
        	if (strcmp(type, "int") != 0)
        		return 0;

            expr_control("int", e->unary.value);
            break;
		case NODE_CALL:
			AST ref;
			ref.call.callee = e->call.callee;
			definiton_control("call", ref, 1);

            for (unsigned int i = 0; i < e->call.argc; i++)
            	expr_control("int", e->call.args[i]);

        default:
	}

	return 1;
}

void type_control(const char* type, const AST current)
{

}
*/

void semantic_main()
{
	var_buffer = malloc(sizeof(AST) * 2);
	function_buffer = malloc(sizeof(AST) * 2);

	for (unsigned int i = 0; i < ast_counter; i++)
	{
		switch (ast[i].type)
		{
			case VAR:
				if (definiton_control("var", ast[i]))
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, ast[i].scope, ast[i].scpline,
                                                             ast[i].scpcolumn, ast[i].var.name, REDEFINITION);
				}

				var_buffer[var_counter] = ast[i];
				var_counter++;
				var_buffer = realloc(var_buffer, sizeof(AST) * var_counter * 2);
				break;
			case FUNCTION:
				if (definiton_control("function", ast[i]))
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, ast[i].scope, ast[i].scpline,
					                           ast[i].scpcolumn, ast[i].function.function_name, REDEFINITION);	
				}

				function_buffer[function_counter] = ast[i];
				function_counter++;
				function_buffer = realloc(function_buffer, sizeof(AST) * function_counter * 2);
				break;
			case PARSE_ASSIGNMENT:

				/*
					Created AST references because
					define_control uses AST while checking
				*/

				AST assgnmnt_ref;
				assgnmnt_ref.var.name = ast[i].assignment.name;

				if (!definiton_control("var", assgnmnt_ref))
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, ast[i].scope, ast[i].scpline, 
					                                     ast[i].scpcolumn, ast[i].assignment.name, UNDEFINED);
				}

				break;
			case CALL:
				AST call_ref;
				call_ref.function.function_name = ast[i].call.callee;

				if (!definiton_control("function", call_ref))
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, ast[i].scope, ast[i].scpline, 
					                                         ast[i].scpcolumn, ast[i].call.callee, UNDEFINED);
				}

			default:
		}
	}
}
