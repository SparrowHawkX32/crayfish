#include "util.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
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
  memcpy(string->data + string->length - (string->length > 1 ? 2 : 1), data, bytes);
  string->length += bytes + 1;
}


void string_zero(String *string) {
  memset(string->data, 0, string->length);
  string->length = 1;
}

int verify_filename(char* text, char* out, bool allow_empty) {
  bool valid = true;
  char c;
  for (int i = 0; valid && (c = text[i]) != '\0'; i++) {
    if (!isalnum(c) && !strchr("_-", c)) valid = false;
  }
  if ((!allow_empty && text[0] == '\0') || !valid) {
    strcpy(text, out);
    return 0;
  }
  strcpy(out, text);
  return 1;
}

int parse_uint(char* text, unsigned int* out, unsigned int min, unsigned int max, unsigned int fallback) {
  errno = 0;
  char* endptr = 0;
  long val = strtol(text, &endptr, 10);
  if (errno == ERANGE || (val == 0 && text == endptr) || val < min || val > max) {
    snprintf(text, 10, "%d", fallback);
    return 0;
  }
  *out = val;
  snprintf(text, 10, "%d", *out);
  return 1;
}


int parse_float(char* text, float* out, float min, float max, float fallback) {
  errno = 0;
  char* endptr = 0;
  double val = strtod(text, &endptr);
  if (errno == ERANGE || (val == 0 && text == endptr) || val < min || val > max) {
    snprintf(text, 10, "%.2f", fallback);
    return 0;
  }
  *out = val;
  snprintf(text, 10, "%.2f", *out);
  return 1;
}

int parse_color(char* text, Color* out, Color fallback) {
  errno = 0;
  char* endptr = 0;
  long val = strtol(text, &endptr, 16);
  if (errno == ERANGE || val < 0 || val > UINT_MAX || (val == 0 && text == endptr)) {
    snprintf(text, 10, "%08X", ColorToInt(fallback));
    return 0;
  }
  *out = GetColor(val);
  snprintf(text, 10, "%08X", ColorToInt(*out));
  return 1;
}
