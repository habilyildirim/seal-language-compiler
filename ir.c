#include "parser.h"
#include "ir.h"
#include "semantic.h"
#include <limits.h>
#include <string.h>

uint tmp_counter = 0;
uint irlabel_counter = 0;

IR* ir = NULL;
uint ir_counter = 0;

char* general_scope = NULL;
IR current_func;

bool is_arg(char* arg)
{
	for (uint c = 0; c < current_func.func.argc; c++)
	{
		if (strcmp(current_func.func.args[c].name, arg) == 0)
			return 1;
	}

	return 0;
}

void emit_ret(char* type, char* value)
{
	char* val = NULL;

	if (is_arg(value))
		asprintf(&val, "%s__addr__", value);
	else
		val = value;

	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_RET;
	ir[ir_counter].ret.type = type;
	ir[ir_counter].ret.value = val;
	ir[ir_counter].scope = general_scope;
	ir_counter++;
}

void emit_tmp_singleop(OP_TYPE optype, char* type, char* name, char* left, char* right, 
	char* size, char* oper, bool lo_key, bool global_key)
{
	char* left_val = NULL;
	if (left != NULL && is_arg(left))
		asprintf(&left_val, "%s__addr__", left);
	else
		left_val = left;

	char* right_val = NULL;

	if (right != NULL && is_arg(right))
		asprintf(&right_val, "%s__addr__", right);
	else
		right_val = right;

	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_TMP;
	ir[ir_counter].tmp.oper = oper;
	ir[ir_counter].tmp.type = type;
	ir[ir_counter].tmp.name = name;
	ir[ir_counter].tmp.left = left_val;
	ir[ir_counter].tmp.lo_key = lo_key;
	ir[ir_counter].tmp.size = size;

	if (right != NULL)
		ir[ir_counter].tmp.right = right_val;
	if (global_key)
		ir[ir_counter].scope = "global";
	else
		ir[ir_counter].scope = general_scope;

	ir[ir_counter].tmp.op = optype;
	ir_counter++;
}

void emit_call(char* tmp, char* callee, char* type, arg* args, uint argc)
{
	arg* args_val = malloc(sizeof(arg) * argc);

	for (uint i = 0; i < argc; i++)
	{
		if (is_arg(args[i].name))
		{
			char* arg_addr = NULL;
			asprintf(&arg_addr, "%s__addr__", args[i].name);
			args_val[i].name = arg_addr;
			args_val[i].type = args[i].type;
			continue;
		}

		args_val[i].name = args[i].name;
		args_val[i].type = args[i].type;
	}

	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_TMP;
	ir[ir_counter].tmp.oper = "call";
	ir[ir_counter].tmp.type = type;
	ir[ir_counter].tmp.name = tmp;
	ir[ir_counter].tmp.callee = callee;

	if (argc == 0)
		ir[ir_counter].tmp.args = args;
	else
		ir[ir_counter].tmp.args = args_val;

	ir[ir_counter].tmp.argc = argc;
	ir[ir_counter].scope = "global";
	ir[ir_counter].tmp.lo_key = 0;
	ir[ir_counter].tmp.op = OP_CALL;
	ir[ir_counter].tmp.left = NULL;
	ir[ir_counter].tmp.right = NULL;
	ir_counter++;
}

void emit_alloc(char* var_name, char* type, bool global_key, char* size)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_ALLOCATE;
	ir[ir_counter].allocate.var_name = var_name;
	ir[ir_counter].allocate.type = type;
	ir[ir_counter].allocate.size = size;
	
	if (global_key)
		ir[ir_counter].scope = "global";
	else
		ir[ir_counter].scope = general_scope;

	ir_counter++;
}

void emit_label(char* label_name)
{
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_LABEL;
	ir[ir_counter].label.label_name = label_name;
	ir[ir_counter].scope = general_scope;
	ir_counter++;
}

