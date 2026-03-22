/*

	Seal Compiler - Parser layer

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

#include "parser.h"
#include "lexer.h"
#include "common.h"
#include "diagnostic.h"

#define _operators _token_type
#define _dtype _token_type

void overflow_control(const unsigned int c, PARSER_LAYER_ERROR_TYPE err)
{
	if (c >= tokens_counter)
		parser_error(source_files[0], tokens[c - 1].line, tokens[c - 1].column, err);
}

int get_precedence(_token t)
{	
    if (t.token_group != BINARY_OP)
        return -1;

    if ((strcmp(t.value, "*") == 0 || strcmp(t.value, "/") == 0) || (strcmp(t.value, "%") == 0))
        return 3;

    if (strcmp(t.value, "+") == 0 || strcmp(t.value, "-") == 0)
    	return 2;

    if (strcmp(t.value, "==") == 0 ||
     	strcmp(t.value, "!=") == 0 ||
     	strcmp(t.value, ">=") == 0 ||
     	strcmp(t.value, "<=") == 0 ||
     	strcmp(t.value, "<") == 0  ||
    	strcmp(t.value, ">") == 0)
     	return 1;

    if (strcmp(t.value, "&&") == 0 || strcmp(t.value, "||") == 0)
    	return 0;

    return -1;
}

EXPR* parse_expression
(unsigned int *i, int precedence);

EXPR* parse_primary(unsigned int *i)
{
	overflow_control(*i, WRONG_EXPRESSION);

    _token tok = tokens[*i];

    EXPR* node = malloc(sizeof(EXPR));
    memset(node, 0, sizeof(EXPR));

	// Handle negative expression
    if (tok.token_type == SYMBOL_MINUS)
    {
        (*i)++;

        EXPR* operand = parse_primary(i);

        EXPR* node = malloc(sizeof(EXPR));
        memset(node, 0, sizeof(EXPR));

        node->type = NODE_UNARY;
        node->unary.op = strdup("-");
        node->unary.value = operand;

        return node;
    }

    if (tok.token_group == _IDENTIFIER) 
    {
        node->type = NODE_IDENTIFIER;
        node->identifier = strdup(tok.value);
        (*i)++;
        
    	if (*i < tokens_counter && tokens[*i].token_type == SYMBOL_LPAREN)
    	{
        	(*i)++;

        	EXPR** args = NULL;
        	int arg_count = 0;

        	while (*i < tokens_counter && tokens[*i].token_type != SYMBOL_RPAREN)
        	{
            	args = realloc(args, sizeof(EXPR*) * (arg_count + 1));
            	args[arg_count++] = parse_expression(i, 0);

           		if (tokens[*i].token_type == SYMBOL_COMMA)
                	(*i)++;
            	else
        			break;
        	}

        	if (tokens[*i].token_type != SYMBOL_RPAREN)
        		parser_error(source_files[0], tokens[*i - 1].line, tokens[*i - 1].column, WRONG_EXPRESSION);

        	(*i)++;

        	EXPR* node = malloc(sizeof(EXPR));
        	memset(node, 0, sizeof(EXPR));

        	node->type = NODE_CALL;
        	node->call.callee = strdup(tok.value);
        	node->call.args = args;
        	node->call.argc = arg_count;

        	return node;
    	}

        return node;
    }

    if (tok.token_type == INTEGER_LITERAL)
    {
        node->type = NODE_INT_LITERAL;
        node->literal = strdup(tok.value);
        (*i)++;
        return node;
    }

    if (tok.token_type == SYMBOL_LPAREN)
    {
        (*i)++;
        EXPR* inner = parse_expression(i, 0);

		overflow_control(*i, WRONG_EXPRESSION);
        	
        (*i)++;
        free(node);
        return inner;
    }

	parser_error(source_files[0], tok.line, tok.column, WRONG_EXPRESSION);
}

EXPR* parse_expression(unsigned int *i, int precedence)
{
    EXPR* left = parse_primary(&(*i));

    while (*i < tokens_counter && 
    		isbinop(tokens[*i].value) && 
    		get_precedence(tokens[*i]) >= precedence)
    {
        _token tok_op = tokens[*i];
        char* op = strdup(tok_op.value);
        unsigned int op_prec = get_precedence(tok_op);

        (*i)++;
        EXPR* right = parse_expression(i, op_prec + 1);

        EXPR* bin = malloc(sizeof(EXPR));
        memset(bin, 0, sizeof(EXPR));
        bin->type = NODE_BINARY;
        bin->binary.left = left;
        bin->binary.right = right;
        bin->binary.op = op;

        left = bin;
    }

    return left;
}

void free_expr(EXPR* e)
{
    if (!e)
    	return;

    switch (e->type)
    {
        case NODE_BINARY:
            free_expr(e->binary.left);
            free_expr(e->binary.right);
            free(e->binary.op);
            break;
        case NODE_IDENTIFIER:
            free(e->identifier);
            break;
        case NODE_INT_LITERAL:
            free(e->literal);
            break;
        case NODE_CALL:
            for (unsigned int i = 0; i < e->call.argc; i++)
                free_expr(e->call.args[i]);
        default:
            break;
    }

    free(e);
}

AST parse_include(unsigned int *i, unsigned int c)
{
	AST result;

	result.seq = c;
	result.type = INCLUDE;

	if (tokens[*i + 1].token_type != STRING_LITERAL)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_INCLUDE);

	result.include.lib = tokens[*i + 1].value;
	(*i)++;

	return result;
}

AST parse_macro(unsigned int *i, unsigned int c)
{
	AST result;
	
	result.type = MACRO;
	result.seq = c;

	(*i)++;

	if (tokens[*i].token_type != IDENTIFIER)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_MACRO);

	result.macro.name = tokens[*i].value;

	(*i)++;
	char* temp = malloc(strlen(tokens[*i].value) + 1);

	if (tokens[*i].token_type == SYMBOL_DOLAR)
	{
		result.macro.value = NULL;
		return result;
	}
			
	strcpy(temp, tokens[*i].value);
	(*i)++;

	for (;tokens[*i].token_type != SYMBOL_DOLAR; (*i)++)
	{
		overflow_control(*i, MISSING_DOLLAR);
		temp = realloc(temp, strlen(temp) + strlen(tokens[*i].value) + 1);
		strcat(temp, tokens[*i].value);
	}

	temp[strlen(temp)] = '\0';

	result.macro.value = temp;

	
	return result;
}

AST parse_var(unsigned int *i, unsigned int is_unsigned, unsigned int c)
{
	AST result;
	
	result.type = VAR;
	result.seq = c;
	
	if (is_unsigned)
	{
		result.type = UVAR;
		(*i)++;

		if (tokens[*i].token_group != DTYPE)
			parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_UVAR);

		overflow_control(*i, MISSING_ARG);
		char* us_type = malloc(strlen("unsigned ") + strlen(tokens[*i + 2].value) + 1);

		strcpy(us_type, "unsigned ");
		strcat(us_type, tokens[*i].value);
		result.var.type = us_type;
		(*i)++;

		if (tokens[*i].token_group != _IDENTIFIER)
			parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_UVAR);
	
		result.var.name = tokens[*i].value;
		(*i)++;

		if (tokens[*i].token_type == SYMBOL_ASSIGN)
		{
			(*i)++;
			result.var.value = parse_expression(&(*i), 0);

			overflow_control(*i, MISSING_SEMICOLON);

			if (tokens[*i].token_type != SYMBOL_SEMICOLON)
				parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);
			
			return result;
		}

		result.var.value = NULL;
		if (tokens[*i].token_type != SYMBOL_SEMICOLON)
			parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);
	
		return result;
	}
	
	result.var.type = tokens[*i].value;
	(*i)++;
	
	if (tokens[*i].token_type != IDENTIFIER)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_VAR);

	result.var.name = tokens[*i].value;
	(*i)++;
	
	if (tokens[*i].token_type == SYMBOL_ASSIGN)
	{
		(*i)++;
		
		result.var.value = parse_expression(&(*i), 0);
		overflow_control(*i, MISSING_SEMICOLON);

		if (tokens[*i].token_type != SYMBOL_SEMICOLON)
			parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

		return result;
	}

	overflow_control(*i, MISSING_SEMICOLON);

	result.var.value = NULL;
	if (tokens[*i].token_type != SYMBOL_SEMICOLON)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

	return result;
}

AST parse_assignment(unsigned int *i, unsigned int c)
{
	AST result;
	
	result.type = PARSE_ASSIGNMENT;
	result.seq = c;

	/*	
		   i=0     1      2
		identifier = expression;
		         ^      (i+=2)
		         |
			     assignment->name	
	*/

	result.assignment.name = tokens[*i].value;
	
	(*i)+=2;
	result.assignment.value = parse_expression(&(*i), 0);
	
	overflow_control(*i, MISSING_SEMICOLON);
	
	if (tokens[*i].token_type != SYMBOL_SEMICOLON)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

	return result;
}

