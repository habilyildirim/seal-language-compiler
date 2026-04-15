/*

	Seal Compiler - Preprocessor Engine

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
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

/*
	top holds chars from the start of 
	the root file up to the include point

	middle holds chars from the include 
	point to the end of the included file

	bottom: holds chars from the end of the 
	included file to the end of the root file
*/

char* top;
char* middle;
char* bottom;

uint middle_size = 128;

uint topbuffer_size = 128;
uint middlebuffer_size = 128;
uint bottombuffer_size = 128;

char* root_file;
uint rf_counter = 0;

char* include_handler(char* rf, uint rf_counter, uint i)
{
    char* for_top = malloc(128);
    uint c = 0;

    char* for_middle = malloc(middlebuffer_size);
    uint d = 0;

    i++;
    for (;i < rf_counter && rf[i] != '"'; i++)
    {
        if (rf[i] == ' ')
        {
            if ((c + 1) % 128 == 0) 
            	for_top = realloc(for_top, (c + 1) * 2);

            for_top[c] = rf[i];
            c++;
            continue;
        }

        return NULL;
    }

    for_top[c] = rf[i];
	c++;

    i++;
    for (;i < rf_counter && rf[i] != '"'; i++)
    {
        if ((c + 1) % 128 == 0)
        {
        	for_top = realloc(for_top, (c + 1) * 2);
        	for_middle = realloc(for_middle, (c + 1) * 2);
        }

        for_top[c] = rf[i];
		c++;

        if (d + 1 >= middlebuffer_size)
        {
            middlebuffer_size *= 2;
            char* tmp = realloc(for_middle, middlebuffer_size);
            for_middle = tmp;
        }

        for_middle[d] = rf[i];
        d++;
    }
    
    for_top[c] = rf[i];
    c++;
    for_top[c] = '\0';
	for_middle[d] = '\0';

    middle = open_buffer(for_middle, &middle_size);

	if (middle == NULL) 
		return NULL;

	if (middle[strlen(middle) - 2] != '@')
		prep_error(for_middle, 0, 0, END_SYMBOL);
    
    return for_top;
}

void sync_top(char* synced, uint *i)
{
	/*
		add remaining chars to top 
		buffer after detected "INCLUDE" 
	*/
    (*i)++;

    for (uint c = 0; synced[c] != '\0'; c++)
    {
        if (*i + 1 >= topbuffer_size)
        {
            topbuffer_size *= 2;
            char* tmp = realloc(top, topbuffer_size);
            top = tmp;
        }

        top[*i] = synced[c];
        (*i)++;
    }

    if (*i + 2 >= topbuffer_size)
    {
        topbuffer_size *= 2;
        top = realloc(top, topbuffer_size);
    }

    top[*i] = '\n';
    top[*i + 1] = '\0';
}

void get_bottom(char* rf, uint start, uint stop)
{
    uint l = 0;

    for (uint c = start + 1; c < stop; c++)
    {
        if (l + 1 >= bottombuffer_size)
        {
            bottombuffer_size *= 2;
            char* tmp = realloc(bottom, bottombuffer_size);
            bottom = tmp;
        }

        bottom[l] = rf[c];
        l++;
    }

    bottom[l] = '\0';
}

void clear_posstrg()
{
    clear_buffer(middle, &middlebuffer_size);
    clear_buffer(bottom, &bottombuffer_size);

    middlebuffer_size = 128;
    bottombuffer_size = 128;

    middle = malloc(middlebuffer_size);
    bottom = malloc(bottombuffer_size);
}


uint8_t delimiter_key = 0;
uint8_t commentline_key = 0;
uint8_t blockcomment_key = 0;

void pass_scan(char chr, char* str)
{
	if ( (commentline_key == 0 && delimiter_key == 0) && chr == '"')
	{
		delimiter_key = 1;
		return;
	}
	if (delimiter_key == 1 && chr == '"')
		delimiter_key = 0;

	if ( (delimiter_key == 0 && commentline_key == 0) && chr == '~')
	{
		commentline_key = 1;
		return;
	}
	if (commentline_key == 1 && chr == '\n')
		commentline_key = 0;

	if ((delimiter_key == 0 && commentline_key == 0) && strcmp(str, "/~") == 0)
	{
		if (blockcomment_key == 0)
		{
			blockcomment_key = 1;
			return;	
		}
	}
	if (blockcomment_key == 1 && strcmp(str, "~/") == 0)
		commentline_key = 0;
}

