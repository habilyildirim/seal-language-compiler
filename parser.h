#ifndef PARSER_H
#define PARSER_H

typedef enum 
{
	NODE_INT_LITERAL,
	NODE_FUNCTION_CALL,
	NODE_MACRO,
	NODE_BINARY,
	NODE_IDENTIFIER,
	NODE_UNARY,
	NODE_CALL,
	NULL_NT,
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
            unsigned int argc;
        } 
        call;
    };
} 
EXPR;

typedef struct
{
	_AST_TYPE type;
	unsigned int seq;
	char* scope;

	union
	{
		struct
		{
			char* lib;
		}
		include;

		struct
		{
			char* macro_name;
			char* macro_value;
		}
		macro;
		
		struct
		{
			char* function_type;
			char* function_name;
			struct var* args;
			unsigned int argc;
		}
		function;

		struct
		{
            char* callee;
            struct EXPR** args;
            unsigned int argc;
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
extern unsigned int ast_counter;

void run_parser();

#endif
