/*

	Seal Compiler - Lexer layer

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

#include "lexer.h"
#include "diagnostic.h"

/* ======================================== TABLES ======================================== */

const _symbol symbol_table[] =
{
	{';',  SYMBOL_SEMICOLON},
	{',',  SYMBOL_COMMA},
	{'.',  SYMBOL_DOT},

	{'(',  SYMBOL_LPAREN},
	{')',  SYMBOL_RPAREN},
	{'{',  SYMBOL_LBRACE},
	{'}',  SYMBOL_RBRACE},
	{'[',  SYMBOL_LBRACKET},
	{']',  SYMBOL_RBRACKET},
	{'\'', SYMBOL_SINGLE_QUOTE},
	{'"',  SYMBOL_DOUBLE_QUOTE},

	{'+',  SYMBOL_PLUS},
	{'-',  SYMBOL_MINUS},
	{'*',  SYMBOL_MULTIPLY},
	{'/',  SYMBOL_DIVIDE},
	{'%',  SYMBOL_MODULO},
	{'=',  SYMBOL_ASSIGN},

	{'\\', SYMBOL_BACKSLASH},
	{'#',  SYMBOL_HASH}
};

#define SYMBOL_TABLE_LENGTH 19

const _loperator logical_operator_table[] =
{
	{"&&", LOPERATOR_AND},
	{"||", LOPERATOR_OR},
	{"!",  LOPERATOR_NOT},
};

#define LOGICAL_OPERATOR_TABLE_LENGTH 3

const _roperator relational_operator_table[] =
{
	{"==", ROPERATOR_EQUAL},
	{"!=", ROPERATOR_NEQUAL},
	{"<=", ROPERATOR_LOE},
	{">=", ROPERATOR_GOE},
	{"<",  ROPERATOR_LESS},
	{">",  ROPERATOR_GREATER},
};

#define RELATIONAL_OPERATOR_TABLE_LENGTH 6

const _data_type data_type_table[] =
{
	{"binary",    DTYPE_BINARY},
	{"integer",   DTYPE_INTEGER},
	{"integer8",  DTYPE_INTEGER8},
	{"integer16", DTYPE_INTEGER16},
	{"integer32", DTYPE_INTEGER32},
	{"integer64", DTYPE_INTEGER64},
	{"float",     DTYPE_FLOAT},
	{"double",    DTYPE_DOUBLE},
	{"char",      DTYPE_CHAR},
};

#define DATA_TYPE_TABLE_LENGTH 9

const _keyword keyword_table[] =
{
	{"INCLUDE",  KEYWORD_INCLUDE},
	{"DEFINE",   KEYWORD_MACRO},
	{"ff",       KEYWORD_FUNCTION},
	{"return",   KEYWORD_RETURN},
	{"jump",     KEYWORD_JUMPER},
	{"unsigned", KEYWORD_UNSIGNED},
};

#define KEYWORD_TABLE_LENGTH 6

/* ======================================== TOOLS ======================================== */

_token_type query_symbol(const char lexeme)
{
	for (unsigned int i = 0; i < SYMBOL_TABLE_LENGTH; i++)
	{
		if (lexeme == symbol_table[i].lexeme)
			return symbol_table[i].token_type;
	}

	return SYMBOL_INVALID;
}

_token_type query_loperator_token(const char *lexeme, unsigned int *loop_i)
{
	if (strcmp(lexeme, "!=") == 0)
		return NON;

	char single_operant[2] = {(char)lexeme[0], '\0'};

	for (unsigned int i = 0; i < LOGICAL_OPERATOR_TABLE_LENGTH; i++)
	{
		if (strcmp(lexeme, logical_operator_table[i].lexeme) == 0)
		{
			(*loop_i)++;
			return logical_operator_table[i].token_type;
		}

		if (strcmp(single_operant, logical_operator_table[i].lexeme) == 0)
			return logical_operator_table[i].token_type;
	}

	return NON;
}

