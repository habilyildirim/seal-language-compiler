/* This file print arrays for test
*/

#include "lexer.h"
#include "parser.h"

void print_tokens(uint ex)
{
	for (uint i = 0; i < tokens_counter; i++)
	{
		printf("%d %s line->%d column->%d\n",
			i,
			tokens[i].value,
			tokens[i].line,
			tokens[i].column
		);
	}
	
	if (ex)
		exit(0);
}

	void print_expr(EXPR* e) 
	{
    	if (!e) 
    		return;

    	switch (e->type)
    	{
        	case NODE_INT_LITERAL: 
            	printf("%s", e->literal); 
            	break;
        	case NODE_IDENTIFIER: 
            	printf("%s", e->identifier); 
            	break;
        	case NODE_BINARY:
            	printf("(");
            	print_expr(e->binary.left);
            	printf(" %s ", e->binary.op);
            	print_expr(e->binary.right);
            	printf(")");
            	break;
        	case NODE_UNARY:
            	printf("(%s", e->unary.op);
            	print_expr(e->unary.value);
            	printf(")");
            	break;
            case NODE_CALL:
            	printf("(");
            	printf("call>%s ", e->call.callee);
            	
            	for (uint i = 0; i < e->call.argc; i++)
            	{
            		print_expr(e->call.args[i]);
            		printf("<");
            	}
            	
            	printf(")");
            	break;
        	default:
    	}
	}

void print_ast(uint ex)
{
	for (uint i = 0; i < ast_counter; i++)
	{
		switch (ast[i].type)
		{
			case MACRO:
				printf("AST %d - MACRO \n", i);
				printf("	->MACRO NAME; %s\n", ast[i].macro.name);
				printf("	->MACRO VALUE; %s\n", ast[i].macro.value);
				break;
	  		case INCLUDE:
				printf("AST %d - INCLUDE \n", i);
				printf("	->LIB; %s\n", ast[i].include.lib);
				break;
			case FUNCTION:
				printf("AST %d - FUNCTION \n", i);
				printf("	->FUNCTION TYPE; %s\n", ast[i].function.type);
				printf("	->FUNCTION NAME; %s\n", ast[i].function.name);
				printf("	->ARGC; %d\n", ast[i].function.argc);
				break;
			case CALL:
				printf("AST %d - CALL \n", i);
				printf("	->CALL NAME; %s\n", ast[i].call.callee);
				printf("	->ARGC; %d\n", ast[i].call.argc);
				printf("	->ARGS -> \n");

				for (uint j = 0; j < ast[i].call.argc; j++)
				{
					printf("		ARG %d ", j);
					print_expr(ast[i].call.args[j]);
					printf("\n");
				}

				break;
			case VAR:
				printf("AST %d - VARIABLE \n", i);
				printf("	->VAR TYPE; %s\n", ast[i].var.type);
				printf("	->VAR NAME; %s\n", ast[i].var.name);
				printf("	->VAR VALUE; "); 

				if(ast[i].var.value != NULL)
					print_expr(ast[i].var.value);

				printf("\n");
				break;
			case UVAR:
				printf("AST %d - UNSIGNED VARIABLE \n", i);
				printf("	->VAR TYPE; %s\n", ast[i].var.type);
				printf("	->VAR NAME; %s\n", ast[i].var.name);
				printf("	->VAR VALUE; ");

				if(ast[i].var.value != NULL)
					print_expr(ast[i].var.value);

				printf("\n");
				break;
			case PARSE_ASSIGNMENT:
				printf("AST %d - ASSIGNMENT \n", i);
				printf("	->VAR TYPE; %s\n", ast[i].assignment.name);
				printf("	->VAR VALUE; "); 
				print_expr(ast[i].assignment.value);
				printf("\n");
				break;
			case JUMPER:
				printf("AST %d - JUMPER \n", i);
				printf("	->CONDITION; ");
				print_expr(ast[i].jumper.condition);
				printf("\n");
				printf("	->LABEL; %s\n", ast[i].jumper.label);
				break;
			case RETURN:
				printf("AST %d - RETURN \n", i);
				printf("	->RETURN VALUE; ");
				print_expr(ast[i]._return.value);
				printf("\n");
				break;
			case LABEL:
				printf("AST %d - LABEL \n", i);
				printf("	->NAME; %s\n", ast[i].label.name);
				break;
			default:
		}

		printf("	->SCOPE; %s \n", ast[i].scope);
	}

	printf("AST COUNTER; %d\n\n", ast_counter);

	if (ex)
		exit(0);
}
