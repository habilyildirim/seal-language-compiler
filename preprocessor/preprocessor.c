/*

	Seal Compiler - Preprocessor

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

#include "preprocessor.h"
#include "../diagnostic.h"
#include <time.h>

char* top;
char* bottom;
char* middle;

uint middle_size = 128;

uint middlebuffer_size = 128;
uint topbuffer_size = 128;
uint bottombuffer_size = 128;

char* include_handler(char* rf, uint rf_counter, uint i)
{
	char* for_top;
	uint c = 0;

	char* for_middle;
	uint d = 0;
	
	i++;
	for(;rf[i] != '"'; i++)
	{
		if (i > rf_counter)
			return NULL;
		
		if (rf[i] == ' ')
		{
			for_top[c] = rf[i];
			c++;
			continue;
		}

		return NULL;
	}
	for_top[c] = rf[i];
	c++;

	i++;
	for(;rf[i] != '"'; i++)
	{
		if (i > rf_counter)
			return NULL;

		for_top[c] = rf[i];
		c++;

		for_middle[d] = rf[i];
		d++;

		if (d > middle_size)
		{
			middlebuffer_size*=2;
			middle = realloc(middle, middlebuffer_size);
		}
	}
	for_top[c] = rf[i];
	for_top[c + 1] = '\0';
	for_middle[d] = '\0';

	middle = open_buffer(for_middle, &middle_size);
	return for_top;
}

void sync_top(char* *top, char* synced, uint *i)
{
	(*i)++;
	uint c = 0;
	for (;synced[c] != '\0'; c++)
	{
		if (*i > topbuffer_size)
		{
			topbuffer_size*=2;
			top = realloc(top, topbuffer_size);	
		}

		(*top)[*i] = synced[c];
		(*i)++;
	}
	(*top)[*i] = '\n';
	(*top)[*i + 1] = '\0';
}

void get_bottom(char* rf, uint start, uint stop)
{
	uint l = 0;

	for (uint c = start + 1; c != stop; c++)
	{
		if (l > bottombuffer_size)
		{
			bottombuffer_size*=2;
			bottom = realloc(top, bottombuffer_size);	
		}

		bottom[l] = rf[c];
		l++;
	}
	bottom[l] = '\0';
}

void pp_main(char* *converted)
{
	uint rf_counter = 0;
	char* root_file = open_buffer(*converted, &rf_counter);

	char* prep_buffer;
	uint pb_size = 0;
	
	top = malloc(topbuffer_size);
	middle = malloc(middlebuffer_size);
	bottom = malloc(bottombuffer_size);

	prep_buffer = malloc(128);

	for (uint i = 0; i < rf_counter; i++)
	{
		top[i] = root_file[i];

		if (i > topbuffer_size)
		{
			topbuffer_size*=2;
			top = realloc(top, topbuffer_size);	
		}

		if (root_file[i] == ' ' || root_file[i] == '\n')
		{
			clear_buffer(prep_buffer, &pb_size);
			continue;
		}

		prep_buffer[pb_size] = root_file[i];
		prep_buffer[pb_size + 1] = '\0';
		pb_size++;

		if (strcmp(prep_buffer, "INCLUDE") == 0)
		{	
			char* result = include_handler(root_file, rf_counter, i);
			if (result == NULL)
				return;

			sync_top(&top, result, &i);
			get_bottom(root_file, i, rf_counter);

			printf("%s", top);
			printf("%s", middle);
			printf("%s", bottom);
			exit(0);
		}
	}
}