_token_type query_roperator_token(const char *lexeme, unsigned int *loop_i)
{
	char single_operant[2] = {(char)lexeme[0], '\0'};

	for (unsigned int i = 0; i < RELATIONAL_OPERATOR_TABLE_LENGTH; i++)
	{
		if (strcmp(lexeme, relational_operator_table[i].lexeme) == 0)
		{
			(*loop_i)++;
			return relational_operator_table[i].token_type;
		}

		if (strcmp(single_operant, relational_operator_table[i].lexeme) == 0)
			return relational_operator_table[i].token_type;
	}

	return NON;
}

_token_type query_data_type(const char *lexeme)
{
	for (unsigned int i = 0; i < DATA_TYPE_TABLE_LENGTH; i++)
	{
		if (strcmp(lexeme, data_type_table[i].lexeme) == 0)
			return data_type_table[i].token_type;
	}

	return NON;
}

_token_type query_keyword(const char *lexeme)
{
	for (unsigned int i = 0; i < KEYWORD_TABLE_LENGTH; i++)
	{
		if (strcmp(lexeme, keyword_table[i].lexeme) == 0)
			return keyword_table[i].token_type;
	}

	return IDENTIFIER;
}

void emit_token(const _token_type tt, _token_group tg, const char* value, const unsigned int tokens_counter, const unsigned int line_counter, unsigned int column_counter)
{
	void *tmp_tokensr = realloc(tokens, sizeof(_token) * (tokens_counter + 1));

	if (tmp_tokensr == NULL)
	{
		fprintf(stderr, "REALLOC IS NULL");
		exit(0);
	}

	tokens = tmp_tokensr;

	if (tg == KEYWORD || tg == DTYPE)
		column_counter--;

	if ((tg == ROPERATOR || tg == LOPERATOR) && strlen(value) == 2)
		column_counter++;

	if (tt == IDENTIFIER)
		tg = _IDENTIFIER;

	tokens[tokens_counter].token_type = tt;
	tokens[tokens_counter].token_group = tg;
	strcpy(tokens[tokens_counter].value, value);
	tokens[tokens_counter].line = line_counter;
	tokens[tokens_counter].column = column_counter;
}

/* ======================================== LEXER MACHINE ======================================== */

_ar sources_files;

char* buffer = NULL;
unsigned int buffersize = 0;

unsigned int lexeme_buffer_size = 32;
unsigned int lexeme_buffer_counter = 0;
char* lexeme_buffer = {0};

_token* tokens = NULL;
unsigned int tokens_counter = 0;

unsigned int line_counter = 1;
unsigned int column_counter = 1;

_buffer_mod buffer_mod = READ;

void lexer_scan_word(_token_type *dt, const unsigned int i)
{
	if (!_isalnum(buffer[i]) && (*dt = query_data_type(lexeme_buffer)) != NON)
	{
		emit_token(*dt, DTYPE, lexeme_buffer, tokens_counter, line_counter, column_counter);
		clear_buffer(lexeme_buffer, &lexeme_buffer_counter);

		tokens_counter++;
	}

	if ((!_isalnum(buffer[i]) && lexeme_buffer[0] != '\0') && *dt == NON)
	{
		emit_token(query_keyword(lexeme_buffer), KEYWORD, lexeme_buffer, tokens_counter, line_counter, column_counter);
		clear_buffer(lexeme_buffer, &lexeme_buffer_counter);

		tokens_counter++;
	}
}

