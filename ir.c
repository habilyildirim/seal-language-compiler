/*
 Seal Compiler - IR Layer
 Copyright (C) 2026 Habil Yıldırım

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.
 If not, see <https://www.gnu.org/licenses/>.
*/

#include "parser.h"
#include "ir.h"
#include "semantic.h"
#include <limits.h>
#include <string.h>

uint tmp_counter = 0;
uint irlabel_counter = 0;

typedef struct
{
	char* arg;
} 
argsary;

IR* ir = NULL;
uint ir_counter = 0;

char* general_scope = NULL;

void emit_ret(char* type, char* value)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_RET;
	ir[ir_counter].ret.type = type;
	ir[ir_counter].ret.value = value;
	ir_counter++;
}

void emit_tmp_singleop(OP_TYPE optype, char* type, char* name, char* left, char* right, char* oper, bool lo_key)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_TMP;
	ir[ir_counter].tmp.oper = oper;
	ir[ir_counter].tmp.type = type;
	ir[ir_counter].tmp.name = name;
	ir[ir_counter].tmp.left = left;
	ir[ir_counter].tmp.lo_key = lo_key;


	if(right != NULL)
		ir[ir_counter].tmp.right = right;

	ir[ir_counter].tmp.op = optype;
	ir_counter++;
}

void emit_alloc(char* var_name, char* type)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_ALLOCATE;	
	ir[ir_counter].allocate.var_name = var_name;
	ir[ir_counter].allocate.type = type;
	ir_counter++;
}

void emit_store(char* var_name, char* type, char* value)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_STORE;
	ir[ir_counter].store.var_name = var_name;
	ir[ir_counter].store.type = type;
	ir[ir_counter].store.value = value;
	ir_counter++;
}

char* get_vartype(char* var_name)
{
	for (uint i = 0; i < var_counter; i++)
	{
		if (strcmp(var_buffer[i].var.name, var_name) == 0 && 
			(strcmp(var_buffer[i].scope, general_scope) == 0 || strcmp(var_buffer[i].scope, "global") == 0)) 
		{
			return var_buffer[i].var.type;
		}
	}

	return NULL;
}

char* type_control(const char* str)
{
	if (strchr(str, '.') || strchr(str, 'e') || strchr(str, 'E'))
	{
		const double d = strtod(str, NULL);
		const float f = (float)d;

		if ((double)f == d)
			return "float";
			
		return "double";
	}

	const long long val = atoll(str);

	if (val >= SCHAR_MIN && val <= SCHAR_MAX)
		return "i8";
	else if (val >= SHRT_MIN && val <= SHRT_MAX)
		return "i16";
	else if (val >= INT_MIN && val <= INT_MAX)
		return "i32";

	return "i64";
}

FILE* ir_source;

