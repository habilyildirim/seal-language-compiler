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

int definiton_control(const char* type, const AST current)
{
	if (strcmp(type, "var") == 0)
	{
		for (unsigned int i = 0; i < var_counter; i++)
		{
			if (strcmp(current.var.name, var_buffer[i].var.name) == 0 && 
				(strcmp(current.scope, var_buffer[i].scope) == 0 || 
				strcmp(var_buffer[i].scope, "global") == 0))
				return i;
		}

		for (unsigned int i = 0; i < function_counter; i++)
		{
			if (strcmp(current.var.name, function_buffer[i].function.function_name) == 0 &&
				strcmp(current.scope, "global") == 0)
				return i;
		}
	}

	if (strcmp(type, "function") == 0)
	{
		for (unsigned int i = 0; i < function_counter; i++)
		{
			if (strcmp(current.function.function_name, 
				function_buffer[i].function.function_name) == 0)
				return i;
		}

		for (unsigned int i = 0; i < var_counter; i++)
		{
			if (strcmp(current.function.function_name, var_buffer[i].var.name) == 0 &&
				strcmp(var_buffer[i].scope, "global") == 0)
				return i;
		}
	}

	return -1;
}

/*
unsigned int expr_control(AST current, EXPR* e)
{
	EXPR* expr_fbitc;

	char* type;
	char* name;
	
	switch (current.type)
	{
		case UVAR:
		case VAR:
			type = strdup(current.var.type);
			name = strdup(current.var.name);
			break;
		case FUNCTION:
			type = strdup(current.function.function_type);
			name = strdup(current.function.function_name);
			break;
		case RETURN:
			if (strcmp(current.scope, "global") == 0)
			{
				semantic_error(source_files[0], current.line, current.column, 
								current.scope, current.scpline, current.scpcolumn, 
								NULL, GLOBAL_CANNOTRET);	
			}

			AST retast_ref;
			retast_ref.function.function_name = current.scope;

			type = strdup(function_buffer[definiton_control("function", retast_ref)]
			.function.function_type);

			break;
		default:
	}

	switch (e->type)
	{
    	case NODE_INT_LITERAL:
    		break;
        case NODE_IDENTIFIER:
        	AST expriden_ref;
        	expriden_ref.var.name = e->identifier;

			int index_var = definiton_control("var", expriden_ref);
			int index_func = definiton_control("function", expriden_ref);
					
        	if ((index_var + index_func) == -2)
        	{
        		semantic_error(source_files[0], current.line, current.column,
        						current.scope, current.scpline, current.scpcolumn, 
        						expriden_ref.var.name, UNDEFINED);
        	}

        	if (index_var > -1)
			{
				
			}
			
        	if (index_func > -1)
        	{
        		
        	} 
        	
        	break;
        case NODE_BINARY:
            expr_control(current, e->binary.left);
            expr_control(current, e->binary.right);
            break;
        case NODE_UNARY:

            expr_control(current, e->unary.value);
            break;
		case NODE_CALL:
			AST ref;
			ref.call.callee = e->call.callee;
			definiton_control("function", ref);

            for (unsigned int i = 0; i < e->call.argc; i++)
            	expr_control(current, e->call.args[i]);

        default:
	}

	return 1;
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
			case UVAR:
			case VAR:
				if (definiton_control("var", ast[i]) > -1)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].var.name, REDEFINITION);
				}

			/*
				if (ast[i].var.value != NULL)
					expr_control(ast[i], ast[i].var.value);
			*/

				var_buffer[var_counter] = ast[i];
				var_counter++;
				var_buffer = realloc(var_buffer, sizeof(AST) * var_counter * 2);
				break;
			case FUNCTION:
				if (definiton_control("function", ast[i]) > -1)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].function.function_name, REDEFINITION);
				}

				function_buffer[function_counter] = ast[i];
				function_counter++;
				function_buffer = realloc(function_buffer, sizeof(AST) * function_counter * 2);
				break;
			case RETURN:
			/*
				if (ast[i]._return.value != NULL)
					expr_control(ast[i], ast[i]._return.value);
				else
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									NULL, CANNOT_RETNULLVAL);
				}
			*/
				break;
			case PARSE_ASSIGNMENT:

				/*
					Created AST references because
					define_control uses AST while checking
				*/

				AST assgnmnt_ref;
				assgnmnt_ref.var.name = ast[i].assignment.name;

				if (definiton_control("var", assgnmnt_ref) < 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn, 
									ast[i].assignment.name, UNDEFINED);
				}

				break;
			case CALL:
				AST call_ref;
				call_ref.function.function_name = ast[i].call.callee;

				if (definiton_control("function", call_ref) < 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn, 
									ast[i].call.callee, UNDEFINED);
				}
			default:
		}
	}
}
