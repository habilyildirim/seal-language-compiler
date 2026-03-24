#ifndef SYS_H
#define SYS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef char** _ar;
typedef uint _arsize;

int read_f(const char *fname);
void ar_at(_ar *ary, char* data, int index, _arsize *arsize);
int _isalnum(const char c);
void clear_buffer(char *buffer, uint *buffer_counter);
char* open_buffer(const char* source, uint *buffersize);
int _isbinary(char value);
int isbinop (const char* value);
int is_integer(const char* c);

#endif
