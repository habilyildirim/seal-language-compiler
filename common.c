#include "common.h"

int read_f(const char *fname)
{
    FILE *file;

    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 0;
    }

    return -1;
}

void ar_at(_ar *ary, char* data, int index, _arsize *arsize)
{
	(*arsize)++;
	*ary = realloc(*ary, sizeof(char*) * (*arsize));
	(*ary)[index] = data;
}

int _isalnum(const char c)
{
	/*
		[a-zA-Z0-9], _
	*/

	if(isalnum((char)c) || c == '_')
		return 1;

	return 0;
}

void clear_buffer(char *buffer, unsigned int *buffer_counter)
{
	memset(buffer, 0, sizeof((char*)buffer));
	(*buffer_counter) = 0;
}

char* open_buffer(const char* source, unsigned int *buffersize)
{
	FILE* buffer_source;

	if(!(buffer_source = fopen(source, "rb")))
	{
		fprintf(stderr, "fopen error");
		return NULL;
	}

	if(fseek(buffer_source, 0, SEEK_END) != 0)
	{
		fprintf(stderr, "fseek error");
		fclose(buffer_source);
		return NULL;
	}

	unsigned long buffer_size = ftell(buffer_source);
	                            rewind(buffer_source);

	(*buffersize) = buffer_size;

	char* buffer = (char*)malloc(buffer_size + 1);

	fread(buffer, 1, buffer_size, buffer_source);
	buffer[buffer_size] = '\0';

	fclose(buffer_source);

	return buffer;
}

int _isbinary(char value)
{
	if(value == '0' || value == '1')
		return 1;

	return 0;
}