char* expr(EXPR* e)
{
	if (!e)
		return NULL;

	switch (e->type)
	{
		case NODE_INT_LITERAL:
		{
			char* result_literal = NULL;
			asprintf(&result_literal, "t%d", tmp_counter);

			fprintf(ir_source, "tmp t%d const", tmp_counter);

			if (!isdigit(e->literal[0]))
			{
				switch (ir[ir_counter - 1].type)
				{
					case TYPE_TMP:
						fprintf(ir_source, " %s %s", ir[ir_counter - 1].tmp.type, 
													 e->literal);

						emit_tmp_singleop(OP_CONST, ir[ir_counter - 1].tmp.type,
													result_literal, e->literal, NULL, NULL, 0);
						break;
					case TYPE_ALLOCATE:
						fprintf(ir_source, " %s %s", ir[ir_counter - 1].allocate.type,
													 e->literal);
						
						emit_tmp_singleop(OP_CONST, ir[ir_counter - 1].allocate.type,
													result_literal, e->literal, NULL, NULL, 0);
						break;
					case TYPE_STORE:
						fprintf(ir_source, " %s %s", ir[ir_counter - 1].store.type, 
													 e->literal);

						emit_tmp_singleop(OP_CONST, ir[ir_counter - 1].store.type,
													result_literal, e->literal, NULL, NULL, 0);
						break;
					default:
				}
			}
			else
			{
				fprintf(ir_source, " %s %s\n", type_control(e->literal), 
											  e->literal);

				emit_tmp_singleop(OP_CONST, type_control(e->literal),
											result_literal, e->literal, NULL, NULL, 0);
			}

			tmp_counter++;
			return result_literal;
		}

		case NODE_IDENTIFIER:
		{
			char* result_identifier = NULL;
			asprintf(&result_identifier, "t%d", tmp_counter);

			char* type = get_vartype(e->identifier);
			fprintf(ir_source, "tmp t%d load %s %s\n", tmp_counter, type, e->identifier);

			emit_tmp_singleop(OP_LOAD, type, result_identifier, e->identifier, NULL, NULL, 0);

			tmp_counter++;
			return result_identifier;
		}

		case NODE_BINARY:
		{
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
			OP_TYPE _oper;

			char* type = ir[ir_counter - 1].tmp.type;
			bool lo_key = 0;
			if (strcmp(e->binary.op, "*") == 0) 
			{
				oper = "mul";
				_oper = OP_MUL;
			}

			if (strcmp(e->binary.op, "%") == 0) 
			{
				oper = "srem";
				_oper = OP_MOD;
			}
				
			if (strcmp(e->binary.op, "/") == 0) 
			{
				oper = "sdiv";
				_oper = OP_DIV;
			}

			if (strcmp(e->binary.op, "-") == 0) 
			{
				oper = "sub";
				_oper = OP_SUB;
			}

			if (strcmp(e->binary.op, "+") == 0) 
			{
				oper = "add";
				_oper = OP_ADD;
			}

			if (strcmp(e->binary.op, "==") == 0) 
			{
				oper = "icmp eq";
				_oper = OP_CMP_EQ;
				lo_key = 1;
			}
			
			if (strcmp(e->binary.op, "!=") == 0) 
			{
				oper = "icmp ne";
				_oper = OP_CMP_NE;
				lo_key = 1;
			}

			if (strcmp(e->binary.op, ">") == 0) 
			{
				oper = "icmp sgt";
				_oper = OP_CMP_GT;
				lo_key = 1;
			}

			if (strcmp(e->binary.op, "<") == 0) 
			{
				oper = "icmp slt";
				_oper = OP_CMP_LT;
				lo_key = 1;
			}

			if (strcmp(e->binary.op, ">=") == 0) 
			{
				oper = "icmp sge";
				_oper = OP_CMP_GE;
				lo_key = 1;
			}

			if (strcmp(e->binary.op, "<=") == 0) 
			{
				oper = "icmp sle";
				_oper = OP_CMP_LE;
				lo_key = 1;
			}

			tmp_counter++;
			asprintf(&result_binary, "t%d", tmp_counter - 1);

			if (lo_key)
				type = "i1";

			fprintf(ir_source, "tmp %s t%d %s", type, tmp_counter, oper);
			fprintf(ir_source, " %s", left);
			fprintf(ir_source, " %s\n", right);

			emit_tmp_singleop(_oper, type, result_binary,
				left, right, oper, lo_key);
			return result_binary;
		}

		case NODE_UNARY:
		{
			char* unary_value = expr(e->unary.value);
			char* result_unary = NULL;

			asprintf(&result_unary, "t%d", tmp_counter);

			fprintf(ir_source, "tmp t%d neg %s\n", tmp_counter, unary_value);

			tmp_counter++;
			return result_unary;
		}

		case NODE_NOT:
		{
			char* not_value = expr(e->unary.value);
			char* result_not = NULL;

			asprintf(&result_not, "t%d", tmp_counter);

			emit_tmp_singleop(OP_NOT, ir[ir_counter - 1].tmp.type, result_not,
				not_value, NULL, NULL, 0);

			fprintf(ir_source, "tmp %s not %s %s\n", result_not, 
				ir[ir_counter - 1].tmp.type,
				not_value);

			tmp_counter++;
			return result_not;
		}

		case NODE_CALL:
		{
			char* result_call = NULL;

			argsary* argsary_ref = malloc(sizeof(argsary) * e->call.argc);

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
		}
		default:
	}

	return NULL;
}

