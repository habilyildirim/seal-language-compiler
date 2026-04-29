/*

	Seal Compiler - IR Layer

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

#include "parser.h"
#include "ir.h"
#include "common.h"

uint tmp_counter = 0;
uint irlabel_counter = 0;

typedef struct 
{
	char* arg;
}
argsary;

FILE* ir_source;
char* expr(EXPR* e)
{
    if (!e) 
    	return NULL;

    switch (e->type)
    {
        case NODE_INT_LITERAL:
        	char* result_literal = NULL;
        	asprintf(&result_literal, "t%d", tmp_counter);
        	fprintf(ir_source, "tmp t%d const i32 %s\n", tmp_counter, e->literal);

        	tmp_counter++;
            return result_literal;
        case NODE_IDENTIFIER:
        	char* result_identifier = NULL;
        	asprintf(&result_identifier, "t%d", tmp_counter);
        	fprintf(ir_source, "tmp t%d load i32 %s\n", tmp_counter, e->identifier);

        	tmp_counter++;
            return result_identifier;
        case NODE_BINARY:
        	char* result_binary = NULL;
        	if (strcmp(e->binary.op, "&&") == 0)
        	{
        		char* left = expr(e->binary.left);
        		if (left == NULL)
        			asprintf(&left, "t%d", tmp_counter - 1);
        		fprintf(ir_source, "jmp if_false %s L%d_false\n", left, irlabel_counter);
        		
        		char* right = expr(e->binary.right);
        		if (right == NULL)
        			asprintf(&right, "t%d", tmp_counter - 1);
        		fprintf(ir_source, "jmp if_false %s L%d_false\n", right, irlabel_counter);

        		fprintf(ir_source, "tmp t%d const i32 1\n", tmp_counter);
        		fprintf(ir_source, "jmp L%d_end\n", irlabel_counter);
        		fprintf(ir_source, "label L%d_false\n", irlabel_counter);
        		fprintf(ir_source, "tmp t%d const i32 0\n", tmp_counter);
        		fprintf(ir_source, "label L%d_end\n", irlabel_counter);
        		tmp_counter++;
        		irlabel_counter++;

				asprintf(&result_binary, "t%d", tmp_counter - 1);
        		return result_binary;
        	}

        	if (strcmp(e->binary.op, "||") == 0)
        	{
        		char* left = expr(e->binary.left);
        		if (left == NULL)
        			asprintf(&left, "t%d", tmp_counter - 1);
        		fprintf(ir_source, "jmp if_true %s L%d_true\n", left, irlabel_counter);

        		char* right = expr(e->binary.right);
        		if (right == NULL)
        			asprintf(&right, "t%d", tmp_counter - 1);
        		fprintf(ir_source, "jmp if_true %s L%d_true\n", right, irlabel_counter);

        		fprintf(ir_source, "tmp t%d const i32 0\n", tmp_counter);
        		fprintf(ir_source, "jmp L%d_end\n", irlabel_counter);
        		fprintf(ir_source, "label L%d_true\n", irlabel_counter);
        		fprintf(ir_source, "tmp t%d const 1\n", tmp_counter);
        		fprintf(ir_source, "label L%d_end\n", irlabel_counter);
        		tmp_counter++;
        		irlabel_counter++;

				asprintf(&result_binary, "t%d", tmp_counter - 1);
        		return result_binary;
        	}

			char* left = expr(e->binary.left);
			if (left == NULL)
				asprintf(&left, "t%d", tmp_counter - 1);

            char* right = expr(e->binary.right);
            if (right == NULL)
            	asprintf(&right, "t%d", tmp_counter - 1);

            char* oper = NULL;
			if (strcmp(e->binary.op, "*") == 0) oper = "mul";
			if (strcmp(e->binary.op, "%") == 0) oper = "mod";
			if (strcmp(e->binary.op, "/") == 0) oper = "div";
			if (strcmp(e->binary.op, "-") == 0) oper = "sub";
			if (strcmp(e->binary.op, "+") == 0) oper = "add";

			if (strcmp(e->binary.op, "==") == 0) oper = "cmp_eq";
			if (strcmp(e->binary.op, "!=") == 0) oper = "cmp_ne";
			if (strcmp(e->binary.op, ">") == 0) oper = "cmp_gt";
			if (strcmp(e->binary.op, "<") == 0) oper = "cmp_lt";
			if (strcmp(e->binary.op, ">=") == 0) oper = "cmp_ge";
			if (strcmp(e->binary.op, "<=") == 0) oper = "cmp_le";

			fprintf(ir_source, "tmp t%d %s", tmp_counter, oper);
			fprintf(ir_source, " %s", left);
			fprintf(ir_source, " %s\n", right);
			tmp_counter++;

			asprintf(&result_binary, "t%d", tmp_counter - 1);
			return result_binary;
        case NODE_UNARY:
			char* unary_value = expr(e->unary.value);
            char* result_unary = NULL;
            asprintf(&result_unary, "t%d", tmp_counter);
            fprintf(ir_source, "tmp t%d neg %s\n", tmp_counter, unary_value);

            tmp_counter++;
            return result_unary;
        case NODE_NOT:
        	char* not_value = expr(e->unary.value);
            char* result_not = NULL;
            asprintf(&result_not, "t%d", tmp_counter);
            fprintf(ir_source, "tmp t%d not %s\n", tmp_counter, not_value);

            tmp_counter++;
            return result_not;
		case NODE_CALL:
			char* result_call = NULL;
			argsary* argsary_ref = malloc(sizeof(argsary_ref) * e->call.argc);

            for (uint i = 0; i < e->call.argc; i++)
            {
            	argsary_ref[i].arg = expr(e->call.args[i]);
            	argsary_ref = realloc(argsary_ref, sizeof(argsary) * (i + 1));
            }

            fprintf(ir_source, "tmp t%d call %s", tmp_counter, e->call.callee);

           	for (uint i = 0; i < e->call.argc; i++)
            	fprintf(ir_source, " %s", argsary_ref[i].arg);
            fprintf(ir_source, "\n");

			asprintf(&result_call, "t%d", tmp_counter);
			tmp_counter++;

            return result_call;
    	default:
    }

    return NULL;
}

void ir_main()
{
	ir_source = fopen("sealir.sir", "wr");

	bool global_key = 0;
	for (uint i = 0; i < ast_counter; i++)
	{
		if (strcmp(ast[i].scope, "global") == 0)
		{
			if (global_key == 0 && ast[i].type != FUNCTION)
			{
				fprintf(ir_source, "global:\n");
				global_key = 1;
			}

			switch (ast[i].type)
			{
				case UVAR:
				case VAR:
					fprintf(ir_source, "alloc %s %s\n", ast[i].var.name, ast[i].var.type);
					fprintf(ir_source, "store %s %s\n", ast[i].var.name, expr(ast[i].var.value));
				default:
			}
		}
	}

	bool return_key = 1;
	for (uint i = 0; i < ast_counter; i++)
	{
		if (strcmp(ast[i].scope, "global") == 0 && ast[i].type != FUNCTION)
			continue;

		switch (ast[i].type)
		{
			case FUNCTION:
				if (!return_key)
					fprintf(ir_source, "ret 0\n");

				tmp_counter = 0;
				fprintf(ir_source, "func %s:%s", ast[i].function.name, ast[i].function.type);

				for (uint l = 0; l < ast[i].function.argc; l++)
				{
					fprintf(ir_source, " %s:%s", ast[i].function.args[l].name, 
												 ast[i].function.args[l].type);
				}
				fprintf(ir_source, "\n");

				break;
			case CALL:
				argsary* call_arg = NULL;
				call_arg = malloc(sizeof(call_arg) + 1);

				for (uint l = 0; l < ast[i].call.argc; l++)
				{
					call_arg[l].arg = expr(ast[i].call.args[l]);
					call_arg = realloc(call_arg, sizeof(call_arg) + 1);
				}

				fprintf(ir_source, "tmp t%d call %s", tmp_counter, ast[i].call.callee);
				for (uint l = 0; l < ast[i].call.argc; l++)
					fprintf(ir_source, " %s", call_arg[l].arg);
				fprintf(ir_source, "\n");

				break;
			case RETURN:
				fprintf(ir_source, "ret %s\n", expr(ast[i]._return.value));
				break;
			case UVAR:
			case VAR:
				fprintf(ir_source, "alloc %s %s\n", ast[i].var.name, ast[i].var.type);
				fprintf(ir_source, "store %s %s\n", ast[i].var.name, expr(ast[i].var.value));
				break;
			case JUMPER:
				fprintf(ir_source, "jmp if_true %s %s\n", expr(ast[i].jumper.condition), ast[i].jumper.label);
				break;
			case LABEL:
				fprintf(ir_source, "label %s\n", ast[i].label.name);
				break;
			default:
		}

		if (ast[i].type != RETURN)
			return_key = 0;
		else
			return_key = 1;
	}

	if (!return_key)
		fprintf(ir_source, "ret 0\n");

	fclose(ir_source);
}
