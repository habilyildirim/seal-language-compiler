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
#include "common.h"

uint tmp_counter = 0;
uint irlabel_counter = 0;

char* expr(EXPR* e)
{
    if (!e) 
    	return NULL;

    switch (e->type)
    {
        case NODE_INT_LITERAL:
        	char* result_literal = NULL;
        	asprintf(&result_literal, "t%d", tmp_counter);
        	printf("tmp t%d const i32 %s\n", tmp_counter, e->literal);

        	tmp_counter++;
            return result_literal;
        case NODE_IDENTIFIER:
        	char* result_identifier = NULL;
        	asprintf(&result_identifier, "t%d", tmp_counter);
        	printf("tmp t%d load i32 %s\n", tmp_counter, e->identifier);

        	tmp_counter++;
            return result_identifier;
        case NODE_BINARY:
        	char* result_binary = NULL;
        	if (strcmp(e->binary.op, "&&") == 0)
        	{
        		char* left = expr(e->binary.left);
        		if (left == NULL)
        			asprintf(&left, "t%d", tmp_counter - 1);
        		printf("jmp if_false %s L%d_false\n", left, irlabel_counter);
        		
        		char* right = expr(e->binary.right);
        		if (right == NULL)
        			asprintf(&right, "t%d", tmp_counter - 1);
        		printf("jmp if_false %s L%d_false\n", right, irlabel_counter);

        		printf("tmp t%d const i32 1\n", tmp_counter);
        		printf("jmp L%d_end\n", irlabel_counter);
        		printf("label L%d_false\n", irlabel_counter);
        		printf("tmp t%d const i32 0\n", tmp_counter);
        		printf("label L%d_end\n", irlabel_counter);
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
        		printf("jmp if_true %s L%d_true\n", left, irlabel_counter);
        		
        		char* right = expr(e->binary.right);
        		if (right == NULL)
        			asprintf(&right, "t%d", tmp_counter - 1);
        		printf("jmp if_true %s L%d_true\n", right, irlabel_counter);

        		printf("tmp t%d const i32 0\n", tmp_counter);
        		printf("jmp L%d_end\n", irlabel_counter);
        		printf("label L%d_true\n", irlabel_counter);
        		printf("tmp t%d const 1\n", tmp_counter);
        		printf("label L%d_end\n", irlabel_counter);
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

			printf("tmp t%d %s", tmp_counter, oper);
			printf(" %s", left);
			printf(" %s\n", right);
			tmp_counter++;

			asprintf(&result_binary, "t%d", tmp_counter - 1);
			return result_binary;
        case NODE_UNARY:
			char* unary_value = expr(e->unary.value);
            char* result_unary = NULL;
            asprintf(&result_unary, "t%d", tmp_counter);
            printf("tmp t%d neg %s\n", tmp_counter, unary_value);

            tmp_counter++;
            return result_unary;
        case NODE_NOT:
        	char* not_value = expr(e->unary.value);
            char* result_not = NULL;
            asprintf(&result_not, "t%d", tmp_counter);
            printf("tmp t%d not %s\n", tmp_counter, not_value);

            tmp_counter++;
            return result_unary;
		case NODE_CALL:
			char* result_call = NULL;

			typedef struct 
			{
				char* arg;
			}
			argsary;
			argsary* argsary_ref = malloc(sizeof(argsary_ref) + 1);
			
            for (uint i = 0; i < e->call.argc; i++)
            {
            	argsary_ref[i].arg = expr(e->call.args[i]);
            	argsary_ref = realloc(argsary_ref, sizeof(argsary_ref) + 1);
            }
            
            printf("tmp t%d call %s", tmp_counter, e->call.callee);

           	for (uint i = 0; i < e->call.argc; i++)
            	printf(" %s", argsary_ref[i].arg);
            printf("\n");

			asprintf(&result_call, "t%d", tmp_counter);
			tmp_counter++;

            return result_call;
    	default:
    }

    return NULL;
}

void ir_main()
{	
	for (uint i = 0; i < ast_counter; i++)
	{
		if (ast[i].type == RETURN)
			printf("ret %s\n", expr(ast[i]._return.value));	
	}
}