void lexer_scan_operator(_token_type *lo, _token_type *ro, unsigned int *i)
{
	char operator_tmp[3] = {buffer[*i], buffer[*i + 1], '\0'};

	if (isspace(buffer[*i + 1]))
		operator_tmp[1] = '\0';

	if ((!_isalnum(buffer[*i]) && !isspace(buffer[*i]))
	&& (*lo = query_loperator_token(operator_tmp, i)) != NON)
	{
		if (*lo == LOPERATOR_NOT) // if single char
			operator_tmp[1] = '\0';

		emit_token(*lo, LOPERATOR, operator_tmp, tokens_counter, line_counter, column_counter);

		if (*lo != ROPERATOR_LESS && *lo != ROPERATOR_GREATER)
			column_counter++;

		tokens_counter++;
	}

	if ((!_isalnum(buffer[*i]) && !isspace(buffer[*i]))
	&& (*ro = query_roperator_token(operator_tmp, i)) != NON && *lo == NON)
	{
		if (*ro == ROPERATOR_LESS || *ro == ROPERATOR_GREATER) // if single char
			operator_tmp[1] = '\0';

		emit_token(*ro, ROPERATOR, operator_tmp, tokens_counter, line_counter, column_counter);

		if (*ro != ROPERATOR_LESS && *ro != ROPERATOR_GREATER)
			column_counter++;

		tokens_counter++;
	}
}

void lexer_scan_symbol(_token_type *ro, _token_type *lo, const unsigned int i)
{
	if ((!_isalnum(buffer[i]) && !isspace(buffer[i])) && (*ro == NON && *lo == NON))
	{
		char symbol[2] = {buffer[i], '\0'};

		if (query_symbol(buffer[i]) == SYMBOL_INVALID)
			lexer_error(sources_files[0], line_counter, column_counter, INVALID_CHAR);

		emit_token(query_symbol(buffer[i]), SYMBOL, symbol, tokens_counter, line_counter, column_counter);
		tokens_counter++;
	}
}

void try_read_buffer(const char *buffer, const unsigned int i, _buffer_mod BUFFER_MOD)
{
	if (BUFFER_MOD == READ_STRING_LITERAL)
	{
		lexeme_buffer[lexeme_buffer_counter] = *buffer;
		lexeme_buffer[lexeme_buffer_counter + 1] = '\0';
		lexeme_buffer_counter++;

		return;
	}

	if (_isalnum(buffer[i]) || BUFFER_MOD == READ_INTEGER_LITERAL)
	{
		lexeme_buffer[lexeme_buffer_counter] = buffer[i];
		lexeme_buffer[lexeme_buffer_counter + 1] = '\0';
		lexeme_buffer_counter++;

		return;
	}
}

void update_position(const unsigned int i)
{
	column_counter++;

	if (buffer[i] == '\n')
	{
		line_counter++;
		column_counter = 1;
	}
}

