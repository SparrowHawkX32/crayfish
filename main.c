#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define RAY_LEN 2.0f
#define STEP_LEN 0.1f
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

    // Apply velcoity
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
      Vector2 ray_step = Vector2Rotate((Vector2){STEP_LEN, 0}, ray_angle);
      Vector2 ray_pos = pos;

      for (float f = 0.0f; f < RAY_LEN; f += STEP_LEN) {
        ray_pos = Vector2Add(ray_pos, ray_step);

        float cell_x = wrap_geq(ray_pos.x, 0, map.width);
        float cell_y = wrap_geq(ray_pos.y, 0, map.height);
        Color col_color = GetImageColor(map, floorf(cell_x), floorf(cell_y));

        if (!ColorIsEqual(col_color, BLACK))  {
          int col_height = RENDER_Y * (1 - f / RAY_LEN);
          int col_start = (RENDER_Y - col_height) * 0.5;
          col_color = ColorLerp(col_color, BLACK, f / RAY_LEN);

          DrawLine(x, col_start, x, col_start + col_height, col_color);
          break;
        }
      }
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
