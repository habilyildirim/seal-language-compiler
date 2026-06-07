#ifndef IR_H
#define IR_H
#include "common.h"

typedef enum
{
	TYPE_TMP,
	TYPE_FUNC,
	TYPE_ALLOCATE,
	TYPE_STORE,
	TYPE_POINTER,
	TYPE_JUMP,
	TYPE_LABEL,
	TYPE_RET,
	TYPE_NULLCALL,
}
IR_TYPE;

typedef enum
{
	OP_CONST,
	OP_LOAD,
	OP_CALL,
	OP_NOT,
	OP_NEG,
	OP_MUL,
	OP_MOD,
	OP_DIV,
	OP_SUB,
	OP_ADD,
	OP_CMP_EQ,
	OP_CMP_NE,
	OP_CMP_GT,
	OP_CMP_LT,
	OP_CMP_GE,
	OP_CMP_LE,
	OP_AND,
	OP_OR
}
OP_TYPE;

typedef struct
{
	char* type;
	char* name;
	char* value;
}
arg;

typedef struct
{
	IR_TYPE type;
	char* scope;

	union
	{
		struct
		{
			char* type;
			char* name;
			OP_TYPE op;
			char* oper;
			char* left;
			char* right;
			bool lo_key;
			char* size;

			// for call node
			char* callee;
			uint argc;
			arg* args;
		}
		tmp;

		struct
		{
			char* name;
			char* type;
			uint argc;
			arg* args;
		}
		func;

		struct
		{
			char* var_name;
			char* type;
			char* size;
		}
		allocate;

		struct
		{
			char* var_name;
			char* type;
			char* value;
			char* size;
			bool arg_key;
		}
		store;

		struct
		{
			char* condition;
			char* label;
		}
		jump;

		struct
		{
			char* label_name;
		}
		label;

		struct
		{
			char* callee;
			uint argc;
			arg* args;
		}
		null_call;

		struct
		{
			char* type;
			char* value;
		}
		ret;
	};
}
IR;

extern IR* ir;
extern uint ir_counter;
void ir_main();

#endif
