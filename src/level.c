#include "level.h"
#include "util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define U8_TEXT_LEN 8 / 3 + 1


int serialise_level(Level* level, String* buf) {
  char value_text[128] = {0};

  string_append(buf, "name: ");
  string_append(buf, level->name);

  sprintf(value_text, "%d", level->width);
  string_append(buf, "\nwidth: ");
  string_append(buf, value_text);

  sprintf(value_text, "%d", level->height);
  string_append(buf, "\nheight: ");
  string_append(buf, value_text);

  size_t map_size = (U8_TEXT_LEN + 1) * level->width * level->height + 1;
  char* map_text = malloc(map_size);
  size_t bytes_written = 0;

  for (size_t y = 0; y < level->height; y++) {
    if (bytes_written == map_size) {
      printf("ERROR: Map buffer size mismatch!\n");
      return 1;
    }

    map_text[bytes_written++] = '\n';
    for (size_t x = 0; x < level->width; x++) {
      if (x == level->width - 1) {
        bytes_written += sprintf(
            map_text + bytes_written,
            "%d",
            level->map[y * level->width + x]);
        continue;
      }
      bytes_written += sprintf(
          map_text + bytes_written,
          "%d,",
          level->map[y * level->width + x]);
    }
  }
  string_append(buf, "\nmap:");
  string_append(buf, map_text);

  sprintf(value_text, "%.3f,%.3f", level->spawn_pos.x, level->spawn_pos.y);
  string_append(buf, "\nspawn_pos: ");
  string_append(buf, value_text);

  sprintf(value_text, "%.2f", level->spawn_rot);
  string_append(buf, "\nspawn_rot: ");
  string_append(buf, value_text);

  string_append(buf, "\natlas_name: ");
  string_append(buf, level->atlas_name);

  sprintf(value_text, "#%02x%02x%02x%02x",
      level->floor_color.r, 
      level->floor_color.g,
      level->floor_color.b,
      level->floor_color.a);
  string_append(buf, "\nfloor_color: ");
  string_append(buf, value_text);

  sprintf(value_text, "#%02x%02x%02x%02x",
      level->ceil_color.r, 
      level->ceil_color.g,
      level->ceil_color.b,
      level->ceil_color.a);
  string_append(buf, "\nceil_color: ");
  string_append(buf, value_text);

  free(map_text);
  return 0;
}


int deserialise_level(const char* buf, Level* level) {

  return 0;
}


int load_level(const char* path, Level* level) {
  
  return 0;
}


int load_levels_from_path(const char* path, Level** levels) {

  return 0;
}


void unload_level(Level* level) {
  free(level->map);
  free(level);
}


void unload_levels(Level** levels, int length) {
  for (int i = 0; i < length; i++) {
    unload_level(levels[i]);
  }
}
