#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

typedef enum
{
	INVALID_CHAR,
	INVALID_ESCAPE,
	IS_NOT_HEX,
	IS_NOT_BIN,
	IS_NOT_DECIMAL,
	NON_LEXER,
}
LEXER_LAYER_ERROR_TYPE;

typedef enum
{
	//........
	NON_PARSER,
}
PARSER_LAYER_ERROR_TYPE;

typedef enum
{
	//...........
	NON_SEMANTIC,
}
SEMANTIC_LAYER_ERROR_TYPE;

typedef enum
{
	//.....
	NON_IR,
}
IR_LAYER_ERROR_TYPE;

void lexer_error(const char* source_file, const unsigned int line, const unsigned int column, const LEXER_LAYER_ERROR_TYPE ERROR_TYPE);
void parser_error(const char* source_file, const unsigned int line, const unsigned int column, const PARSER_LAYER_ERROR_TYPE ERROR_TYPE);
void semantic_error(const char* source_file, const unsigned int line, const unsigned int column, const SEMANTIC_LAYER_ERROR_TYPE ERROR_TYPE);
void ir_error(const char* source_file, const unsigned int line, const unsigned int column, const IR_LAYER_ERROR_TYPE ERROR_TYPE);

#endif
