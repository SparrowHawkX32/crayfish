#include "render.h"
#include "level.h"
#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>


void init_render_ctx(RenderContext* ctx) {
  ctx->render_dist = DEFAULT_RENDER_DIST;
  ctx->fov = DEFAULT_FOV;
  ctx->render_width = DEFAULT_RENDER_X;
  ctx->render_height = DEFAULT_RENDER_Y;
  ctx->target = LoadRenderTexture(ctx->render_width, ctx->render_height);
  ctx->bg = LoadRenderTexture(ctx->render_width, ctx->render_height);
  load_placeholder_atlas(ctx);
}


void unload_render_ctx(RenderContext* ctx) {
  unload_level(ctx->level);
  UnloadRenderTexture(ctx->target);
  UnloadRenderTexture(ctx->bg);
}


void generate_bg(RenderContext *ctx) {
  BeginTextureMode(ctx->bg);
  ClearBackground(BLANK);
  for (int y = 1; y <= ctx->render_height; y++) {
    Color outer_color;
    float blend_fac = 1 - fabsf(ctx->render_height * 0.5f - y) / ctx->render_height * 2;

    if (y < ctx->render_height * 0.5f) {
      outer_color = ctx->level->floor_color;
    }
    else {
      outer_color = ctx->level->ceil_color;
    }

    Color row_color = ColorLerp(outer_color, ctx->level->air_color, blend_fac);
    DrawLine(0, y, ctx->render_width, y, row_color);
  }
  EndTextureMode();
}


void change_level(RenderContext *ctx, Level *level) {
  if (ctx->level) unload_level(ctx->level);
  ctx->level = level;

  ctx->cam_forward = Vector2Rotate((Vector2){1.0f, 0.0f}, level->spawn_rot * DEG2RAD),
  ctx->cam_right = Vector2Rotate((Vector2){0.0f, 1.0f}, level->spawn_rot * DEG2RAD),
  ctx->cam_pos = level->spawn_pos,

  generate_bg(ctx);
}


void change_atlas(RenderContext* ctx, const char* path) {
      Image new_atlas = LoadImage(path);

      if (new_atlas.data != NULL) {
      printf("Changing to atlas at %s\n", path);
        UnloadImageColors(ctx->atlas_colors);
        UnloadImage(ctx->atlas);
        ctx->atlas = new_atlas;
        ctx->atlas_colors = LoadImageColors(new_atlas);
      }
      else UnloadImage(new_atlas);
}


void load_placeholder_atlas(RenderContext* ctx) {
  if (ctx->atlas.data) {
    UnloadImage(ctx->atlas);
    UnloadImageColors(ctx->atlas_colors);
  }
  ctx->atlas = GenImageColor(16 * 254, 16, RAYWHITE);
  char num[4] = "";
  for (int i = 0; i < 254; i++) {
    ImageDrawRectangle(&ctx->atlas, 16 * i, 0, 8, 8, GRAY);
    ImageDrawRectangle(&ctx->atlas, 16 * i + 8, 8, 8, 8, GRAY);
    sprintf((char*)&num, "%d", i + 1);
    ImageDrawText(&ctx->atlas, (const char*)&num, 16 * i, 0, 10, BLACK);
  }
  ctx->atlas_colors = LoadImageColors(ctx->atlas);
}


void process_input(RenderContext* ctx) {
  if (ctx->level == NULL) return;

  // Process input
  float rot = 0.0f;
  rot = (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * DEG2RAD * ROT_SPEED * GetFrameTime();
  rot = fmodf(rot, 360);
  Vector2 vel = {0};
  vel = Vector2Add(vel, Vector2Scale(ctx->cam_forward, (float)(IsKeyDown(KEY_W) - IsKeyDown(KEY_S))));
  vel = Vector2Add(vel, Vector2Scale(ctx->cam_right, (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A))));
  bool sprinting = IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT);
  vel = Vector2Scale(Vector2Normalize(vel), MOVE_SPEED * (1.0f + sprinting) * GetFrameTime());

  // Apply velocity
  Vector2 new_pos = Vector2Add(ctx->cam_pos, vel);
  new_pos.x = Wrap(new_pos.x, 0, ctx->level->width);
  new_pos.y = Wrap(new_pos.y, 0, ctx->level->height);
  if (ctx->level->map[(int)ctx->cam_pos.y * ctx->level->width + (int)new_pos.x] != 0) {
    new_pos.x = ctx->cam_pos.x;
  }
  if (ctx->level->map[(int)new_pos.y * ctx->level->width + (int)ctx->cam_pos.x] != 0) {
    new_pos.y = ctx->cam_pos.y;
  }
  ctx->cam_pos = new_pos;
  
  ctx->cam_forward = Vector2Rotate(ctx->cam_forward, rot);
  ctx->cam_right = Vector2Rotate(ctx->cam_right, rot);
}


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
    size_t map_offset = Wrap(grid_y, 0, ctx->level->height) * ctx->level->width + Wrap(grid_x, 0, ctx->level->width);
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


