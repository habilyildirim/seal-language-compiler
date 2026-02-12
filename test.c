/* This file print arrays for test
*/

#include "lexer.h"

void print_tokens(unsigned int ex)
{
	for (unsigned int i = 0; i < tokens_counter; i++)
		printf("%d %s line->%d column->%d\n",
			i,
			tokens[i].value,
			tokens[i].line,
			tokens[i].column
		);

	if (ex)
		exit(0);
}
