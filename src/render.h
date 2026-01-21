#ifndef CRAYFISH_RENDER_H
#define CRAYFISH_RENDER_H

#include <raylib.h>
#include <stdbool.h>
#include "level.h"

#define DEFAULT_RENDER_DIST 5.0f
#define DEFAULT_FOCAL_LEN 5.0f
#define DEFAULT_RENDER_X 80
#define DEFAULT_RENDER_Y 60

typedef struct {
  Level* level;
  Vector2 cam_pos;
  Vector2 cam_forward;
  Vector2 cam_right;
  float render_dist;
  float focal_len;
  Vector2 render_size;
} RenderContext;

typedef struct {
  unsigned char atlas_idx;
  bool hit;
  float distance;
  float wall_pos;

} CastResult;


void cast_ray(const RenderContext* ctx, float screen_pos, CastResult* result);


void render_scene(const RenderContext* ctx, RenderTexture* target);


void apply_textures(const RenderContext* ctx, Texture* render);


#endif
