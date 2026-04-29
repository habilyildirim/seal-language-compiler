#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H
#include <stdlib.h>

typedef enum
{
	FILE_NOT_EXISTS,
	END_SYMBOL_WRONG,
	END_SYMBOL,
	NON_PREP	
}
PREP_LAYER_ERROR_TYPE;

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
	UNEXPECTED_CALL,
	UNEXPECTED_UVAR,
	UNEXPECTED_VAR,
	UNEXPECTED_JUMPER,
	UNEXPECTED_LABEL,
	UNEXPECTED,
	WRONG_EXPRESSION,
	WRONG_CHRREQ,
	NESTED_FUNCTIONS,
	NON_PARSER
}
PARSER_LAYER_ERROR_TYPE;

typedef enum
{
	FILE_NOT_OPEN,
	REDEFINITION,
	UNDEFINED,
	GLOBAL_CANNOTRET,
	CANNOT_RETNULLVAL,
	TYPE_ERROR,
	ARGC_MISSMATCH,
	WITHOUT_FUNCTION,
	NON_SEMANTIC,
	MAIN_FUNC_NOT_EXISTS
}
SEMANTIC_LAYER_ERROR_TYPE;

typedef enum
{
	//.....
	NON_IR,
}
IR_LAYER_ERROR_TYPE;

void prep_error(const char* source_file, const uint line,
				const uint column, const PREP_LAYER_ERROR_TYPE ERROR_TYPE);

void semantic_error(const char* source_file, const uint line, const uint column, const char* scope, const uint scpline,
					const uint scpcolumn, const char* argument, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE);

void lexer_error(const uint line, const uint column, const LEXER_LAYER_ERROR_TYPE ERROR_TYPE);
void parser_error(const uint line, const uint column, const PARSER_LAYER_ERROR_TYPE ERROR_TYPE);
void ir_error(const uint line, const uint column, const IR_LAYER_ERROR_TYPE ERROR_TYPE);

#endif
