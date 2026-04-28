#ifndef SYS_H
#define SYS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

typedef char** _ar;
typedef uint _arsize;

int read_f(const char *fname);
void ar_at(_ar *ary, char* data, int index, _arsize *arsize);
bool _isalnum(const char c);
void clear_buffer(char *buffer, uint *buffer_counter);
char* open_buffer(const char* source, uint *buffersize);
bool _isbinary(char value);
bool isbinop (const char* value);
bool is_integer(const char* c);

#endif
