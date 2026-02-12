#ifndef SYS_H
#define SYS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef char** _ar;
typedef unsigned int _arsize;

int read_f(const char *fname);
void ar_at(_ar *ary, char* data, int index, _arsize *arsize);
int _isalnum(const char c);
void clear_buffer(char *buffer, unsigned int *buffer_counter);
char* open_buffer(const char* source, unsigned int *buffersize);
int _isbinary(char value);

#endif