void render_scene(const RenderContext* ctx) {
  if (!ctx->level) return;

  BeginTextureMode(ctx->target);
  ClearBackground(BLANK);
  for (float x = -0.5f; x <= 0.5f; x += 1.0f / ctx->render_width) {
    CastResult result = {0};
    cast_ray(ctx, x * ctx->fov, &result);

    if (!result.hit) continue;

    int col_height = roundf(ctx->render_height / result.distance);
    if (col_height < 0) col_height = INT_MAX;

    int render_height = col_height < ctx->render_height ? col_height : ctx->render_height;

    int col = roundf((x + 0.5f) * ctx->render_width);
    for (int y = 0; y < render_height; y++) {
      int pixel_y;
      int atlas_y;
      if (col_height > render_height) {
        pixel_y = y;
        atlas_y = (float)(y + (col_height - ctx->render_height) * 0.5f) / col_height * ctx->atlas.height;
      }
      else {
        pixel_y = (float)(ctx->render_height - col_height) * 0.5f + y;
        atlas_y = (float)y / col_height * ctx->atlas.height;
      }

      int num_textures = ctx->atlas.width / ctx->atlas.height;
      if (result.atlas_idx > num_textures) result.atlas_idx = 1;

      int atlas_x = ((float)result.atlas_idx - 1 + result.wall_pos) * ctx->atlas.height;
      Color pixel_color = ctx->atlas_colors[atlas_y * ctx->atlas.width + atlas_x];
      pixel_color = ColorLerp(pixel_color, ctx->level->air_color, result.distance / ctx->render_dist);

      DrawPixel(col, ctx->render_height - 1 - pixel_y, pixel_color);
    }
  }
  EndTextureMode();
}


void draw_scene(const RenderContext *ctx) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  float scale;

  Vector2 offset = {0};

  float render_ratio = (float)ctx->render_width / ctx->render_height;
  float screen_ratio = (float)width / height;
  float abs_render_ratio = render_ratio < 1 ? 1 / render_ratio : render_ratio;
  float abs_screen_ratio = screen_ratio < 1 ? 1 / screen_ratio : screen_ratio;

  if (render_ratio == screen_ratio) {
    scale = (float)width / ctx->render_width;
  }
  else if (render_ratio != 1 && screen_ratio != 1 && render_ratio < 1 == screen_ratio < 1 && abs_render_ratio > abs_screen_ratio) {
    scale = fmaxf(width, height) / fmaxf(ctx->render_width, ctx->render_height);
    if (screen_ratio > 1) {
      offset.y = (height - scale * ctx->render_height) * 0.5f;
    }
    else {
      offset.x = (width - scale * ctx->render_width) * 0.5f;
    }
  }
  else if (render_ratio != 1 && screen_ratio != 1 && render_ratio < 1 == screen_ratio < 1) {
    scale = fminf(width, height) / fminf(ctx->render_width, ctx->render_height);
    if (screen_ratio > 1) {
      offset.x = (width - scale * ctx->render_width) * 0.5f;
    }
    else {
      offset.y = (height - scale * ctx->render_height) * 0.5f;
    }
  }
  else {
    scale = fminf(width, height) / fmaxf(ctx->render_width, ctx->render_height);
    if (screen_ratio > 1) {
      offset.x = (width - scale * ctx->render_width) * 0.5f;
    }
    else {
      offset.y = (height - scale * ctx->render_height) * 0.5f;
    }
  }

  DrawTextureEx(ctx->bg.texture, offset, 0, scale, WHITE);
  DrawTextureEx(ctx->target.texture, offset, 0, scale, WHITE);

  Vector2 size = Vector2Scale((Vector2){ctx->render_width, ctx->render_height}, scale);
}
