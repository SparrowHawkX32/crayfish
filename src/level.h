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
} Level;


int serialise_level(Level* level, String* buf);

int deserialise_level(const char* buf, Level* level);

int load_level(const char* path, Level* level);

int load_levels_from_path(const char* path, Level** levels);

void unload_level(Level* level);

void unload_levels(Level** levels, int length);

#endif