void analyzer(char* converted)
{
	root_file = open_buffer(converted, &rf_counter);

	char* prep_buffer = malloc(128);
	uint pb_size = 0;

	top = malloc(topbuffer_size);
	middle = malloc(middlebuffer_size);
	bottom = malloc(bottombuffer_size);

	for (uint i = 0; i < rf_counter; i++)
	{
	    if (i + 1 >= topbuffer_size)
	    {
	        topbuffer_size *= 2;
	        char* tmp = realloc(top, topbuffer_size);
	        top = tmp;
	    }
	    top[i] = root_file[i];

		pass_scan(root_file[i], prep_buffer);
		if (delimiter_key == 1 || commentline_key == 1)
			continue;

	    if (root_file[i] == ' ' || root_file[i] == '\n')
	    {
	        pb_size = 0;
	        prep_buffer[0] = '\0';
	        continue;
	    }

	    if (pb_size + 1 >= 128)
	        continue;

	    prep_buffer[pb_size] = root_file[i];
	    pb_size++;
	    prep_buffer[pb_size] = '\0';

	    if (strcmp(prep_buffer, "INCLUDE") == 0)
	    {
	        char* result = include_handler(root_file, rf_counter, i);

	        if (result == NULL)
	        	return;

	        sync_top(result, &i);
	        get_bottom(root_file, i, rf_counter);

	        char* tmp_root = strdup(top);
	        size_t new_size = strlen(tmp_root) + strlen(middle) + strlen(bottom) + 1;

	        char* resized = realloc(tmp_root, new_size);
	        tmp_root = resized;

	        strcat(tmp_root, middle);
	        strcat(tmp_root, bottom);

	        root_file = tmp_root;
	        rf_counter = strlen(root_file);

	        clear_posstrg();
	        pb_size = 0;
	    }
	}
}

typedef struct
{
	char* file_name;
	uint line;
}
dpfile_layer;

char* diagnostic_mark;
void diagnostic_marker(char* rf_path)
{
	uint dpb_size = 128;
	uint dpb_counter = 0;
	char* dp_buffer = malloc(dpb_size * sizeof(char));

	uint layer_size = 128;
	uint layer_point = 0;
	dpfile_layer* layer = malloc(layer_size * sizeof(dpfile_layer));

	layer[layer_point].file_name = rf_path;
	layer[layer_point].line = 1;
	diagnostic_mark = malloc(1);
	diagnostic_mark[0] = '\0';

	for (uint i = 0; root_file[i] != '\0'; i++)
	{
	    if (dpb_counter + 1 >= dpb_size)
	    {
	        dpb_size *= 2;
	        char* tmp = realloc(dp_buffer, dpb_size);
	        dp_buffer = tmp;
	    }

	    if (layer_point + 1 >= layer_size)
	    {
	        layer_size *= 2;
	        layer = realloc(layer, layer_size * sizeof(dpfile_layer));
	    }

		if (root_file[i] == '@')
		{
			if (layer_point == 0)
	 		{
				printf("%d", layer[layer_point].line);
				prep_error(rf_path, layer[layer_point].line, dpb_counter, END_SYMBOL_WRONG);
			}
				

			layer_point--;
		}

		if (root_file[i] == '\n')
		{
			char* dm_tmpline = NULL;

			while (*layer[layer_point].file_name == ' ' || *layer[layer_point].file_name == '\n')
				layer[layer_point].file_name++;

			asprintf(&dm_tmpline, "%s:%d\n", layer[layer_point].file_name, layer[layer_point].line); 
			diagnostic_mark = realloc(diagnostic_mark, strlen(diagnostic_mark) + strlen(dm_tmpline) + 1); 
			strcat(diagnostic_mark, dm_tmpline);

			layer[layer_point].line++;
		}

		pass_scan(root_file[i], dp_buffer);
		if (delimiter_key == 1 || commentline_key == 1)
			continue;

	    if (root_file[i] == ' ' || root_file[i] == '\n')
	    {
	        dpb_counter = 0;
	        dp_buffer[0] = '\0';
	        continue;
	    }

	    dp_buffer[dpb_counter] = root_file[i];
	    dpb_counter++;
	    dp_buffer[dpb_counter] = '\0';

		if (strcmp(dp_buffer, "DEFINE") == 0)
		{
			// HERE
		}

	    if (strcmp(dp_buffer, "INCLUDE") == 0)
	    {
	    	char* result = include_handler(root_file, rf_counter, i);
			if (result == NULL)
				return;

	    	layer_point++;
	    	layer[layer_point].file_name = result;
	    	layer[layer_point].line = 1;
	    }
	}
}

void pp_main(char* *converted)
{
	analyzer(*converted);
	diagnostic_marker(*converted);
}
