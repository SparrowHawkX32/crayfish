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

int find_levels_in_dir(const char* path, char** levels);

void save_level(Level* level, const char* path);

void unload_level(Level* level);


#endif
