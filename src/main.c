#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "level.h"
#include "render.h"

#define ROT_SPEED 120
#define MOVE_SPEED 1.0f


int main (void) {
  InitWindow(800, 600, "Raycasting");
  SetWindowState(FLAG_WINDOW_RESIZABLE);

  Level test_level = {
    .width = 5,
    .height = 5,
    .atlas_name = "",
    .name = "test_level",
    .ceil_color = BLACK,
    .floor_color = BLACK,
    .spawn_pos = (Vector2){2.5f, 2.5f},
    .spawn_rot = 0
  };
  unsigned char* test_map = malloc(test_level.width * test_level.height);
  for (int y = 0; y < test_level.height; y++) {
    for (int x = 0; x < test_level.width; x++) {
      test_map[y * test_level.width + x] = 0;
      if (x == 0 || x == test_level.width - 1  || y == 0 || y == test_level.height - 1) {
        SetRandomSeed(x * y);
        int wall_idx = GetRandomValue(1, 5) == 1 ? 2 : 1;
        test_map[y * test_level.width + x] = wall_idx;
      }
    }
  }
  test_level.map = test_map;

  RenderContext ctx = {
    .level = &test_level,
    .render_dist = DEFAULT_RENDER_DIST,
    .focal_len = DEFAULT_FOCAL_LEN,
    .cam_forward = Vector2Rotate((Vector2){1.0f, 0.0f}, test_level.spawn_rot),
    .cam_right = Vector2Rotate((Vector2){0.0f, 1.0f}, test_level.spawn_rot),
    .cam_pos = test_level.spawn_pos,
    .render_size = (Vector2){DEFAULT_RENDER_X, DEFAULT_RENDER_Y}
  };


  // Init variables

  Image atlas = LoadImage("../textures/leaf-sheet.png");
  ctx.atlas = &atlas;
  ctx.atlasColors = LoadImageColors(atlas);

  RenderTexture target = LoadRenderTexture(ctx.render_size.x, ctx.render_size.y);

  Image bg = GenImageColor(ctx.render_size.x, ctx.render_size.y, BLACK);
  // Generate background depth effect
  for (int i = 0; i < ctx.render_size.y; i++) {
    Color row_color = ColorLerp(BROWN, BLACK, 1 - fabsf(ctx.render_size.y * 0.5f - i) / ctx.render_size.y * 2);
    ImageDrawLine(&bg, 0, i, ctx.render_size.x, i, row_color);
  }
  Texture bg_tex = LoadTextureFromImage(bg);

  // Main loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();

    // Process input
    float rot = 0.0f;
    rot = (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * DEG2RAD * ROT_SPEED * GetFrameTime();
    rot = fmodf(rot, 360);
    Vector2 vel = {0};
    vel = Vector2Add(vel, Vector2Scale(ctx.cam_forward, (float)(IsKeyDown(KEY_W) - IsKeyDown(KEY_S))));
    vel = Vector2Add(vel, Vector2Scale(ctx.cam_right, (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A))));
    bool sprinting = IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT);
    vel = Vector2Scale(Vector2Normalize(vel), MOVE_SPEED * (1.0f + sprinting) * delta);

    // Apply velocity
    Vector2 new_pos = Vector2Add(ctx.cam_pos, vel);
    new_pos.x = Wrap(new_pos.x, 0, ctx.level->width);
    new_pos.y = Wrap(new_pos.y, 0, ctx.level->height);
    if (ctx.level->map[(int)ctx.cam_pos.y * ctx.level->width + (int)new_pos.x] != 0) {
      new_pos.x = ctx.cam_pos.x;
    }
    if (ctx.level->map[(int)new_pos.y * ctx.level->width + (int)ctx.cam_pos.x] != 0) {
      new_pos.y = ctx.cam_pos.y;
    }
    ctx.cam_pos = new_pos;
    

    ctx.cam_forward = Vector2Rotate(ctx.cam_forward, rot);
    ctx.cam_right = Vector2Rotate(ctx.cam_right, rot);

    render_scene(&ctx, &target);

    BeginDrawing();

    DrawTextureEx(bg_tex, (Vector2){0}, 0, (float)GetScreenWidth() / ctx.render_size.x, WHITE);
    DrawTextureEx(target.texture, (Vector2){0}, 0, (float)GetScreenWidth() / ctx.render_size.x, WHITE);

    char pos_text[32] = "";
    char fps_text[32] = "";

    sprintf(pos_text, "X: %.2f, Y: %.2f", ctx.cam_pos.x, ctx.cam_pos.y);
    sprintf(fps_text, "FPS: %.0f", 1 / GetFrameTime());

    DrawText(pos_text, 0, 0, 24, BLACK);
    DrawText(fps_text, 0, 28, 24, BLACK);

    EndDrawing();
  }
  
  // Unload resources
  UnloadRenderTexture(target);
  UnloadTexture(bg_tex);
  UnloadImage(bg);
  UnloadImageColors(ctx.atlasColors);
  UnloadImage(atlas);

  unload_level(&test_level);

  CloseWindow();
}