void emit_jumper(char* condition, char* label)
{
	char* val = NULL;
	if (is_arg(condition))
		asprintf(&val, "%s__addr__", condition);
	else
		val = condition;

	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_JUMP;
	ir[ir_counter].jump.condition = val;
	ir[ir_counter].jump.label = label;
	ir_counter++;
}

void emit_store(char* var_name, char* type, char* value, char* size, bool global_key, bool arg_key)
{
	char* val = NULL;
	if (is_arg(var_name))
		asprintf(&val, "%s__addr__", var_name);
	else
		val = var_name;
		
	ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
	ir[ir_counter].type = TYPE_STORE;
	ir[ir_counter].store.var_name = val;
	ir[ir_counter].store.type = type;
	ir[ir_counter].store.value = value;
	ir[ir_counter].store.arg_key = arg_key;
	ir[ir_counter].store.size = size;

	if (global_key)
		ir[ir_counter].scope = "global";
	else
		ir[ir_counter].scope = general_scope;

	ir_counter++;
}

bool is_local(char* var_name)
{
	for (uint i = 0; i < ir_counter; i++)
	{
		if (ir[i].type == TYPE_ALLOCATE)
		{
			if (strcmp(ir[i].allocate.var_name, var_name) == 0 && 
			strcmp(ir[i].scope, "global") == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}

char* get_vartype(char* var_name)
{
	for (uint i = 0; i < var_counter; i++)
	{
		if (strcmp(var_buffer[i].var.name, var_name) == 0 && 
			(strcmp(var_buffer[i].scope, general_scope) == 0 || strcmp(var_buffer[i].scope, "global") == 0))
			return var_buffer[i].var.type;
	}

	return NULL;
}

char* get_functype(char* function_name)
{
	for (uint i = 0; i < function_counter; i++)
	{
		if (strcmp(function_buffer[i].var.name, function_name) == 0)
			return function_buffer[i].var.type;
	}

	return NULL;
}

char* get_argtype(char* function_name, uint index)
{
	for (uint i = 0; i < function_counter; i++)
	{
		if (strcmp(function_buffer[i].var.name, function_name) == 0)
		{
			uint c = 0;
			for (;c != index; c++){}
			return function_buffer[i].function.args[c].type;
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

typedef struct
{
	char* name;
	EXPR** dims;
	uint dimc; 
}
array;
char* use_array(array ary);

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
							result_literal, e->literal, NULL, NULL, NULL, 0, 0);
						break;
					case TYPE_ALLOCATE:
						fprintf(ir_source, " %s %s", ir[ir_counter - 1].allocate.type, 
							e->literal);
						emit_tmp_singleop(OP_CONST, ir[ir_counter - 1].allocate.type,
							result_literal, e->literal, NULL, NULL, NULL, 0, 0);
						break;
					case TYPE_STORE:
						fprintf(ir_source, " %s %s", ir[ir_counter - 1].store.type, 
							e->literal);
						emit_tmp_singleop(OP_CONST, ir[ir_counter - 1].store.type,
							result_literal, e->literal, NULL, NULL, NULL, 0, 0);
						break;
					default:
				}
			}
			else
			{
				fprintf(ir_source, " %s i64\n", e->literal);
				emit_tmp_singleop(OP_CONST, "i64", result_literal, 
					e->literal, NULL, NULL, NULL, 0, 0);
			}

			tmp_counter++;
			return result_literal;
		}

		case NODE_ARRAY:
		{
			array current;
			current.name = e->array.name;
			current.dims = e->array.dims;
			current.dimc = e->array.dimc;
			char* size = use_array(current);

			char* result_identifier = NULL;
			asprintf(&result_identifier, "t%d", tmp_counter);
			char* type = get_vartype(e->array.name);

			fprintf(ir_source, "tmp t%d load %s %s\n", 
				tmp_counter, type, e->array.name);

			emit_tmp_singleop(OP_LOAD, type, result_identifier,
				e->identifier, NULL, size, "load", 0, 0);

			tmp_counter++;
			return result_identifier;
		}

		case NODE_IDENTIFIER:
		{
			char* result_identifier = NULL;
			asprintf(&result_identifier, "t%d", tmp_counter);
			char* type = get_vartype(e->identifier);

			if (is_local(e->identifier) == 0)
			{
				fprintf(ir_source, "tmp t%d load %s @%s\n",
					tmp_counter, type, e->identifier);
				emit_tmp_singleop(OP_LOAD, type, result_identifier, 
					e->identifier, NULL, NULL, "load", 0, 1);
			}
			else
			{
				fprintf(ir_source, "tmp t%d load %s %s\n", 
					tmp_counter, type, e->identifier);
				emit_tmp_singleop(OP_LOAD, type, result_identifier,
					e->identifier, NULL, NULL, "load", 0, 0);
			}

			tmp_counter++;
			return result_identifier;
		}

		case NODE_BINARY:
		{
			char* result_binary = NULL;

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

			if (strcmp(e->binary.op, "&&") == 0)
			{
				oper = "and";
				_oper = OP_AND;
			}

			if (strcmp(e->binary.op, "||") == 0)
			{
				oper = "or";
				_oper = OP_OR;
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

			asprintf(&result_binary, "t%d", tmp_counter);
			fprintf(ir_source, "tmp %s t%d %s", type, tmp_counter, oper);
			fprintf(ir_source, " %s", left);
			fprintf(ir_source, " %s\n", right);

			emit_tmp_singleop(_oper, type, result_binary,
				left, right, NULL, oper, lo_key, 0);

			tmp_counter++;
			return result_binary;
		}

		case NODE_UNARY:
		{
			char* unary_value = expr(e->unary.value);
			char* result_unary = NULL;

			asprintf(&result_unary, "t%d", tmp_counter);
			fprintf(ir_source, "tmp t%d neg %s\n", tmp_counter, unary_value);

			emit_tmp_singleop(OP_NEG, ir[ir_counter - 1].tmp.type, result_unary, 
				NULL, unary_value, NULL, NULL, 0, 0);

			tmp_counter++;
			return result_unary;
		}

		case NODE_NOT:
		{
			char* not_value = expr(e->unary.value);
			char* result_not = NULL;

			asprintf(&result_not, "t%d", tmp_counter);

			emit_tmp_singleop(OP_NOT, ir[ir_counter - 1].tmp.type, result_not,
				not_value, NULL, NULL, NULL, 0, 0);

			fprintf(ir_source, "tmp %s not %s %s\n", result_not, 
				ir[ir_counter - 1].tmp.type,
				not_value);

			tmp_counter++;
			return result_not;
		}

		case NODE_CALL:
		{
			char* result_call = NULL;
			arg* args = malloc(sizeof(arg) * e->call.argc);
			char* type = get_functype(e->call.callee);

			for (uint i = 0; i < e->call.argc; i++)
			{
				args[i].name = expr(e->call.args[i]);
				args[i].type = get_argtype(e->call.callee, i);
			}
			fprintf(ir_source, "tmp t%d %s call %s", tmp_counter, type, e->call.callee);

			fprintf(ir_source, "(");
			for (uint i = 0; i < e->call.argc; i++)
				fprintf(ir_source, " %s:%s", args[i].name, args[i].type);
			fprintf(ir_source, ")\n");

			asprintf(&result_call, "t%d", tmp_counter);
			emit_call(result_call, e->call.callee, type, args, e->call.argc);

			tmp_counter++;
			return result_call;
		}
		default:
	}

	return NULL;
}

char* use_array(array ary)
{
	uint index_counter = 1;
	char* size = NULL;

	for (uint c = 0; c != ary.dimc; c++)
	{
		char* current = expr(ary.dims[c]);

		if (ary.dimc == 1)
			return current;

		char* dim_value = NULL;
		char* tdim = NULL;
		asprintf(&tdim, "t%d", tmp_counter);
		tmp_counter++;

		if (c == 0)
		{
			asprintf(&dim_value, "t%s_%d", ary.name, index_counter);
			index_counter++;

			emit_tmp_singleop(OP_MUL, "i64", tdim,
				dim_value, current, NULL, "mul", 0, 0);
		}
		else
		{
			emit_tmp_singleop(OP_ADD, "i64", tdim,
				size, current, NULL, "add", 0, 0);

			if (c == ary.dimc - 1)
			{
				size = tdim;
				return size;
			}

			asprintf(&dim_value, "t%s_%d", ary.name, index_counter);
			index_counter++;
			asprintf(&tdim, "t%d", tmp_counter);
			tmp_counter++;

			emit_tmp_singleop(OP_MUL, "i64", tdim, ir[ir_counter - 1].tmp.name, 
				dim_value, NULL, "mul", 0, 0);
		}

		size = tdim;
	}

	return size;
}

void ir_main()
{
	ir = malloc(sizeof(IR) * 512);
	ir_source = fopen("sealir.sir", "wr");

	bool return_key = 1;
	for (uint i = 0; i < ast_counter; i++)
	{
		switch (ast[i].type)
		{
			case FUNCTION:
			{
				if (!return_key && strcmp(ast[i - 1].scope, "global") != 0)
					emit_ret("i8", "0");

				fprintf(ir_source, "func %s:%s ", ast[i].function.type,
					ast[i].function.name);

				ir = realloc(ir, sizeof(IR) * (ir_counter + 1));
				ir[ir_counter].type = TYPE_FUNC;
				ir[ir_counter].func.type = ast[i].function.type;
				ir[ir_counter].func.name = ast[i].function.name;
				ir[ir_counter].func.argc = ast[i].function.argc;

				if (ir[ir_counter].func.argc > 0)
					ir[ir_counter].func.args = malloc(sizeof(arg) * ast[i].function.argc);

				IR arg_addr;
				memset(&arg_addr, 0, sizeof(IR));
				arg_addr.func.args = malloc(sizeof(arg) * ast[i].function.argc);
				uint l = 0;
				for (;l < ast[i].function.argc; l++)
				{
					fprintf(ir_source, " %s:%s", ast[i].function.args[l].name,
						ast[i].function.args[l].type);
					char* arg = NULL;
					asprintf(&arg, "%s__addr__", ast[i].function.args[l].name);

					arg_addr.func.args[l].type = ast[i].function.args[l].type;
					arg_addr.func.args[l].name = arg;

					ir[ir_counter].func.args[l].type = ast[i].function.args[l].type;
					ir[ir_counter].func.args[l].name = ast[i].function.args[l].name;
				}
				fprintf(ir_source, "\n");

				if (ast[i].function.argc == 0)
				{
					memset(&current_func, 0, sizeof(IR));
					memset(&arg_addr, 0, sizeof(IR));
				}
				else
					current_func = ir[ir_counter];

				general_scope = ast[i].function.name;
				ir_counter++;

				for (uint c = 0; c < l; c++)
				{
					fprintf(ir_source, "alloc %s %s\n", arg_addr.func.args[c].name,
						arg_addr.func.args[c].type);
					emit_alloc(arg_addr.func.args[c].name, arg_addr.func.args[c].type, 0, NULL);

					fprintf(ir_source, "store %s %s %s\n", arg_addr.func.args[c].name,
						arg_addr.func.args[c].type, ast[i].function.args[c].name);
					emit_store(arg_addr.func.args[c].name, arg_addr.func.args[c].type, 
						ast[i].function.args[c].name, NULL, 0, 1);
				}

				break;
			}

			case CALL:
			{
				char* result_call = NULL;
				arg* args = malloc(sizeof(arg) * ast[i].call.argc);
				char* type = get_functype(ast[i].call.callee);

				for (uint i = 0; i < ast[i].call.argc; i++)
				{
					args[i].name = expr(ast[i].call.args[i]);
					args[i].type = get_argtype(ast[i].call.callee, i);
				}
				fprintf(ir_source, "tmp t%d %s call %s", tmp_counter, type, ast[i].call.callee);

				fprintf(ir_source, "(");
				for (uint i = 0; i < ast[i].call.argc; i++)
					fprintf(ir_source, " %s:%s", args[i].name, args[i].type);
				fprintf(ir_source, ")\n");

				asprintf(&result_call, "t%d", tmp_counter);
				emit_call(result_call, ast[i].call.callee, type, args, ast[i].call.argc);
				tmp_counter++;
				break;
			}

			case RETURN:
			{
				char* result = expr(ast[i]._return.value);

				fprintf(ir_source, "ret %s %s\n", ir[ir_counter - 1].tmp.type, result);
				emit_ret(ir[ir_counter - 1].tmp.type, result);
				break;
			}

			case UVAR:
			case VAR:
			{
				if (strcmp(ast[i].scope, "global") == 0)
				{
					fprintf(ir_source, "alloc %s %s\n", ast[i].var.name, ast[i].var.type);
					emit_alloc(ast[i].var.name, ast[i].var.type, 1, NULL);
					break;
				}

				char* tmp_dim = NULL;
				for (uint c = 0; ast[i].var.dim_key && c != ast[i].var.dimc; c++)
				{
					char* current = expr(ast[i].var.dims[c]);

					char* dim_value = NULL;
					asprintf(&dim_value, "t%s_%d", ast[i].var.name, c);
					emit_tmp_singleop(OP_CONST, ast[i].var.type, dim_value,
						current, NULL, NULL, NULL, 0, 0);

					if (!c)
					{
						tmp_dim = current;
						continue;
					}

					char* tdim = NULL;
					asprintf(&tdim, "t%d", tmp_counter);
					tmp_counter++;

					// ast[i].var.type
					emit_tmp_singleop(OP_MUL, "i64", tdim,
						tmp_dim, current, NULL, "mul", 0, 0);

					tmp_dim = tdim;
				}

				fprintf(ir_source, "alloc %s %s size %s\n", ast[i].var.name, ast[i].var.type, tmp_dim);
				emit_alloc(ast[i].var.name, ast[i].var.type, 0, tmp_dim);
				char* result = expr(ast[i].var.value);
				emit_store(ast[i].var.name, ast[i].var.type, result, NULL, 0, 0);

				fprintf(ir_source, "store %s %s %s\n", ast[i].var.name, ast[i].var.type, result);
				break;
			}

			case PARSE_ASSIGNMENT:
			{
				char* result = expr(ast[i].assignment.value);

				if (!is_local(ast[i].assignment.name))
				{
					fprintf(ir_source, "store @%s %s %s\n", ast[i].assignment.name,
						ast[i].assignment.type, result);
					emit_store(ast[i].assignment.name, ast[i].assignment.type, result, NULL, 1, 0);
					break;
				}

				if (ast[i].assignment.dim_key)
				{
					array current;
					current.name = ast[i].assignment.name;
					current.dims = ast[i].assignment.dims;
					current.dimc = ast[i].assignment.dimc;
					char* size = use_array(current);

					fprintf(ir_source, "store %s %s %%%s\n", ast[i].assignment.name,
						ast[i].assignment.type, result);
					emit_store(ast[i].assignment.name, ast[i].assignment.type, result, size, 0, 0);
					break;
				}

				fprintf(ir_source, "store %s %s %%%s\n", ast[i].assignment.name,
					ast[i].assignment.type, result);
				emit_store(ast[i].assignment.name, ast[i].assignment.type, result, NULL, 0, 0);
				break;
			}

			case JUMPER:
			{
				char* result = expr(ast[i].jumper.condition);
				fprintf(ir_source, "br %s %s\n",
					result,
					ast[i].jumper.label);

				emit_jumper(result, ast[i].jumper.label);
				break;
			}

			case LABEL:
			{
				fprintf(ir_source, "label %s\n", ast[i].label.name);
				emit_label(ast[i].label.name);
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