AST parse_call(unsigned int *i, unsigned int c)
{
	AST result;

	result.type = CALL;
	result.seq = c;

	EXPR* inner_ast = parse_expression(&(*i), 0);
	overflow_control(*i, MISSING_SEMICOLON);

	result.call.callee = inner_ast->call.callee;
	result.call.argc = inner_ast->call.argc;
	result.call.args = NULL;
	
	for (unsigned int j = 0; j < inner_ast->call.argc; j++)
	{
		result.call.args = realloc(result.call.args, sizeof(EXPR*) * (j + 1));
		result.call.args[j] = inner_ast->call.args[j];
	}

	if (tokens[*i].token_type != SYMBOL_SEMICOLON)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

	return result;
}

AST parse_return(unsigned int *i, unsigned int c)
{
	AST result;
	
	result.type = RETURN;
	result.seq = c;

	(*i)++;
	result._return.value = parse_expression(&(*i), 0);

	overflow_control(*i, MISSING_SEMICOLON);

	if (tokens[*i].token_type != SYMBOL_SEMICOLON)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

	return result;
}

AST parse_jumper(unsigned int *i, unsigned int c)
{
	AST result;
	
	result.type = JUMPER;
	result.seq = c;

	(*i)++;
	
	if (tokens[*i].token_type != SYMBOL_LPAREN)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_JUMPER);

	(*i)++;

	result.jumper.condition = parse_expression(&(*i), 0);

	if (tokens[*i].token_type != SYMBOL_RPAREN)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_JUMPER);

	(*i)++;
	
	if (tokens[*i].token_type != IDENTIFIER)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_JUMPER);

	result.jumper.label = tokens[*i].value;

	(*i)++;

	overflow_control(*i, MISSING_SEMICOLON);

	if (tokens[*i].token_type != SYMBOL_SEMICOLON)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, MISSING_SEMICOLON);

	return result;
}

