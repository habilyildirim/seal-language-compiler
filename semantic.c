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
uint var_counter = 0;

AST* function_buffer;
uint function_counter = 0;

AST* label_buffer;
uint label_counter = 0;

int definiton_control(const char* type, const AST current)
{
	if (strcmp(type, "var") == 0)
	{
		for (uint i = 0; i < var_counter; i++)
		{
			if (strcmp(current.var.name, var_buffer[i].var.name) == 0 && 
				(strcmp(current.scope, var_buffer[i].scope) == 0 || 
				strcmp(var_buffer[i].scope, "global") == 0))
				return i;
		}

		for (uint i = 0; i < function_counter; i++)
		{
			if (strcmp(current.var.name, function_buffer[i].function.name) == 0 &&
				strcmp(current.scope, "global") == 0)
				return i;
		}
	}

	if (strcmp(type, "function") == 0)
	{
		for (uint i = 0; i < function_counter; i++)
		{
			if (strcmp(current.function.name, 
				function_buffer[i].function.name) == 0)
				return i;
		}

		for (uint i = 0; i < var_counter; i++)
		{
			if (strcmp(current.function.name, var_buffer[i].var.name) == 0 &&
				strcmp(var_buffer[i].scope, "global") == 0)
				return i;
		}
	}

	if (strcmp(type, "label") == 0)
	{
		for (uint i = 0; i < label_counter; i++)
		{
			if (strcmp(current.label.name, label_buffer[i].label.name) == 0 && 
				strcmp(current.scope, label_buffer[i].scope) == 0)
				return i;
		}
	}

	return -1;
}

int is_int(const char* dt)
{
	if (strcmp(dt, "binary") == 0 ||
		strcmp(dt, "integer") == 0 ||
		strcmp(dt, "integer8") == 0 ||
		strcmp(dt, "integer16") == 0 ||
		strcmp(dt, "integer32") == 0 ||
		strcmp(dt, "integer64") == 0 ||
		strcmp(dt, "double") == 0 ||
		strcmp(dt, "float") == 0)
		return 1;

	return 0;
}

void expr_control(AST ast_root, const char* data_type, EXPR* e)
{
	AST ref;
	int index;
	
    switch (e->type)
    {
        case NODE_INT_LITERAL:
            if (is_int(data_type) < 1)
            {
           		semantic_error(source_files[0], ast_root.line,  ast_root.column, 
           						ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
           						NULL, TYPE_ERROR);	
            }

            break;
        case NODE_IDENTIFIER:
        	ref.scope = ast_root.scope;
        	ref.var.name = e->identifier;

			index = definiton_control("var", ref);

			if (index > -1)
			{
				// Int escape
				if (is_int(var_buffer[index].var.type) && is_int(data_type))
					break;

				// Identifier type controls
				if (strcmp(var_buffer[index].var.type, data_type) != 0)
				{
					semantic_error(source_files[0], ast_root.line,  ast_root.column, 
									ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
									e->identifier, TYPE_ERROR);
				}

				break;
			}

			semantic_error(source_files[0], ast_root.line, ast_root.column, 
							ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
							e->identifier, UNDEFINED);
        case NODE_BINARY:
        	if (is_int(data_type) < 1)
        	{
           		semantic_error(source_files[0], ast_root.line,  ast_root.column, 
           						ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
           						NULL, TYPE_ERROR);
        	}

            expr_control(ast_root, data_type, e->binary.left);
            expr_control(ast_root, data_type, e->binary.right);
            break;
        case NODE_UNARY:
        	if (is_int(data_type) < 1)
        	{
        		semantic_error(source_files[0], ast_root.line,  ast_root.column, 
        						ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
        						NULL, TYPE_ERROR);
        	}

            expr_control(ast_root, data_type, e->unary.value);
            break;
		case NODE_CALL:
			if (strcmp(ast_root.scope, "global") == 0)
			{
				semantic_error(source_files[0], ast_root.line, ast_root.column,
								ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
								ast_root.label.name, WITHOUT_FUNCTION);
			}

			ref.scope = ast_root.scope;
			ref.function.name = e->call.callee;

			index = definiton_control("function", ref);

            if (index > -1)
            {
            	// Args type control
            	if (function_buffer[index].function.argc != e->call.argc)
            	{
            		semantic_error(source_files[0], ast_root.line, ast_root.column,
            						ast_root.scope, ast_root.scpline, ast_root.scpcolumn, 
            						NULL, ARGC_MISSMATCH);
            	}

				printf("%d", e->call.argc);
            	if (e->call.argc == 0)
            		break;

            	for (uint i = 0; i < e->call.argc; i++)
            	{
            		if (e->call.args[i] != NULL)
            		{
            			expr_control(ast_root, function_buffer[index].function.args[i].type, 
									  e->call.args[i]);
            		}
            	}

            	// Call return type control
            	if (is_int(function_buffer[index].function.type) && is_int(data_type))
            		break;

            	if (strcmp(function_buffer[index].function.type, data_type) != 0)
            	{
            		semantic_error(source_files[0], ast_root.line,  ast_root.column,
            						ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
            						e->identifier, TYPE_ERROR);
            	}

            	break;
            }

			semantic_error(source_files[0], ast_root.line, ast_root.column,
							ast_root.scope, ast_root.scpline, ast_root.scpcolumn,
							e->identifier, UNDEFINED);
        default:
    }
}

