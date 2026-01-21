#include "util.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void _string_resize(String *string) {
  string->data = realloc(string->data, string->capacity * STRING_GROWTH_FAC);
  memset(string->data + string->capacity, 0, string->capacity);
  string->capacity = string->capacity * STRING_GROWTH_FAC;
}


void string_init(String* string) {
  string->data = malloc(STRING_INITIAL_SIZE);
  string->capacity = STRING_INITIAL_SIZE;
  memset(string->data, 0, string->capacity);
  string->length = 1;
}


void string_append(String* string, const char* str) {
  size_t bytes_written = 0;
  while (1) {
    char* str_end = stpncpy(
        string->data + (string->length - 1),
        str + bytes_written, 
        (string->capacity - string->length) + 1);
    bytes_written += (str_end - string->data) - string->length;
    string->length = str_end - string->data + 1;

    if (string->length >= string->capacity) {
      bytes_written++;
      _string_resize(string);
    }
    else return;
  }
}


void string_append_bytes(String *string, void *data, size_t bytes) {
  while (string->capacity - string->length <= bytes) {
    _string_resize(string);
  }
  memcpy(string->data + string->length - 1, data, bytes);
  string->length += bytes + 1;
}