AST parse_label(unsigned int *i, unsigned int c)
{
	AST result;

	result.type = LABEL;
	result.seq = c;

	(*i)++;
	
	if (tokens[*i].token_type != IDENTIFIER)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_LABEL);
	
	result.label.name = tokens[*i].value;
	
	return result;
}

AST* ast;
unsigned int ast_counter = 0;
char* scope;
unsigned int scope_line = 0;
unsigned int scope_column = 0;

AST parse_function(unsigned int *i, unsigned int c)
{
	AST result;

	result.type = FUNCTION;
	result.seq = c;

	(*i)++;

	if (tokens[*i].token_group != DTYPE)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);

	result.function.type = tokens[*i].value;

	(*i)++;

	if (tokens[*i].token_type != IDENTIFIER)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);

	result.function.name = tokens[*i].value;
	scope = strdup(tokens[*i].value);

	(*i)++;

	if (tokens[*i].token_type != SYMBOL_LPAREN)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);

	(*i)++;

	/* PARSE FUNCTION PARAMETERS */

	result.function.args = NULL;
	unsigned int argc = 0;

	while (tokens[*i].token_type != SYMBOL_RPAREN)
	{
		if (tokens[*i].token_type == SYMBOL_COMMA)
			(*i)++;

		if (tokens[*i].token_group == DTYPE)
		{	
			result.function.args = realloc(result.function.args, 
			        		sizeof(*result.function.args) * (argc + 1));
			result.function.args[argc].type = tokens[*i].value;

			(*i)++;

			if (tokens[*i].token_type == IDENTIFIER)
			{
				result.function.args[argc].name = tokens[*i].value;
				(*i)++;
				argc++;

				continue;
			}

			parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);
		}

		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);
	}

	result.function.argc = argc;

	(*i)++;

	if (tokens[*i].token_type != SYMBOL_LBRACE)
		parser_error(source_files[0], tokens[*i].line, tokens[*i].column, UNEXPECTED_FUNCTION);

	return result;
}