void semantic_main()
{
	var_buffer = malloc(sizeof(AST) * 2);
	function_buffer = malloc(sizeof(AST) * 2);
	label_buffer = malloc(sizeof(AST) * 2);

	for (uint i = 0; i < ast_counter; i++)
	{
		int index = 0;

		switch (ast[i].type)
		{
			case LABEL:
				if (strcmp(ast[i].scope, "global") == 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, WITHOUT_FUNCTION);
				}

				if (definiton_control("label", ast[i]) > -1)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, REDEFINITION);
				}

				label_buffer[label_counter] = ast[i];
				label_counter++;
				label_buffer = realloc(label_buffer, sizeof(AST) * label_counter * 2);
				break;
			case JUMPER:
				if (strcmp(ast[i].scope, "global") == 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column,
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, WITHOUT_FUNCTION);
				}

				expr_control(ast[i], "integer", ast[i].jumper.condition);

				AST jumper_ref;
				jumper_ref.label.name = ast[i].jumper.label;
				jumper_ref.scope = ast[i].scope;

				if (definiton_control("label", jumper_ref) > -1)
					break;

				for (uint c = i;;c++)
				{
					if (ast[c].type == LABEL && strcmp(ast[c].label.name, ast[i].jumper.label) == 0)
						break;

					if(!(c < ast_counter) || strcmp(ast[c].scope, ast[i].scope) != 0)
					{
						semantic_error(source_files[0], ast[i].line, ast[i].column,
										ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
										ast[i].label.name, UNDEFINED);
					}
				}

				break;
			case UVAR:
			case VAR:
				if (definiton_control("var", ast[i]) > -1)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].var.name, REDEFINITION);
				}

				if (ast[i].var.value != NULL)
					expr_control(ast[i], ast[i].var.type, ast[i].var.value);

				var_buffer[var_counter] = ast[i];
				var_counter++;
				var_buffer = realloc(var_buffer, sizeof(AST) * var_counter * 2);
				break;
			case FUNCTION:
				if (definiton_control("function", ast[i]) > -1)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].function.name, REDEFINITION);
				}

				function_buffer[function_counter] = ast[i];
				function_counter++;
				function_buffer = realloc(function_buffer, sizeof(AST) * function_counter * 2);
				break;
			case RETURN:
				if (strcmp(ast[i].scope, "global") == 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, WITHOUT_FUNCTION);
				}

				AST return_ref;
				return_ref.function.name = ast[i].scope;

				char* return_type = function_buffer[definiton_control("function", return_ref)]
									.function.type;
				
				if (ast[i]._return.value != NULL)
					expr_control(ast[i], return_type, ast[i]._return.value);

				break;
			case PARSE_ASSIGNMENT:
				if (strcmp(ast[i].scope, "global") == 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, WITHOUT_FUNCTION);
				}

				/*
					Created AST references because
					define_control uses AST while checking
				*/

				AST assgnmnt_ref;
				assgnmnt_ref.var.name = ast[i].assignment.name;
				assgnmnt_ref.scope = ast[i].scope;
				index = definiton_control("var", assgnmnt_ref); 

				if (index < 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn, 
									ast[i].assignment.name, UNDEFINED);
				}

				/*
					No null expression control because 
					assignment expression cannot be null there	
				*/

				const char* assignment_type = var_buffer[index].var.type;
				
				expr_control(ast[i], assignment_type, ast[i].assignment.value);
				break;
			case CALL:
				if (strcmp(ast[i].scope, "global") == 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn,
									ast[i].label.name, WITHOUT_FUNCTION);
				}

				AST call_ref;
				call_ref.function.name = ast[i].call.callee;
				index = definiton_control("function", call_ref);

				if (index < 0)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column, 
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn, 
									ast[i].call.callee, UNDEFINED);
				}

				if (function_buffer[index].function.argc != ast[i].call.argc)
				{
					semantic_error(source_files[0], ast[i].line, ast[i].column,
									ast[i].scope, ast[i].scpline, ast[i].scpcolumn, 
									NULL, ARGC_MISSMATCH);
				}

				if (ast[i].call.argc == 0)
					break;

				// Args type control
				for (uint c = 0; c < ast[i].call.argc; c++)
				{
					const char* type = function_buffer[index].function.args[c].type;

					if (ast[i].call.args[c]->identifier != NULL)
						expr_control(ast[i], type, ast[i].call.args[c]);
				}
			default:
		}
	}
}