void try_read_string_literal(unsigned int *loop_i)
{
	char delimiter = '\"';
	_token_type emit_type = STRING_LITERAL;

	if (buffer_mod == READ_CHAR_LITERAL)
	{
		delimiter = '\'';
		emit_type = CHAR_LITERAL;
	}

	for (;buffer_mod == READ_STRING_LITERAL || buffer_mod == READ_CHAR_LITERAL; (*loop_i)++)
	{
		update_position(*loop_i);

		if (buffer[*loop_i] == delimiter)
		{
			emit_token(emit_type, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
			clear_buffer(lexeme_buffer, &lexeme_buffer_counter);

			tokens_counter++;
			buffer_mod = READ;
		}

		if (buffer[*loop_i] == '\\')
		{
			char escape_tmp[2];

			switch (buffer[*loop_i + 1])
			{
				case '\\':
					escape_tmp[0] = '\\';
					break;
				case '\'':
					escape_tmp[0] = '\'';
					break;
				case '\"':
					escape_tmp[0] = '\"';
					break;
				case 'n':
					escape_tmp[0] = '\n';
					break;
				case 'r':
					escape_tmp[0] = '\r';
					break;
				case 't':
					escape_tmp[0] = '\t';
					break;
				case 'v':
					escape_tmp[0] = '\v';
					break;
				case 'b':
					escape_tmp[0] = '\b';
					break;
				case 'f':
					escape_tmp[0] = '\f';
					break;
				case 'a':
					escape_tmp[0] = '\a';
					break;
				case '0':
				default:
					lexer_error(sources_files[0], line_counter, column_counter, INVALID_ESCAPE);
			}

			escape_tmp[1] = '\0';
			try_read_buffer(escape_tmp, *loop_i, buffer_mod);

			(*loop_i) += 2;
			continue;
		}

		try_read_buffer(buffer, *loop_i, READ);
	}
}

void try_read_integer_literal(unsigned int *loop_i)
{
	if (buffer_mod != READ_INTEGER_LITERAL)
		return;

	char prefix[3] = {buffer[*loop_i], buffer[*loop_i + 1], '\0'};

	/* ==== HEXADECIMAL ==== */

	if (strcmp(prefix, "0x") == 0)
	{
		for (;isxdigit(buffer[*loop_i]) || (buffer[*loop_i] == 'x' && lexeme_buffer[1] == '\0'); (*loop_i)++)
		{
			update_position(*loop_i);
			try_read_buffer(buffer, *loop_i, READ_INTEGER_LITERAL);
		}

		if (isalpha(buffer[*loop_i]) && !isxdigit(buffer[*loop_i]))
			lexer_error(sources_files[0], line_counter, column_counter, IS_NOT_HEX);

		emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
		return;
	}

	/* ==== BINARY ==== */

	if (strcmp(prefix, "0b") == 0)
	{
		for (;_isbinary(buffer[*loop_i]) || (buffer[*loop_i] == 'b' && lexeme_buffer[1] == '\0'); (*loop_i)++)
		{
			update_position(*loop_i);
			try_read_buffer(buffer, *loop_i, READ_INTEGER_LITERAL);
		}

		if (isalpha(buffer[*loop_i]) || (!_isbinary(buffer[*loop_i]) && isdigit(buffer[*loop_i])))
			lexer_error(sources_files[0], line_counter, column_counter, IS_NOT_BIN);

		emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
		return;
	}

	/* ==== DECIMAL ==== */

	for (;isdigit(buffer[*loop_i]); (*loop_i)++)
	{
		update_position(*loop_i);
		try_read_buffer(buffer, *loop_i, READ_INTEGER_LITERAL);
	}

	if (isalpha(buffer[*loop_i]))
		lexer_error(sources_files[0], line_counter, column_counter, IS_NOT_DECIMAL);

	emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
	return;
}

void run_lexer_machine(const _ar sources_list)
{
	buffer = open_buffer(sources_list[0], &buffersize);

	lexeme_buffer = malloc(lexeme_buffer_size);
	lexeme_buffer[0] = '\0';

	sources_files = sources_list;

	for (unsigned int i = 0; i < buffersize; i++)
	{
		if (lexeme_buffer_counter + 1 >= lexeme_buffer_size)
		{
			lexeme_buffer_size *= 2;
			lexeme_buffer = realloc(lexeme_buffer, lexeme_buffer_size);
		}

		if (buffer[i] == '~')
			buffer_mod = PASS;

		if (buffer_mod == PASS && buffer[i] != '\n')
		{
			update_position(i);
			continue;
		}

		if (buffer_mod == READ && buffer[i] == '\"')
		{
			buffer_mod = READ_STRING_LITERAL;
			continue;
		}

		if (buffer_mod == READ && buffer[i] == '\'')
		{
			buffer_mod = READ_CHAR_LITERAL;
			continue;
		}

		if (buffer_mod == READ && (lexeme_buffer[0] == '\0' && isdigit(buffer[i])))
			buffer_mod = READ_INTEGER_LITERAL;

		try_read_string_literal(&i);
		try_read_integer_literal(&i);

		buffer_mod = READ;

		try_read_buffer(buffer, i, buffer_mod);

		_token_type dt = NON;
		_token_type lo = NON;
		_token_type ro = NON;

		lexer_scan_word(&dt, i);
		lexer_scan_operator(&lo, &ro, &i);
		lexer_scan_symbol(&lo, &ro, i);

		update_position(i);
	}
}
