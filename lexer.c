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
	
	{'(',  SYMBOL_LPAREN},
	{')',  SYMBOL_RPAREN},
	{'{',  SYMBOL_LBRACE},
	{'}',  SYMBOL_RBRACE},
	{'[',  SYMBOL_LBRACKET},
	{']',  SYMBOL_RBRACKET},
	{'\'', SYMBOL_SINGLE_QUOTE},
	{'"',  SYMBOL_DOUBLE_QUOTE},
	{'#',  KEYWORD_FUNCTION},
	
	{'+',  SYMBOL_PLUS},
	{'-',  SYMBOL_MINUS},
	{'*',  SYMBOL_MULTIPLY},
	{'/',  SYMBOL_DIVIDE},
	{'%',  SYMBOL_MODULO},
	{'=',  SYMBOL_ASSIGN},

	{'\\', SYMBOL_BACKSLASH},
	{':',  KEYWORD_LABEL},
	{'$',  SYMBOL_DOLAR},
};
	
#define SYMBOL_TABLE_LENGTH 20

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
	{"return",   KEYWORD_RETURN},
	{"jump",     KEYWORD_JUMPER},
	{"unsigned", KEYWORD_UNSIGNED},
};

#define KEYWORD_TABLE_LENGTH 5

/* ======================================== TOOLS ======================================== */

_token_type query_symbol(const char lexeme)
{
	for (uint i = 0; i < SYMBOL_TABLE_LENGTH; i++)
	{
		if (lexeme == symbol_table[i].lexeme)
			return symbol_table[i].token_type;
	}

	return SYMBOL_INVALID;
}

_token_type query_loperator_token(const char *lexeme, uint *i)
{
	// This control is for the != and ! not to overlap
	if (strcmp(lexeme, "!=") == 0)
		return NON;

	char single_operant[2] = {(char)lexeme[0], '\0'};

	for (uint c = 0; c < LOGICAL_OPERATOR_TABLE_LENGTH; c++)
	{
		if (strcmp(lexeme, logical_operator_table[c].lexeme) == 0)
		{
			(*i)++;
			return logical_operator_table[c].token_type;
		}

		if (strcmp(single_operant, logical_operator_table[c].lexeme) == 0)
			return logical_operator_table[c].token_type;
	}

	return NON;
}

_token_type query_roperator_token(const char *lexeme, uint *i)
{
	char single_operant[2] = {(char)lexeme[0], '\0'};

	for (uint c = 0; c < RELATIONAL_OPERATOR_TABLE_LENGTH; c++)
	{
		if (strcmp(lexeme, relational_operator_table[c].lexeme) == 0)
		{
			(*i)++;
			return relational_operator_table[c].token_type;
		}

		if (strcmp(single_operant, relational_operator_table[c].lexeme) == 0)
			return relational_operator_table[c].token_type;
	}

	return NON;
}

_token_type query_data_type(const char *lexeme)
{
	for (uint c = 0; c < DATA_TYPE_TABLE_LENGTH; c++)
	{
		if (strcmp(lexeme, data_type_table[c].lexeme) == 0)
			return data_type_table[c].token_type;
	}

	return NON;
}

_token_type query_keyword(const char *lexeme)
{
	for (uint c = 0; c < KEYWORD_TABLE_LENGTH; c++)
	{
		if (strcmp(lexeme, keyword_table[c].lexeme) == 0)
			return keyword_table[c].token_type;
	}

	if(is_integer(lexeme))
		return INTEGER_LITERAL;

	return IDENTIFIER;
}

void emit_token(const _token_type tt, _token_group tg, const char* value, const uint tokens_counter, const uint line_counter, uint column_counter)
{
	if (strlen(value) > 255)
		lexer_error(source_files[0], line_counter, column_counter, IDENTIFIER_OVERFLOW);

	void *tmp_tokensr = realloc(tokens, sizeof(_token) * (tokens_counter + 1));

	if (tmp_tokensr == NULL)
	{
		fprintf(stderr, "Lexer realloc error");
		exit(1);
	}

	tokens = tmp_tokensr;

	if (tg == KEYWORD || tg == DTYPE)
		column_counter--;

	if ((tg == ROPERATOR || tg == LOPERATOR) && strlen(value) == 2)
		column_counter++;

	if (tt == IDENTIFIER)
		tg = _IDENTIFIER;

	if (isbinop(value))
		tg = BINARY_OP;
	
	tokens[tokens_counter].token_type = tt;
	tokens[tokens_counter].token_group = tg;
	strcpy(tokens[tokens_counter].value, value);
	tokens[tokens_counter].line = line_counter;
	tokens[tokens_counter].column = column_counter;
}

