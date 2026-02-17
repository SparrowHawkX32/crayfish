#ifndef CRAYFISH_RENDER_H
#define CRAYFISH_RENDER_H

#include <raylib.h>
#include <stdbool.h>
#include "level.h"

#define DEFAULT_RENDER_DIST 5.0f
#define DEFAULT_FOV 1.0f
#define DEFAULT_RENDER_X 80
#define DEFAULT_RENDER_Y 60

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
} RenderContext;

typedef struct {
  unsigned char atlas_idx;
  bool hit;
  float distance;
  float wall_pos;
} CastResult;


void change_level(RenderContext* ctx, Level* level, RenderTexture* background);

void cast_ray(const RenderContext* ctx, float screen_pos, CastResult* result);

void render_scene(const RenderContext* ctx, RenderTexture* target);


#endif
