#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define MAX_RAY_LEN 2.0f
#define FOV 90
#define ROT_SPEED 120
#define MOVE_SPEED 1.0f
#define RENDER_X 80
#define RENDER_Y 60


float wrap_geq(float f, float min, float max) {
  while (true) {
    if (f < min) f += max;
    else if (f >= max) f -= max;
    else break;
  }
  return f;
}


int main (void) {
  InitWindow(800, 600, "Raycasting");

  // Init variables
  Image map = LoadImage("maps/map.png");
  Image bg = GenImageColor(RENDER_X, RENDER_Y, BLACK);
  RenderTexture target = LoadRenderTexture(RENDER_X, RENDER_Y);

  Vector2 pos = (Vector2){1.5f, 0.5f};
  //Vector2 pos = (Vector2){0.0f, 10.0f};
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

    BeginTextureMode(target);

    DrawTexture(bg_tex, 0, 0, WHITE);
    
    for (int x = 0; x < RENDER_X; x++) {
      float ray_angle = ((float)x / RENDER_X - 0.5) * FOV * DEG2RAD + rot;
      Vector2 ray_dir = Vector2Rotate((Vector2){1, 0}, ray_angle);
      bool x_first = fabsf(ray_dir.x) > fabsf(ray_dir.y);
      float slope = fabsf(ray_dir.x / ray_dir.y);
      int step_x;
      int step_y;
      int grid_x = (int)pos.x;
      int grid_y = (int)pos.y;
      float hyp_dist_x = fabsf(1 / ray_dir.x);
      float hyp_dist_y = fabsf(1 / ray_dir.y);
      float dist_x;
      float dist_y;
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
  
      while (true) {
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
        if (!ColorIsEqual(col_color, BLACK) || fminf(dist_x, dist_y) >= MAX_RAY_LEN) break; // ERROR WITH LENGTH CHECK
      }
      
      float ray_len;
      if (hit_side == 0) {
        ray_len = dist_x - hyp_dist_x;
      }
      else {
        ray_len = dist_y - hyp_dist_y;
      }

      int col_height = RENDER_Y * (1 - ray_len / MAX_RAY_LEN);
      int col_start = (RENDER_Y - col_height) * 0.5;
      col_color = ColorLerp(col_color, BLACK, ray_len / MAX_RAY_LEN);

      DrawLine(x, col_start, x, col_start + col_height, col_color);
    }

    EndTextureMode();

    BeginDrawing();
    DrawTextureEx(target.texture, (Vector2){0}, 0, (float)GetScreenWidth() / RENDER_X, WHITE);
    EndDrawing();
  }
  
  // Unload resources
  UnloadRenderTexture(target);
  UnloadTexture(bg_tex);
  UnloadImage(bg);
  UnloadImage(map);

  CloseWindow();
}
