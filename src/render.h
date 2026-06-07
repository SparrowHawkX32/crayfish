#ifndef CRAYFISH_RENDER_H
#define CRAYFISH_RENDER_H

#include <raylib.h>
#include <stdbool.h>
#include "level.h"

#define DEFAULT_RENDER_DIST 5.0f
#define DEFAULT_FOV 1.0f
#define DEFAULT_RENDER_X 80
#define DEFAULT_RENDER_Y 60

#define ROT_SPEED 120
#define MOUSE_SENS 0.1f // Radians per pixel
#define MOVE_SPEED 1.0f

typedef struct {
  Level* level;
  Vector2 cam_pos;
  Vector2 cam_forward;
  Vector2 cam_right;
  float render_dist;
  float fov;
  Vector2 render_size;
  Image atlas;
  Color* atlasColors;
  RenderTexture bg;
  RenderTexture target;
} RenderContext;

typedef struct {
  unsigned char atlas_idx;
  bool hit;
  float distance;
  float wall_pos;
} CastResult;


void init_render_ctx(RenderContext* ctx);

void unload_render_ctx(RenderContext* ctx);

void change_level(RenderContext* ctx, Level* level);

void change_atlas(RenderContext* ctx, const char* path);

void process_input(RenderContext* ctx);

void cast_ray(const RenderContext* ctx, float screen_pos, CastResult* result);

void render_scene(const RenderContext* ctx);

void draw_scene(const RenderContext* ctx);


#endif