typedef enum
{
	PASS,
	BLOCK_PASS,
	READ,
	READ_STRING_LITERAL,
	READ_ESCAPE,
	READ_CHAR_LITERAL,
	READ_INTEGER_LITERAL,
}
_buffer_mod;

/* ======================================== LEXER MACHINE ======================================== */

_ar source_files;

char* buffer = NULL;
uint buffersize = 0;

uint lexeme_buffer_size = 32;
uint lexeme_buffer_counter = 0;
char* lexeme_buffer = {0};

_token* tokens = NULL;
uint tokens_counter = 0;

uint line_counter = 1;
uint column_counter = 1;

_buffer_mod buffer_mod = READ;

void scan_word(_token_type *dt, const uint i)
{
	/*
		_isalnum -> If parameter is equal to '_'
		or alnum(chr) is true then return true
	*/

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

void scan_operator(_token_type *lo, _token_type *ro, uint *i)
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

		/*
			If lo is single char then increase
			column position for diagnostic
		*/

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

void scan_symbol(_token_type *ro, _token_type *lo, const uint i)
{
	if ((!_isalnum(buffer[i]) && !isspace(buffer[i])) && (*ro == NON && *lo == NON))
	{
		char symbol[2] = {buffer[i], '\0'};

		if (query_symbol(buffer[i]) == SYMBOL_INVALID)
			lexer_error(source_files[0], line_counter, column_counter, INVALID_CHAR);

		emit_token(query_symbol(buffer[i]), SYMBOL, symbol, tokens_counter, line_counter, column_counter);
		tokens_counter++;
	}
}

void read_buffer(const char* buffer, const uint i, _buffer_mod BUFFER_MOD)
{
	if (BUFFER_MOD == READ_STRING_LITERAL || BUFFER_MOD == READ_ESCAPE)
	{
		if (BUFFER_MOD == READ_ESCAPE)
			lexeme_buffer[lexeme_buffer_counter] = *buffer;
		else
			lexeme_buffer[lexeme_buffer_counter] = buffer[i];
			
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

void update_position(const uint i)
{
	// Position for diagnostic

	column_counter++;

	if (buffer[i] == '\n')
	{
		line_counter++;
		column_counter = 1;
	}
}

void read_string_literal(uint *i, const uint is_charliteral)
{
	if (buffer_mod != READ_STRING_LITERAL)
		return;

	char delimiter = '\"';

	if (is_charliteral)
		delimiter = '\'';

	while (buffer_mod == READ_STRING_LITERAL)
	{
		update_position(*i);

		if(buffer[*i] == delimiter)
		{
			_token_type ltmp_tt = STRING_LITERAL;

			if (is_charliteral)
				ltmp_tt = CHAR_LITERAL;


			emit_token(ltmp_tt, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
			clear_buffer(lexeme_buffer, &lexeme_buffer_counter);

			tokens_counter++;
			buffer_mod = READ;
		}

		if(buffer[*i] == '\\')
		{
			char escape_tmp[2];

			switch(buffer[*i + 1])
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
					lexer_error(source_files[0], line_counter, column_counter, INVALID_ESCAPE);
			}

			escape_tmp[1] = '\0';
			read_buffer(escape_tmp, *i, READ_ESCAPE);

			/*
				Skip 2 char for simple 
				(max char 2) espace seq

				\\ \' \" \n \r \t \v \b \f \a
			*/

			(*i) += 2;
			continue;
		}

		read_buffer(buffer, *i, buffer_mod);
		(*i)++;
	}
}

void read_char_literal(uint *i)
{
	read_string_literal(&(*i), 1);
}

void read_integer_literal(uint *i)
{
	if (buffer_mod != READ_INTEGER_LITERAL)
		return;

	char prefix[3] = {buffer[*i], buffer[*i + 1], '\0'};

	/* ==== HEXADECIMAL ==== */

	if (strcmp(prefix, "0x") == 0)
	{
		/*
			(buffer[*i] == 'x' && lexeme_buffer[1] == '\0')
			(buffer[*i] == 'b' && lexeme_buffer[1] == '\0')

			Check buffer for prevent literal prefixes 
			from being entered integer literal.
		*/

		for (;isxdigit(buffer[*i]) || (buffer[*i] == 'x' && lexeme_buffer[1] == '\0'); (*i)++)
		{
			update_position(*i);
			read_buffer(buffer, *i, READ_INTEGER_LITERAL);
		}

		if (isalpha(buffer[*i]) && !isxdigit(buffer[*i]))
			lexer_error(source_files[0], line_counter, column_counter, IS_NOT_HEX);

		emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
		return;
	}

	/* ==== BINARY ==== */

	if (strcmp(prefix, "0b") == 0)
	{
		for (;_isbinary(buffer[*i]) || (buffer[*i] == 'b' && lexeme_buffer[1] == '\0'); (*i)++)
		{
			update_position(*i);
			read_buffer(buffer, *i, READ_INTEGER_LITERAL);
		}

		if (isalpha(buffer[*i]) || (!_isbinary(buffer[*i]) && isdigit(buffer[*i])))
			lexer_error(source_files[0], line_counter, column_counter, IS_NOT_BIN);

		emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
		return;
	}

	/* ==== DECIMAL ==== */

	for (uint dot_counter = 0; isdigit(buffer[*i]) || buffer[*i] == '.'; (*i)++)
	{
		if (buffer[*i] == '.')
			dot_counter++;
		else
			dot_counter = 0;

		if (dot_counter > 1)
			lexer_error(source_files[0], line_counter, column_counter, MULTIPLE_DOTS);

		update_position(*i);
		read_buffer(buffer, *i, READ_INTEGER_LITERAL);
	}

	if (isalpha(buffer[*i]) || strcmp(lexeme_buffer, ".") == 0)
		lexer_error(source_files[0], line_counter, column_counter, IS_NOT_DECIMAL);

	emit_token(INTEGER_LITERAL, LITERAL, lexeme_buffer, tokens_counter, line_counter, column_counter);
	return;
}

void lexer_main(const _ar sources_list)
{
	buffer = open_buffer(sources_list[0], &buffersize);

	lexeme_buffer = malloc(lexeme_buffer_size);
	lexeme_buffer[0] = '\0';

	source_files = sources_list;
	
	for (uint i = 0; i < buffersize; i++)
	{		
		if (lexeme_buffer_counter + 1 >= lexeme_buffer_size)
		{
			lexeme_buffer_size *= 2;
			lexeme_buffer = realloc(lexeme_buffer, lexeme_buffer_size);
		}

		/*
			/~
				Block comment
			~/
		*/

		if (buffer[i] == '/' && buffer[i + 1] == '~')
			buffer_mod = BLOCK_PASS;
		if (buffer[i] == '~' && buffer[i + 1] == '/')
			buffer_mod = READ;

		if (buffer_mod == BLOCK_PASS)
		{
			update_position(i);
			continue;
		}

		// ~ <- Comment line

		if (buffer[i] == '~')
			buffer_mod = PASS;

		if (buffer_mod == PASS && buffer[i] != '\n')
		{
			update_position(i);
			continue;
		}

		if (buffer_mod == READ && (lexeme_buffer[0] == '\0' && 
			(isdigit(buffer[i]) || buffer[i] == '.')))
			buffer_mod = READ_INTEGER_LITERAL;

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

		/*
			If buffer_mod is match then start
			literal by read_literal functions

			string_literal: If buffermod is not equal to
							string_literal then exit the function.

							buffermod is equal to string_literal then 
							start generate literal.
	
			... and others
		*/

		read_string_literal(&i, 0);
		read_integer_literal(&i);
		read_char_literal(&i);

		buffer_mod = READ;

		read_buffer(buffer, i, buffer_mod);

		_token_type dt = NON;
		_token_type lo = NON;
		_token_type ro = NON;

		/*
			Scan the word for detection 
			identifier and the keyword.
		*/
		
		scan_word(&dt, i);
		scan_operator(&lo, &ro, &i);
		scan_symbol(&lo, &ro, i);

		update_position(i);
	}
}
