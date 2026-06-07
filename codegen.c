/*

	Seal Compiler - Code Generator
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

#include "codegen.h"
#include "ir.h"
#include "common.h"
#include "diagnostic.h"

FILE* llvm;

IR* tmp_buffer;
uint tmpbuffer_counter = 0;

char* get_tmptype(char* tmp_name)
{
	for (uint i = 0; i < tmpbuffer_counter; i++)
	{
		if (strcmp(tmp_buffer[i].tmp.name, tmp_name) == 0)
			return tmp_buffer[i].tmp.type;
	}

	return NULL;
}

bool get_tmplokey(char* tmp_name)
{
	for (uint i = 0; i < tmpbuffer_counter; i++)
	{
		if (strcmp(tmp_buffer[i].tmp.name, tmp_name) == 0)
			return tmp_buffer[i].tmp.lo_key;
	}

	return 0;
}

void clear(char* out)
{
	if (out == NULL)
	{
		if (arg_flagref.asm_flag)
		{
			if (system("llc a.ll -filetype=asm -o a.s") != 0)
				codegen_error(0, 0, LLVM_CANNOT_COMPILE);
		}

		if (!arg_flagref.obj)
		{
			#ifdef _WIN32
				if (system("del a.obj") != 0)
					printf("OBJ cannot be cleared. Check permisson\n");
			#else
				if (system("rm a.o") != 0)
					printf("OBJ cannot be cleared. Check permisson\n");
			#endif
		}

		if (!arg_flagref.llvm)
		{
			#ifdef _WIN32
				if (system("del a.ll") != 0)
					printf("IR cannot be cleared. Check permisson\n");
			#else
				if (system("rm a.ll") != 0)
					printf("IR cannot be cleared. Check permisson\n");
			#endif
		}

		if (!arg_flagref.ir)
		{
			#ifdef _WIN32
				if (system("del sealir.sir") != 0)
					printf("IR cannot be cleared. Check permisson\n");
			#else
				if (system("rm sealir.sir") != 0)
					printf("IR cannot be cleared. Check permisson\n");
			#endif
		}

		return;
	}

	if (arg_flagref.asm_flag)
	{
		char* command = malloc(256);
		sprintf(command, "llc %s.ll -filetype=asm -o %s.s", out, out);

		if (system(command) != 0)
			codegen_error(0, 0, LLVM_CANNOT_COMPILE);
	}

	if (!arg_flagref.obj)
	{
		char* command = malloc(128);

		#ifdef _WIN32
			sprintf(command, "del %s.obj", out);
		#else
			sprintf(command, "rm %s.o", out);
		#endif

		if (system(command) != 0)
			printf("OBJ cannot be cleared. Check permisson\n");
	}

	if (!arg_flagref.llvm)
	{
		char* command = malloc(128);

		#ifdef _WIN32
			sprintf(command, "del %s.ll", out);
		#else
			sprintf(command, "rm %s.ll", out);
		#endif

		if (system(command) != 0)
			printf("IR cannot be cleared. Check permisson\n");
	}

	if (!arg_flagref.ir)
	{
		#ifdef _WIN32
			if (system("del sealir.sir") != 0)
				printf("IR cannot be cleared. Check permisson\n");
		#else
			if (system("rm sealir.sir") != 0)
				printf("IR cannot be cleared. Check permisson\n");
		#endif
	}
}

void generate_bin(char* out)
{	
	#ifdef _WIN32
		if (out == NULL)
		{
	    	if (system("llc a.ll -filetype=obj -o a.obj") != 0)
	        	codegen_error(0, 0, LLVM_CANNOT_COMPILE);
	    	if (system("lld-link a.obj /OUT:a.exe") != 0)
	        	codegen_error(0, 0, LLVM_CANNOT_LINK);

			clear(out);
	        return;
	    }
	#else
		if (out == NULL)
		{
			if (system("llc a.ll -filetype=obj -o a.o") != 0)
				codegen_error(0, 0, LLVM_CANNOT_COMPILE);
			if (system("ld.lld a.o -o a") != 0)
				codegen_error(0, 0, LLVM_CANNOT_LINK);

			clear(out);
			return;
		}
	#endif

	#ifdef _WIN32
		char* compile = malloc(256);
		sprintf(compile, "llc %s.ll -filetype=obj -o %s.obj", out, out);
	    char* link = malloc(256);
	    sprintf(compile, "lld-link %s.obj /OUT:%s.exe", out, out);

	    if (system(compile) != 0)
	    	codegen_error(0, 0, LLVM_CANNOT_COMPILE);
	    if (system(link) != 0)
	    	codegen_error(0, 0, LLVM_CANNOT_LINK);

		clear(out);
	#else
		char* compile = malloc(256);
		sprintf(compile, "llc %s.ll -filetype=obj -o %s.o", out, out);
		char* link = malloc(256);
		sprintf(link, "ld.lld %s.o -o %s", out, out);

		if (system(compile) != 0)
			codegen_error(0, 0, LLVM_CANNOT_COMPILE);
		if (system(link) != 0)
			codegen_error(0, 0, LLVM_CANNOT_LINK);

		clear(out);
	#endif
}

typedef struct
{
	uint order;
	char* type;
	char* byte;
}
type_order;

const type_order _type_order[] =
{
	{0, "i1", "1"},
	{1, "i8", "1"},
	{2, "i16", "2"},
	{3, "i32", "4"},
	{4, "i64", "8"}
};
#define TYPE_ORDER_SIZE 5

int get_torder(char* type)
{
	if (type == NULL)
		return -1; 

	for (uint i = 0; i != TYPE_ORDER_SIZE; i++)
	{
		if (strcmp(_type_order[i].type, type) == 0)
			return _type_order[i].order;
	}

	return -1; // unsafe will be change later
}

char* get_tbyte(char* type)
{
	if (type == NULL)
		return NULL;

	for (uint i = 0; i != TYPE_ORDER_SIZE; i++)
	{
		if (strcmp(_type_order[i].type, type) == 0)
			return _type_order[i].byte;
	}

	return NULL; // unsafe will be change later
}

void parse_ir()
{
	tmp_buffer = malloc(sizeof(IR) * 2);

	uint storecast_counter = 0;
	uint returncast_counter = 0;
	uint negcast_counter = 0;
	uint leftcast_counter = 0;
	uint rightcast_counter = 0;
	uint jumpcast_counter = 0;
	uint callcast_counter = 0;
	uint ptr_counter = 0;

	bool leftcast_key = 0;
	bool rightcast_key = 0;

	bool scope = 0;
	char* current_functype = NULL;
	for (uint i = 0; i < ir_counter; i++)
	{
		switch (ir[i].type)
		{
			case TYPE_FUNC:
				if (scope)
					fprintf(llvm, "}");
				else
					scope = 1;

				fprintf(llvm, "define %s @%s(", ir[i].func.type, 
												ir[i].func.name);
				// Handle args
				for (uint c = 0; c < ir[i].func.argc; c++)
				{
					if (c != 0)
						fprintf(llvm, " ");

					fprintf(llvm, "%s %%%s", ir[i].func.args[c].type, 
											 ir[i].func.args[c].name);

					if (c != ir[i].func.argc - 1)
						fprintf(llvm, ",");
				}

				fprintf(llvm ,") {\nentry:\n");
				current_functype = ir[i].func.type;
				break;
			case TYPE_TMP:
				// if binary op
				int current_order = 0;
				if (ir[i].tmp.right != NULL && ir[i].tmp.left != NULL)
				{
					char* tmpleft_type = get_tmptype(ir[i].tmp.left);
					char* tmpright_type = get_tmptype(ir[i].tmp.right);
					int left_order = get_torder(tmpleft_type);
					int right_order = get_torder(tmpright_type);
					current_order = get_torder(ir[i].tmp.type);

					if (get_tmplokey(ir[i].tmp.left) && current_order)
					{
						fprintf(llvm, "%%__leftcast__%d = zext i1 %%%s to %s\n",
							leftcast_counter,
							ir[i].tmp.left,
							ir[i].tmp.type);

						leftcast_key = 1;
					} 

					if (get_tmplokey(ir[i].tmp.right) && current_order)
					{
						fprintf(llvm, "%%__rightcast__%d = zext i1 %%%s to %s\n",
							rightcast_counter,
							ir[i].tmp.right,
							ir[i].tmp.type);

						rightcast_key = 1;
					}

					if (!current_order && !ir[i].tmp.lo_key)
					{
						if (left_order < 1 && !leftcast_key)
						{
							fprintf(llvm, "%%__leftcast__%d = zext %s %%%s to i8\n",
								leftcast_counter,
								tmpleft_type,
								ir[i].tmp.left);

							leftcast_key = 1;
						}

						if (left_order > 1 && !leftcast_key)
						{
							fprintf(llvm, "%%__leftcast__%d = trunc %s %%%s to i8\n",
								leftcast_counter,
								tmpleft_type,
								ir[i].tmp.left);

							leftcast_key = 1;
						}

						if (right_order < 1 && !rightcast_key)
						{
							fprintf(llvm, "%%__rightcast__%d = zext %s %%%s to i8\n",
								rightcast_counter,
								tmpright_type,
								ir[i].tmp.right);

							rightcast_key = 1;
						}

						if (right_order > 1 && !rightcast_key)
						{
							fprintf(llvm, "%%__rightcast__%d = trunc %s %%%s to i8\n",
								rightcast_counter,
								tmpright_type,
								ir[i].tmp.right);

							rightcast_key = 1;
						}
					}
					else
					{
						if (current_order > left_order && !leftcast_key)
						{
							fprintf(llvm, "%%__leftcast__%d = zext %s %%%s to %s\n",
								leftcast_counter,
								tmpleft_type,
								ir[i].tmp.left,
								ir[i].tmp.type);

							leftcast_key = 1;
						}

						if (current_order > right_order && !rightcast_key)
						{
							fprintf(llvm, "%%__rightcast__%d = zext %s %%%s to %s\n",
								rightcast_counter,
								tmpright_type,
								ir[i].tmp.right,
								ir[i].tmp.type);

							rightcast_key = 1;
						}

						if (current_order < left_order && !leftcast_key)
						{
							fprintf(llvm, "%%__leftcast__%d = trunc %s %%%s to %s\n",
								leftcast_counter,
								tmpleft_type,
								ir[i].tmp.left,
								ir[i].tmp.type);

							leftcast_key = 1;
						}

						if (current_order < right_order && !rightcast_key)
						{
							fprintf(llvm, "%%__rightcast__%d = trunc %s %%%s to %s\n",
								rightcast_counter,
								tmpright_type,
								ir[i].tmp.right,
								ir[i].tmp.type);

							rightcast_key = 1;
						}
					}
				}

				switch (ir[i].tmp.op)
				{
					case OP_NOT:
					case OP_NEG:
					case OP_CALL:
						break;
					default:
						if (ir[i].tmp.size != NULL)
							break;
						fprintf(llvm, "%%%s = ", ir[i].tmp.name);
				}

				switch (ir[i].tmp.op)
				{
					case OP_CONST:
						char* type = get_tmptype(ir[i].tmp.left);
						if (type == NULL)
						{
							type = ir[i].tmp.type;
							fprintf(llvm, "add %s %s, 0\n", type,
								ir[i].tmp.left);
						}
						else
						{
							fprintf(llvm, "add %s %%%s, 0\n", type,
								ir[i].tmp.left);
						}

						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;
						tmp_buffer[tmpbuffer_counter].tmp.type = type;
						tmp_buffer[tmpbuffer_counter].tmp.lo_key = 0;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					case OP_LOAD:
						char* ptr = NULL;
						if (ir[i].tmp.size != NULL)
						{
							asprintf(&ptr, "ptr%d", ptr_counter);
							ptr_counter++;
							fprintf(llvm, "%%%s = getelementptr %s, %s* %%%s, %s %%%s\n",
								ptr, ir[i].tmp.type,
								ir[i].tmp.type, ir[i].tmp.left,
								get_tmptype(ir[i].tmp.size), ir[i].tmp.size);
							fprintf(llvm, "%%%s = load %s, %s* %%%s, align %s\n", ir[i].tmp.name, 
								ir[i].tmp.type, ir[i].tmp.type, ptr, get_tbyte(ir[i].tmp.type));
						}
						else
						{
							fprintf(llvm, "load %s, %s* ", ir[i].tmp.type,
									ir[i].tmp.type);

							if (strcmp(ir[i].scope, "global") == 0)
								fprintf(llvm, "@%s\n", ir[i].tmp.left);
							else
								fprintf(llvm, "%%%s\n", ir[i].tmp.left);	
						}

						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;
						tmp_buffer[tmpbuffer_counter].tmp.type = ir[i].tmp.type;
						tmp_buffer[tmpbuffer_counter].tmp.lo_key = 0;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					case OP_NOT:
						if (get_tmplokey(ir[i].tmp.left))
						{
							fprintf(llvm, "%%%s = icmp eq i1 %%%s, 0\n",
									ir[i].tmp.name,
									ir[i].tmp.left);
						}
						else
						{
							fprintf(llvm, "%%%s = icmp eq %s %%%s, 0\n",
								ir[i].tmp.name,
								ir[i].tmp.type,
								ir[i].tmp.left);
						}

						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;
						tmp_buffer[tmpbuffer_counter].tmp.type = "i1";
						tmp_buffer[tmpbuffer_counter].tmp.lo_key = 1;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					case OP_NEG:
						if (get_tmplokey(ir[i].tmp.right))
						{
							fprintf(llvm, "%%__negcast__%d = zext i1 %%%s to i8\n", negcast_counter, 
								ir[i].tmp.right);
							fprintf(llvm, "%%%s = sub i8 0, %%__negcast__%d\n", ir[i].tmp.name, 
								negcast_counter);
							tmp_buffer[tmpbuffer_counter].tmp.type = "i8";
							negcast_counter++;
						}
						else
						{
							fprintf(llvm, "%%%s = sub %s 0, %%%s\n", ir[i].tmp.name, ir[i].tmp.type, 
								ir[i].tmp.right);
							tmp_buffer[tmpbuffer_counter].tmp.type = ir[i].tmp.type;
						}

						tmp_buffer[tmpbuffer_counter].tmp.lo_key = 0;
						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					case OP_CALL:
						typedef struct
						{
							char* arg_value;
						} 
						call_args;
						call_args* call_arg = NULL;

						for (uint c = 0; c < ir[i].tmp.argc; c++)
						{
							call_arg = realloc(call_arg, sizeof(call_args) * (c + 1));
							uint current_order = get_torder(ir[i].tmp.args[c].type);							
							char* value_type = get_tmptype(ir[i].tmp.args[c].name);
							uint value_order = get_torder(value_type);
							char* callcast = NULL;

							if (current_order > value_order)
							{
								fprintf(llvm, "%%__callcast__%d = zext %s %%%s to %s\n", callcast_counter, value_type, 
									ir[i].tmp.args[c].name, ir[i].tmp.args[c].type);	
								asprintf(&callcast, "__callcast__%d", callcast_counter);
								call_arg[c].arg_value = callcast;
								callcast_counter++;
								continue;
							}

							if (current_order < value_order)
							{
								fprintf(llvm, "%%__callcast__%d = trunc %s %%%s to %s\n", callcast_counter, value_type, 
									ir[i].tmp.args[c].name, ir[i].tmp.args[c].type);
								asprintf(&callcast, "__callcast__%d", callcast_counter);
								call_arg[c].arg_value = callcast;
								callcast_counter++;
								continue;
							}

							call_arg[c].arg_value = ir[i].tmp.args[c].name;
						}

						fprintf(llvm, "%%%s = call %s @%s(", ir[i].tmp.name, ir[i].tmp.type, ir[i].tmp.callee);
						for (uint c = 0; ir[i].tmp.argc != 0 && c < ir[i].tmp.argc; c++)
						{
							if (c < ir[i].tmp.argc - 1)
							{
								fprintf(llvm, "%s %%%s, ", ir[i].tmp.args[c].type, call_arg[c].arg_value);
								continue;
							}

							fprintf(llvm, "%s %%%s", ir[i].tmp.args[c].type, call_arg[c].arg_value);
						}
						fprintf(llvm, ")\n");

						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;
						tmp_buffer[tmpbuffer_counter].tmp.type = ir[i].tmp.type;
						tmp_buffer[tmpbuffer_counter].tmp.lo_key = ir[i].tmp.lo_key;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					default:
						// Binary Handle
						if (ir[i].tmp.right != NULL)
						{
							if (!current_order && !ir[i].tmp.lo_key)
								fprintf(llvm, "%s i8", ir[i].tmp.oper);
							else
								fprintf(llvm, "%s %s", ir[i].tmp.oper, ir[i].tmp.type);

							if (leftcast_key)
							{
								fprintf(llvm, " %%__leftcast__%d,", leftcast_counter);
								leftcast_key = 0;
								leftcast_counter++;
							}
							else
								fprintf(llvm, " %%%s,", ir[i].tmp.left);

							if (rightcast_key)
							{
								fprintf(llvm, " %%__rightcast__%d\n", rightcast_counter);
								rightcast_key = 0;
								rightcast_counter++;
							}
							else
								fprintf(llvm, " %%%s\n", ir[i].tmp.right);
						}
						tmp_buffer[tmpbuffer_counter].tmp.name = ir[i].tmp.name;

						if (!current_order && !ir[i].tmp.lo_key)
							tmp_buffer[tmpbuffer_counter].tmp.type = "i8";
						else
							tmp_buffer[tmpbuffer_counter].tmp.type = ir[i].tmp.type;
							
						tmp_buffer[tmpbuffer_counter].tmp.lo_key = ir[i].tmp.lo_key;
						tmpbuffer_counter++;
						tmp_buffer = realloc(tmp_buffer, sizeof(IR) * tmpbuffer_counter * 2);
						break;
					}

				break;
			case TYPE_ALLOCATE:
				if (strcmp(ir[i].scope, "global") == 0)
				{
					if (scope)
					{
						fprintf(llvm, "}");
						scope = 0;
					}

					fprintf(llvm, "@%s = global %s 0\n", ir[i].allocate.var_name, ir[i].allocate.type);
				}
				else
				{
					if (ir[i].allocate.size != NULL)
					{
						fprintf(llvm, "%%%s = alloca %s, %s %%%s, align %s\n", ir[i].allocate.var_name,
							ir[i].allocate.type, get_tmptype(ir[i].allocate.size),
							ir[i].allocate.size, get_tbyte(ir[i].allocate.type));
						break;
					}

					fprintf(llvm, "%%%s = alloca %s\n", ir[i].allocate.var_name, ir[i].allocate.type);
				}

				break;
			case TYPE_STORE:
				char* tmp_type = get_tmptype(ir[i].store.value);
				int tmp_order = get_torder(tmp_type);
				const int storevar_order = get_torder(ir[i].store.type);
				char* ptr = NULL;

				if (ir[i].store.size != NULL)
				{
					asprintf(&ptr, "ptr%d", ptr_counter);
					ptr_counter++;
					fprintf(llvm, "%%%s = getelementptr %s, %s* %%%s, %s %%%s\n",
						ptr, ir[i].store.type,
						ir[i].store.type, ir[i].store.var_name,
						get_tmptype(ir[i].store.size), ir[i].store.size);
				}

				if (get_tmplokey(ir[i].store.value))
				{
					tmp_type = "i1";
					tmp_order = 0;
				}

				if (tmp_order == storevar_order || ir[i].store.arg_key)
				{
					if (ir[i].store.size == NULL)
					{
						fprintf(llvm, "store %s %%%s, %s* %%%s\n",
							ir[i].store.type, ir[i].store.value,
							ir[i].store.type, ir[i].store.var_name);
						break;
					}

					fprintf(llvm, "store %s %%%s, %s* %%%s, align %s\n",
						ir[i].store.type, ir[i].store.value,
						ir[i].store.type, ptr, get_tbyte(ir[i].store.type));
					break;
				}

				fprintf(llvm, "%%__storecast__%d =", storecast_counter);
				if (storevar_order > tmp_order)
					fprintf(llvm, " zext");
				else
					fprintf(llvm, " trunc");

				fprintf(llvm, " %s %%%s to %s\n",
					tmp_type,
					ir[i].store.value,
					ir[i].store.type);

				if (ir[i].store.size == NULL)
				{
					fprintf(llvm, "store %s %%__storecast__%d, %s* ",
						ir[i].store.type, storecast_counter, ir[i].store.type);

					if (strcmp(ir[i].scope, "global") == 0)
						fprintf(llvm, "@%s\n", ir[i].store.var_name);
					else
						fprintf(llvm, "%%%s\n", ir[i].store.var_name);
				}
				else
				{
					fprintf(llvm, "store %s %%__storecast__%d, %s* %%%s, align %s\n",
						ir[i].store.type, storecast_counter, ir[i].store.type,
						ptr, get_tbyte(ir[i].store.type));
				}

				storecast_counter++;
				break;
			case TYPE_LABEL:
				fprintf(llvm, "br label %%%s\n", ir[i].label.label_name);
				fprintf(llvm, "%s:\n", ir[i].label.label_name);
				break;
			case TYPE_JUMP:
				char* condition_type = get_tmptype(ir[i].jump.condition);
				if (get_tmplokey(ir[i].jump.condition) || strcmp(condition_type, "i1") == 0)
				{
					fprintf(llvm, "br i1 %%%s, label %%%s, label %%%s__false__%d\n",
						ir[i].jump.condition,
						ir[i].jump.label,
						ir[i].jump.label,
						jumpcast_counter);
					fprintf(llvm, "%s__false__%d:\n", ir[i].jump.label, jumpcast_counter);
					jumpcast_counter++;
					break;
				}

				fprintf(llvm, "%%__jumpcast__%d = trunc %s %%%s to i1\n",
					jumpcast_counter,
					condition_type,
					ir[i].jump.condition);
				fprintf(llvm, "br i1 %%__jumpcast__%d, label %%%s, label %%%s__false__%d\n",
					jumpcast_counter,
					ir[i].jump.label,
					ir[i].jump.label,
					jumpcast_counter);
				fprintf(llvm, "%s__false__%d:\n", ir[i].jump.label, jumpcast_counter);
				jumpcast_counter++;
				break;
			case TYPE_RET:
				const int ret_order = get_torder(ir[i].ret.type);
				const int current_funcorder = get_torder(current_functype);
				bool ret_lo = get_tmplokey(ir[i].ret.value);

				if (ret_order == current_funcorder)
				{
					fprintf(llvm, "ret %s %%%s\n",
						current_functype,
						ir[i].ret.value);

					break;
				}

				fprintf(llvm, "%%___returncast___%d =", returncast_counter);
				if (ret_lo && current_funcorder)
				{
					fprintf(llvm, " zext i1 %%%s to %s\n",
						ir[i].ret.value,
						current_functype);
				}
				else
				{
					if (current_funcorder > ret_order)
						fprintf(llvm, " zext");
					else
						fprintf(llvm, " trunc");

					fprintf(llvm, " %s %%%s to %s\n",
						ir[i].ret.type,
						ir[i].ret.value,
						current_functype);
				}

				fprintf(llvm, "ret %s %%___returncast___%d\n",
					current_functype,
					returncast_counter);
				returncast_counter++;
				break;
			default:
		}
	}

	if (scope == 1)
		fprintf(llvm, "}\n\n");
}

void codegen_main(char* out)
{
	if (out == NULL)
		llvm = fopen("a.ll", "wr");
	else
	{
		if (strlen(out) > 64)
			printf("Output name length must be max 64\n");

		char* llvmfile_name = malloc(64);
		sprintf(llvmfile_name, "%s.ll", out);
		llvm = fopen(llvmfile_name, "wr");
	}
	parse_ir();

	fprintf(llvm,
	"define void @_start() {\n"
	"entry:\n"
		"\t%%r = call i32 @main()\n"
		"\t%%r64 = sext i32 %%r to i64\n"
		"\tcall void asm sideeffect \"syscall\", \"{rax},{rdi}\"(i64 60, i64 %%r64)\n"
		"\tunreachable\n"
	"}\n"
	);

	fclose(llvm);
	generate_bin(out);
}
