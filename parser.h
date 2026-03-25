#ifndef PARSER_H
#define PARSER_H
#include <stdlib.h>

typedef enum 
{
	NODE_INT_LITERAL,
	NODE_BINARY,
	NODE_IDENTIFIER,
	NODE_UNARY,
	NODE_NOT,
	NODE_CALL,
}
_node_type;

typedef enum
{
	INCLUDE,
	MACRO,
	FUNCTION,
	CALL,
	RETURN,
	JUMPER,
	LABEL,
	VAR,
	UVAR,
	PARSE_ASSIGNMENT,
}
_AST_TYPE;

typedef struct EXPR
{
    _node_type type;

    union
    {
        char* literal;
        char* identifier;

        struct
        {
            struct EXPR* left;
            struct EXPR* right;
            char* op;
        }
        binary;

        struct
        {
        	char* op;
        	struct EXPR* value;
        }
        unary;

        struct
        {
            char* callee;
            struct EXPR** args;
            uint argc;
        }
        call;
    };
}
EXPR;

typedef struct
{
	_AST_TYPE type;
	uint seq;

	uint line;
	uint column;

	uint scpline;
	uint scpcolumn;
	char* scope;

	union
	{
		/*
			scope_ref vars written for already
			exists control from semantic
		*/

		struct
		{
			char* lib;
		}
		include;

		struct
		{
			char* name;
			char* value;
		}
		macro;
		
		struct
		{
			char* type;
			char* name;
			struct var* args;
			uint argc;
		}
		function;

		struct
		{
            char* callee;
            struct EXPR** args;
            uint argc;
		}
		call;

		struct
		{
			struct EXPR* value;
		}
		_return;

		struct
		{
			char* label;
			struct EXPR* condition;
		}
		jumper;

		struct
		{
			char* name;
		}
		label;

		struct var
		{
			char* type;
			char* name;
			struct EXPR* value;
		}
		var;

		struct
		{
			char* name;
			struct EXPR* value;
		}
		assignment;
	};
}
AST;

extern AST* ast;
extern uint ast_counter;

void parser_main();

#endif
