#ifndef CRAYFISH_LEVEL_H
#define CRAYFISH_LEVEL_H

#include "util.h"
#include <raylib.h>

typedef struct {
  char name[128];
  unsigned int width;
  unsigned int height;
  unsigned char* map;
  Vector2 spawn_pos;
  float spawn_rot;
  char atlas_name[128];
  Color floor_color;
  Color ceil_color;
  Color air_color;
} Level;


void serialise_level(Level* level, String* buf);

int deserialise_level(const char* buf, size_t buf_len, Level* level);

int load_level_file(const char* path, Level* level);

void save_level_file(Level* level, const char* path);

void unload_level(Level* level);


#endif