void parser_main()
{
	ast = malloc(sizeof(AST) * 2);

	scope = malloc(255);
	strcpy(scope, "global");

	for (unsigned int i = 0; i < tokens_counter; i++)
	{
		// If brace is closed then clean the scope.
		if (strcmp(scope, "global") != 0 && tokens[i].token_type == SYMBOL_RBRACE)
		{
			strcpy(scope, "global");
			continue;
		}

		/* PARSE DATA TYPE */

		const unsigned int tmp_line = tokens[i].line;
		const unsigned int tmp_column = tokens[i].column;

		if (tokens[i].token_group == DTYPE)
		{
			ast[ast_counter] = parse_var(&i, 0, ast_counter);
			ast[ast_counter].scope = strdup(scope);
			ast[ast_counter].line = tmp_line;
			ast[ast_counter].column = tmp_column;
			ast[ast_counter].scpline = scope_line;
			ast[ast_counter].scpcolumn = scope_column;

			ast_counter++;
			ast = realloc(ast, sizeof(AST) * ast_counter * 2);
			continue;
		}

		/* PARSE IDENTIFIER ASSIGNMENT */

		overflow_control(i, MISSING_ARG);
		if (tokens[i].token_type == IDENTIFIER && tokens[i + 1].token_type == SYMBOL_ASSIGN)
		{
			ast[ast_counter] = parse_assignment(&i, ast_counter);
			ast[ast_counter].scope = strdup(scope);
			ast[ast_counter].line = tokens[i].line;
			ast[ast_counter].column = tokens[i].column;
			ast[ast_counter].scpline = scope_line;
			ast[ast_counter].scpcolumn = scope_column;

			ast_counter++;
			ast = realloc(ast, sizeof(AST) * ast_counter * 2);
			continue;
		}

		/* PARSE CALL */

		overflow_control(i, MISSING_ARG);
		if (tokens[i].token_type == IDENTIFIER && tokens[i + 1].token_type == SYMBOL_LPAREN)
		{
			ast[ast_counter] = parse_call(&i, ast_counter);
			ast[ast_counter].scope = strdup(scope);
			ast[ast_counter].line = tokens[i].line;
			ast[ast_counter].column = tokens[i].column;
			ast[ast_counter].scpline = scope_line;
			ast[ast_counter].scpcolumn = scope_column;

			ast_counter++;
			ast = realloc(ast, sizeof(AST) * ast_counter * 2);
			continue;
		}

		/* PARSE */

		switch (tokens[i].token_type)
		{
			case KEYWORD_INCLUDE:
				ast[ast_counter] = parse_include(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			case KEYWORD_MACRO:
				ast[ast_counter] = parse_macro(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			case KEYWORD_UNSIGNED:
				ast[ast_counter] = parse_var(&i, 1, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			case KEYWORD_FUNCTION:
				if (strcmp(scope, "global") != 0)
					parser_error(source_files[0], tokens[i].line, tokens[i].column, UNEXPECTED_FUNCTION);

				ast[ast_counter] = parse_function(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;

				scope_line = tokens[i].line;
				scope_column = tokens[i].column;

				ast_counter++;
				break;
			case KEYWORD_RETURN:
				ast[ast_counter] = parse_return(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			case KEYWORD_JUMPER:
				ast[ast_counter] = parse_jumper(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			case KEYWORD_LABEL:
				ast[ast_counter] = parse_label(&i, ast_counter);
				ast[ast_counter].scope = strdup(scope);
				ast[ast_counter].line = tokens[i].line;
				ast[ast_counter].column = tokens[i].column;
				ast[ast_counter].scpline = scope_line;
				ast[ast_counter].scpcolumn = scope_column;
				ast_counter++;
				break;
			default:
				parser_error(source_files[0], tokens[i].line, tokens[i].column, UNEXPECTED);
		}

		ast = realloc(ast, sizeof(AST) * ast_counter * 2);
	}

	if (strcmp(scope, "global") != 0)
		parser_error(source_files[0], scope_line, scope_column, UNEXPECTED_FUNCTION);
}
