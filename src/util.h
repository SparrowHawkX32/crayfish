#ifndef CRAYFISH_UTIL_H
#define CRAYFISH_UTIL_H

#include <raylib.h>
#include <stddef.h>

#define STRING_GROWTH_FAC 2
#define STRING_INITIAL_SIZE 4


typedef struct {
  char* data;
  size_t length;
  size_t capacity;
} String;


void _string_resize(String* string);

void string_init(String* string);

void string_append(String* string, const char* str);

void string_append_bytes(String* string, void* data, size_t bytes);

void string_zero(String* string);
 
int verify_filename(char* text, char* out, bool allow_empty);

int parse_uint(char* text, unsigned int* out, unsigned int min, unsigned int max, unsigned int fallback);

int parse_float(char* text, float* out, float min, float max, float fallback);

int parse_color(char* text, Color* out, Color fallback);


#endif
