#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include "util.h"

#define RENDER_DIST 5.0f
#define FOCAL_LEN 5.0f
#define ROT_SPEED 120
#define MOVE_SPEED 1.0f
#define RENDER_X 80
#define RENDER_Y 60


int main (void) {
  InitWindow(800, 600, "Raycasting");
  SetWindowState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);

  // Init variables
  Image map = LoadImage("../maps/map.png");
  Image bg = GenImageColor(RENDER_X, RENDER_Y, BLACK);
  float pixel_fraction = 1.0f / RENDER_X;

  Texture wall_tex = LoadTexture("../textures/wall.png");
  RenderTexture target = LoadRenderTexture(RENDER_X, RENDER_Y);
  Shader texture_shader = LoadShader(NULL, "../src/texturer.frag");

  Vector2 pos = (Vector2){1.5f, 0.5f};
  float rot = 0.0f;

  // Generate background depth effect
  for (int i = 0; i < RENDER_Y; i++) {
    Color row_color = ColorLerp(BROWN, BLACK, 1 - fabsf(RENDER_Y * 0.5f - i) / RENDER_Y * 2);
    ImageDrawLine(&bg, 0, i, RENDER_X, i, row_color);
  }
  Texture bg_tex = LoadTextureFromImage(bg);

  // Main loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();

    // Process input
    rot += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * DEG2RAD * ROT_SPEED * GetFrameTime();
    rot = fmodf(rot, 360);
    Vector2 vel = {0};
    vel.x = (float)(IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    vel.y = (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    bool sprinting = IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT);
    vel = Vector2Rotate(Vector2Scale(Vector2Normalize(vel), MOVE_SPEED * (1.0f + sprinting) * delta), rot);

    // Apply velocity
    Vector2 new_pos = Vector2Add(pos, vel);
    new_pos.x = wrap_geq(new_pos.x, 0, map.width);
    new_pos.y = wrap_geq(new_pos.y, 0, map.height);
    if (ColorIsEqual(GetImageColor(map, floorf(new_pos.x), floorf(new_pos.y)), BLACK)) {
      pos = new_pos;
    }

    Vector2 forward = Vector2Rotate((Vector2){1, 0}, rot);
    Vector2 right = Vector2Rotate((Vector2){0, 1}, rot);

    BeginTextureMode(target);
    BeginBlendMode(BLEND_ADD_COLORS);
    ClearBackground(BLANK);
    for (int x = -RENDER_X * 0.5f; x <= RENDER_X * 0.5f; x++) {
      Vector2 ray_dir = Vector2Add(forward, Vector2Scale(right, pixel_fraction * x));
      int step_x;
      int step_y;
      int grid_x = (int)pos.x;
      int grid_y = (int)pos.y;
      float hyp_dist_x = fabsf(1 / ray_dir.x);
      float hyp_dist_y = fabsf(1 / ray_dir.y);
      float dist_x;
      float dist_y;
      float prev_dist_x = 0;
      float prev_dist_y = 0;
      bool hit = false;
      int hit_side = 0;
      Color col_color = BLACK;

      if (ray_dir.x >= 0 ) {
        step_x = 1;
        dist_x = (grid_x + 1.0f - pos.x) * hyp_dist_x;
      }
      else {
        step_x = -1;
        dist_x = (pos.x - grid_x) * hyp_dist_x;
      }
      if (ray_dir.y >= 0 ) {
        step_y = 1;
        dist_y = (grid_y + 1.0f - pos.y) * hyp_dist_y;
      }
      else {
        step_y = -1;
        dist_y = (pos.y - grid_y) * hyp_dist_y;
      }
  
      while (hit == false) {
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

        col_color = GetImageColor(map, wrap_geq(grid_x, 0, map.width), wrap_geq(grid_y, 0, map.height));
        if ((hit_side == 0 && prev_dist_x >= RENDER_DIST) || 
            (hit_side == 1 && prev_dist_y >= RENDER_DIST)) break;
        if (!ColorIsEqual(col_color, BLACK)) hit = true;

        prev_dist_x = dist_x;
        prev_dist_y = dist_y;
      }

      if (!hit) continue; // ignore rays out of range
      
      float ray_len;
      float wall_pos;
      if (hit_side == 0) {
        ray_len = dist_x - hyp_dist_x;
        wall_pos = pos.y + ray_len * ray_dir.y;
      }
      else {
        ray_len = dist_y - hyp_dist_y;
        wall_pos = pos.x + ray_len * ray_dir.x;
      }
      wall_pos -= (int)wall_pos;

      int col_height = roundf(RENDER_Y / (ray_len * FOCAL_LEN / RENDER_DIST));
      if (col_height > RENDER_Y * 3) col_height = RENDER_Y * 3; // prevent issues with column height approaching infinity

      int col = x + 0.5f * RENDER_X;
      for (int y = 0; y < col_height; y++) {
        int pixel_y = (RENDER_Y - col_height) * 0.5 + y;
        if (pixel_y < 0 || pixel_y > RENDER_Y) continue;
        DrawPixel(col, RENDER_Y - pixel_y, (Color){
            wall_pos * 255,                   // pos along tile wall
            (float)y / col_height * 255,      // pos along column
            col_color.r,                      // texture index
            (1 - ray_len / RENDER_DIST) * 255 // brightness
            });
      }
    }

    EndBlendMode();
    EndTextureMode();

    SetShaderValueTexture(texture_shader, GetShaderLocation(texture_shader, "texture1"), wall_tex);

    BeginDrawing();

    DrawTextureEx(bg_tex, (Vector2){0}, 0, (float)GetScreenWidth() / RENDER_X, WHITE);

    BeginShaderMode(texture_shader);
    DrawTextureEx(target.texture, (Vector2){0}, 0, (float)GetScreenWidth() / RENDER_X, WHITE);
    EndShaderMode();

    /*
    char fps[5] = {0};
    sprintf(fps, "%d", (int)(1 / GetFrameTime())); // THIS IS VERY BAD, CAUSING OVERFLOW PROBABLY
    DrawText(fps, 0, 0, 24, YELLOW);
    */

    EndDrawing();
  }
  
  // Unload resources
  UnloadRenderTexture(target);
  UnloadTexture(bg_tex);
  UnloadImage(bg);
  UnloadImage(map);
  UnloadShader(texture_shader);

  CloseWindow();
}
