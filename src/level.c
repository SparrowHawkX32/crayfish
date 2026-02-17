#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "level.h"
#include "util.h"


void serialise_level(Level* level, String* buf) {
  unsigned char* map_pointer = level->map;
  level->map = 0;
  string_append_bytes(buf, level, sizeof(Level));
  level->map = map_pointer;

  string_append_bytes(buf, level->map, 
      level->width * level->height);
}


int deserialise_level(const char* buf, size_t buf_len, Level* level) {
  if (sizeof(Level) > buf_len) {
    printf("ERROR: truncated level data!\n");
    return 1;
  }
  memcpy(level, buf, sizeof(Level));

  size_t map_size = level->width * level->height;
  if (map_size > buf_len - sizeof(Level)) {
    printf("ERROR: truncated level map data!\n");
    return 1;
  }
  level->map = malloc(map_size);
  memcpy(level->map, buf + sizeof(Level), map_size);

  return 0;
}


int load_level_file(const char* path, Level* level) {
  struct stat file_info;
  int result;
  char* level_bytes;
  
  result = stat(path, &file_info);
  if (result != 0) {
    printf("ERROR: failed to get file info for %s!\n", path);
    return 1;
  }

  if ((size_t)file_info.st_size < sizeof(Level)) {
    printf("ERROR: incorrect level file!\n");
    return 1;
  }

  FILE* level_file = fopen(path, "r");
  if (level_file == NULL) {
    printf("ERROR: failed to open file %s!\n", path);
    return 1;
  }

  level_bytes = malloc(file_info.st_size);
  size_t bytes_read = fread(level_bytes, 1, file_info.st_size, level_file);
  if (bytes_read < (size_t)file_info.st_size) {
    printf("ERROR: level file truncated!\n");
    return 1;
  }

  result = deserialise_level(level_bytes, bytes_read, level);

  free(level_bytes);
  return result;
}


void save_level_file(Level* level, const char* dir) {
  String level_txt, file_path;
  string_init(&level_txt);
  string_init(&file_path);
  serialise_level(level, &level_txt);

  string_append(&file_path, dir);
  if (file_path.length > 1 && file_path.data[file_path.length - 1] != '/') {
    string_append(&file_path, "/");
  }
  string_append(&file_path, level->name);

  FILE* level_file = fopen(file_path.data, "w");
  if (level_file == NULL) {
    printf("ERROR: failed to open file %s!\n", file_path.data);
    free(level_txt.data);
    free(file_path.data);
    return;
  }

  size_t bytes_written = fwrite(level_txt.data, 1, level_txt.length - 1, level_file);
  if (bytes_written < level_txt.length - 1) {
    printf("ERROR: failed to write level data to %s!\n", file_path.data);
  }

  fclose(level_file);
  free(level_txt.data);
  free(file_path.data);
}


void unload_level(Level* level) {
  free(level->map);
}