void ir_main()
{
	ir = malloc(sizeof(IR) * 512);
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
					fprintf(ir_source, "alloc %s %s\n", ast[i].var.name, 
						ast[i].var.type);
					fprintf(ir_source, "store %s %s\n", ast[i].var.name, 
						expr(ast[i].var.value));
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
			{
				if (!return_key)
					emit_ret("i8", "0");

				tmp_counter = 0;

				fprintf(ir_source, "func %s:%s ", ast[i].function.type,
												  ast[i].function.name);

				ir[ir_counter].type = TYPE_FUNC;
				ir[ir_counter].func.type = ast[i].function.type;
				ir[ir_counter].func.name = ast[i].function.name;
				ir[ir_counter].func.argc = ast[i].function.argc;

				uint l = 0;

				for (; l < ast[i].function.argc; l++)
				{
					fprintf(ir_source, " %s:%s", ast[i].function.args[l].name,
												 ast[i].function.args[l].type);

					ir[ir_counter].func.args[l].type = ast[i].function.args[l].type;
					ir[ir_counter].func.args[l].name = ast[i].function.args[l].name;
				}
				fprintf(ir_source, "\n");

				general_scope = ast[i].function.name;
				ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
				ir_counter++;
				break;
			}

			case CALL:
			{
				argsary* call_arg = NULL;
				call_arg = malloc(sizeof(argsary) * ast[i].call.argc);

				for (uint l = 0; l < ast[i].call.argc; l++)
				{
					call_arg[l].arg = expr(ast[i].call.args[l]);
					call_arg = realloc(call_arg, sizeof(call_arg) + 1);
				}

				fprintf(ir_source, "tmp t%d call %s",
					tmp_counter,
					ast[i].call.callee);

				for (uint l = 0; l < ast[i].call.argc; l++)
					fprintf(ir_source, " %s", call_arg[l].arg);

				fprintf(ir_source, "\n");
				break;
			}

			case RETURN:
			{
				char* result = expr(ast[i]._return.value);

				fprintf(ir_source, "ret");
				if (1) //!isdigit(result[0]))
				{
					fprintf(ir_source, " %s %s\n", ir[ir_counter - 1].tmp.type, result);
					emit_ret(ir[ir_counter - 1].tmp.type, result);
					break;
				}

				fprintf(ir_source, " %s %s\n", type_control(result), result);
				emit_ret(type_control(result), result);
				break;
			}

			case UVAR:
			case VAR:
			{
				fprintf(ir_source, "alloc %s %s\n", ast[i].var.name, ast[i].var.type);
				emit_alloc(ast[i].var.name, ast[i].var.type);

				char* result = expr(ast[i].var.value);
				fprintf(ir_source, "store %s %s %s\n", ast[i].var.name, ast[i].var.type, result);
				emit_store(ast[i].var.name, ast[i].var.type, result);
				break;
			}

			case PARSE_ASSIGNMENT:
			{
				char* result = expr(ast[i].assignment.value);
				fprintf(ir_source, "store %s %s %s\n", ast[i].assignment.name, ast[i].assignment.type, result);
				emit_store(ast[i].var.name, ast[i].assignment.type, result);
				break;
			}

			case JUMPER:
			{
				fprintf(ir_source, "jmp if_true %s %s\n",
							expr(ast[i].jumper.condition),
							ast[i].jumper.label);
				break;
			}

			case LABEL:
			{
				fprintf(ir_source, "label %s\n", ast[i].label.name);
				break;
			}
			default:
		}

		if (ast[i].type != RETURN)
			return_key = 0;
		else
			return_key = 1;
	}

	if (!return_key)
		fprintf(ir_source, "ret i32 0\n");

	fclose(ir_source);
}
