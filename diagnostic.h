#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H
#include <stdlib.h>

typedef enum
{
	INVALID_CHAR,
	INVALID_ESCAPE,
	IS_NOT_HEX,
	IS_NOT_BIN,
	IS_NOT_DECIMAL,
	IDENTIFIER_OVERFLOW,
	MULTIPLE_DOTS,
	NON_LEXER,
}
LEXER_LAYER_ERROR_TYPE;

typedef enum
{
	MISSING_SEMICOLON,
	MISSING_DOLLAR,
	MISSING_ARG,
	UNEXPECTED_INCLUDE,
	UNEXPECTED_MACRO,
	UNEXPECTED_FUNCTION,
	NESTED_FUNCTIONS,
	UNEXPECTED_CALL,
	UNEXPECTED_UVAR,
	UNEXPECTED_VAR,
	UNEXPECTED_JUMPER,
	UNEXPECTED_LABEL,
	WRONG_EXPRESSION,
	UNEXPECTED,
	NON_PARSER,
}
PARSER_LAYER_ERROR_TYPE;

typedef enum
{
	REDEFINITION,
	UNDEFINED,
	GLOBAL_CANNOTRET,
	CANNOT_RETNULLVAL,
	TYPE_ERROR,
	NON_SEMANTIC,
	ARGC_MISSMATCH,
	WITHOUT_FUNCTION
}
SEMANTIC_LAYER_ERROR_TYPE;

typedef enum
{
	//.....
	NON_IR,
}
IR_LAYER_ERROR_TYPE;

void lexer_error(const char* source_file, const uint line, const uint column, 
					const LEXER_LAYER_ERROR_TYPE ERROR_TYPE);

void parser_error(const char* source_file, const uint line, const uint column, 
					const PARSER_LAYER_ERROR_TYPE ERROR_TYPE);

void semantic_error(const char* source_file, const uint line, const uint column, const char* scope, 
						const uint scpline, const uint scpcolumn, 
						const char* argument, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE);

void ir_error(const char* source_file, const uint line, const uint column, 
				const IR_LAYER_ERROR_TYPE ERROR_TYPE);

#endif
