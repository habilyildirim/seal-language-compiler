#include "preprocessor.h"
#include "../diagnostic.h"
#include <time.h>

const char* diagnostic_ref;

char tmp_file[512];
FILE* tmp;

char* file = {0};
uint file_size = 0;

char* prep_buffer = NULL;
uint prep_buffer_size = 0;

uint prep_line = 1;
uint prep_column = 0;

void include_handler(const char* file_ref)
{
	uint ifile_size;
	char* inc_file = open_buffer(file_ref, &ifile_size);

	if (inc_file == NULL)
	{	
		prep_error(diagnostic_ref, prep_line,
			prep_column, INCFILE_NOT_EXISTS);
	}

	fputc('\n', tmp);
	uint c = 0;

	for (;c + 1 < ifile_size; c++)
		fputc(inc_file[c], tmp);

	if (inc_file[c - 1] != '@')
		prep_error(file_ref, 0, 0, END_SYMBOL);
}

uint include_analyzer(uint *i)
{
	(*i)++;

	for (;file[*i] != '\"'; (*i)++)
	{
		fputc(file[*i], tmp);
		if (file[*i] == ' ' || file[*i] == '\n')
			continue;

		return 0;	
	}
	fputc(file[*i], tmp);

	(*i)++;
	for (;file[*i] != '\"'; (*i)++)
	{
		if ((*i) + 1 > file_size)
			return 0;

		fputc(file[*i], tmp);
		prep_buffer[prep_buffer_size] = file[*i];
		prep_buffer[prep_buffer_size + 1] = '\0';
		prep_buffer_size++;
	}

	fputc(file[*i], tmp);
	include_handler(prep_buffer);
}

void prep_updatepos(char c)
{
	if (c == '\n')
	{
		prep_line++;
		return;
	}

	prep_column++;
}

void prep_analyzer(const char* sf)
{
	diagnostic_ref = sf;
	file = open_buffer(sf, &file_size);
	prep_buffer = malloc(512);
	
	for (uint i = 0; i < file_size; i++)
	{
		fputc(file[i], tmp);
		prep_updatepos(file[i]);

		if (file[i] == ' ' || file[i] == '\n')
		{
			clear_buffer(prep_buffer, &prep_buffer_size);
			continue;
		}

		if (isalpha(file[i]))
		{
			prep_buffer[prep_buffer_size] = file[i];
			prep_buffer[prep_buffer_size + 1] = '\0';
			prep_buffer_size++;
		}

		if (strcmp(prep_buffer, "INCLUDE") == 0)
		{
			clear_buffer(prep_buffer, &prep_buffer_size);

			if (!include_analyzer(&i))
				return;
		}
	}
}

void write_tmp(char* *converted)
{
	srand(time(NULL));
	uint tmpcode = rand();

	sprintf(tmp_file, "%d", tmpcode);	
	strcat(tmp_file, "sealpreptmp.seal");
	tmp = fopen(tmp_file, "w");

	if (tmp == NULL)
		prep_error(NULL, 0, 0, TMP_CNB_CREATED);

	(*converted) = tmp_file;
}

void pp_main(char* *converted, const char* sf)
{
	write_tmp(&(*converted));
	prep_analyzer(sf);
	fclose(tmp);
}
