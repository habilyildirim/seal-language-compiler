#ifndef LEXER_H
#define LEXER_H

#include "common.h"

typedef enum
{
	/* SYMBOLS */

	SYMBOL_SEMICOLON,    // ;
	SYMBOL_COMMA,        // ,
	SYMBOL_DOT,          // .

	SYMBOL_LPAREN,       // (
	SYMBOL_RPAREN,       // )
	SYMBOL_LBRACE,       // {
	SYMBOL_RBRACE,       // }
	SYMBOL_LBRACKET,     // [
	SYMBOL_RBRACKET,     // ]
	SYMBOL_SINGLE_QUOTE, // '
	SYMBOL_DOUBLE_QUOTE, // "

	SYMBOL_PLUS,         // +
	SYMBOL_MINUS,        // -
	SYMBOL_MULTIPLY,     // *
	SYMBOL_DIVIDE,       // /
	SYMBOL_MODULO,       // %
	SYMBOL_ASSIGN,       // =

	SYMBOL_BACKSLASH,    /* \ */
	SYMBOL_HASH,         // #

	SYMBOL_INVALID,

	/* LOGICAL OPERATOR */

	LOPERATOR_AND, // &&
	LOPERATOR_OR,  // ||
	LOPERATOR_NOT,     // !

	/* RELATIONAL OPERATOR */

	ROPERATOR_EQUAL,   // ==
	ROPERATOR_NEQUAL,  // !=
	ROPERATOR_LESS,    // <
	ROPERATOR_GREATER, // >
	ROPERATOR_LOE,     // <=
	ROPERATOR_GOE,     // >=

	/* DATA TYPE */

	DTYPE_BINARY,    // binary
	DTYPE_INTEGER,   // integer
	DTYPE_INTEGER8,  // integer_8
	DTYPE_INTEGER16, // integer_16
	DTYPE_INTEGER32, // integer_32
	DTYPE_INTEGER64, // integer_64
	DTYPE_FLOAT,     // float
	DTYPE_DOUBLE,    // double
	DTYPE_CHAR,      // char

	/* KEYWORDS */

	KEYWORD_INCLUDE,  // INCLUDE
	KEYWORD_MACRO,    // DEF
	KEYWORD_FUNCTION, // ff
	KEYWORD_RETURN,   // return
	KEYWORD_JUMPER,   // jump
	KEYWORD_UNSIGNED, // unsigned

	INTEGER_LITERAL,
	STRING_LITERAL,
	CHAR_LITERAL,

	IDENTIFIER,
	NON,
}
_token_type;

typedef enum
{
	SYMBOL,
	LOPERATOR, // LOGICAL OPERATOR
	ROPERATOR, // RELATIONAL OPERATOR
	DTYPE,
	LITERAL,
	KEYWORD,
	_IDENTIFIER,
}
_token_group;

typedef enum
{
	PASS,
	READ,
	READ_STRING_LITERAL,
	READ_CHAR_LITERAL,
	READ_INTEGER_LITERAL,
}
_buffer_mod;

/* LEXEME TYPE */

typedef struct
{
	const char lexeme;
	_token_type token_type;
}
_symbol;

typedef struct
{
	const char *lexeme;
	_token_type token_type;
}
_loperator;

typedef struct
{
	const char *lexeme;
	_token_type token_type;
}
_roperator;

typedef struct
{
	const char *lexeme;
	_token_type token_type;
}
_data_type;

typedef struct
{
	const char *lexeme;
	_token_type token_type;
}
_keyword;

/* TOKEN STRUCT */

typedef struct
{
	_token_type token_type;
	_token_group token_group;

			  // LEXEME_BUFFER_LEN
	char value[256];

	unsigned int line;
	unsigned int column;
}
_token;

void print_tokens();

extern _token* tokens;
extern unsigned int tokens_counter;

void run_lexer_machine(const _ar sources_list);
void print_tokens(unsigned int ex);

#endif

