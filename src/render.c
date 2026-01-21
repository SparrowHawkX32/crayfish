#include "render.h"
#include <limits.h>
#include <raylib.h>
#include <raymath.h>

// h_offset should be from -0.5 to 0.5
void cast_ray(const RenderContext *ctx, float h_offset, CastResult *result) {
  // Initialise variables
  Vector2 ray_dir = Vector2Add(ctx->cam_forward, Vector2Scale(ctx->cam_right, h_offset));
  int step_x;
  int step_y;
  int grid_x = (int)ctx->cam_pos.x;
  int grid_y = (int)ctx->cam_pos.y;
  float hyp_dist_x = fabsf(1 / ray_dir.x);
  float hyp_dist_y = fabsf(1 / ray_dir.y);
  float dist_x;
  float dist_y;
  float prev_dist_x = 0;
  float prev_dist_y = 0;
  int hit_side = 0;

  result->atlas_idx = 0;
  result->hit = false;

  // Set grid step and cell border distance based on ray direction
  if (ray_dir.x >= 0 ) {
    step_x = 1;
    dist_x = (grid_x + 1.0f - ctx->cam_pos.x) * hyp_dist_x;
  }
  else {
    step_x = -1;
    dist_x = (ctx->cam_pos.x - grid_x) * hyp_dist_x;
  }
  if (ray_dir.y >= 0 ) {
    step_y = 1;
    dist_y = (grid_y + 1.0f - ctx->cam_pos.y) * hyp_dist_y;
  }
  else {
    step_y = -1;
    dist_y = (ctx->cam_pos.y - grid_y) * hyp_dist_y;
  }

  while (result->hit == false) {
    // Step ray to closest x/y border
    if (dist_x < dist_y) {
      dist_x += hyp_dist_x;
      grid_x += step_x;
      hit_side = 0;
    }
    else {
      dist_y += hyp_dist_y;
      grid_y += step_y;
      hit_side = 1;
    }

    // Break if past render distance
    if ((hit_side == 0 && prev_dist_x >= ctx->render_dist) || 
        (hit_side == 1 && prev_dist_y >= ctx->render_dist)) break;

    // Check if a hit has occurred
    size_t map_offset = Wrap(grid_y, 0, ctx->level->height) * ctx->level->width + Wrap(grid_x, 1, ctx->level->width);
    result->atlas_idx = ctx->level->map[map_offset];
    if (result->atlas_idx != 0) result->hit = true;

    prev_dist_x = dist_x;
    prev_dist_y = dist_y;
  }

  // Ignore rays out of render distance
  if (!result->hit) return; 
  
  // Calculate ray length and position along the hit wall
  float ray_len;
  float wall_pos;
  if (hit_side == 0) {
    ray_len = dist_x - hyp_dist_x;
    wall_pos = ctx->cam_pos.y + ray_len * ray_dir.y;
  }
  else {
    ray_len = dist_y - hyp_dist_y;
    wall_pos = ctx->cam_pos.x + ray_len * ray_dir.x;
  }
  wall_pos -= (int)wall_pos;
  if (wall_pos < 0) wall_pos += 1;

  result->distance = ray_len;
  result->wall_pos = wall_pos;
}


void render_scene(const RenderContext* ctx, RenderTexture* target) {
  BeginTextureMode(*target);
  ClearBackground(BLANK);
  for (float x = -0.5f; x <= 0.5f; x += 1 / ctx->render_size.x) {
    CastResult result = {0};
    cast_ray(ctx, x, &result);

    if (!result.hit) continue;

    int col_height = roundf(ctx->render_size.y / (result.distance * ctx->focal_len / ctx->render_dist));
    if (col_height < 0) col_height = INT_MAX;

    int col = roundf((x + 0.5f) * ctx->render_size.x);
    for (int y = 0; y < col_height; y++) {
      int pixel_y = (ctx->render_size.y - col_height) * 0.5 + y;
      if (pixel_y < 0 || pixel_y > ctx->render_size.y) continue;

      int atlas_x = ((float)result.atlas_idx - 1 + result.wall_pos) * ctx->atlas->height;
      int atlas_y = (float)y / col_height * ctx->atlas->height;
      Color pixel_color = ctx->atlasColors[atlas_y * ctx->atlas->width + atlas_x];
      pixel_color = ColorLerp(pixel_color, BLACK, result.distance / ctx->render_dist);

      DrawPixel(col, ctx->render_size.y - pixel_y, pixel_color);
    }
  }

  EndTextureMode();
}
