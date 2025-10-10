#ifndef CRAYFISH_UTIL_H
#define CRAYFISH_UTIL_H

#define STRING_GROWTH_FAC 2
#define STRING_INITIAL_SIZE 4

#include <stddef.h>


typedef struct {
  char* data;
  size_t length;
  size_t capacity;
} String;

void _string_realloc(String* string);

void string_init(String* string);

void string_append(String* string, const char* str);

float wrap_geq(float f, float min, float max);


#endif
