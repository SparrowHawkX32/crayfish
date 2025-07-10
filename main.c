#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define RAY_LEN 0.2f
#define STEP_LEN 0.01f
#define FOV 90
#define ROT_SPEED 120
#define MOVE_SPEED 0.1f
#define RENDER_X 80
#define RENDER_Y 60

static Vector2 pos;
static float rot;
static Image map;
static int map_size;


Color get_map_color(Vector2 p) {
  if (p.x < 0) p.x += 1;
  else if (p.x > 1) p.x -= 1;
  if (p.y < 0) p.y += 1;
  else if (p.y > 1) p.y -= 1;
  p = Vector2Scale(p, map_size);
  int x = (int)p.x >= map_size ? (int)p.x - 1 : (int)p.x;
  int y = (int)p.y >= map_size ? (int)p.y - 1 : (int)p.y;
  return GetImageColor(map, x, y);
}


int main (void) {
  InitWindow(800, 600, "Raycasting");

  // Init variables
  map = LoadImage("maps/map.png");
  map_size = map.width;
  Image bg = GenImageColor(RENDER_X, RENDER_Y, BLACK);
  RenderTexture target = LoadRenderTexture(RENDER_X, RENDER_Y);

  pos = (Vector2){0.15f, 0.05f};
  rot = 0.0f;

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

    Vector2 new_pos = Vector2Add(pos, vel);
    if (ColorIsEqual(get_map_color(new_pos), BLACK)) {
      pos = new_pos;
      if (pos.x < 0) pos.x += 1;
      else if (pos.x > 1) pos.x -= 1;
      if (pos.y < 0) pos.y += 1;
      else if (pos.y > 1) pos.y -= 1;
    }

    BeginTextureMode(target);

    DrawTexture(bg_tex, 0, 0, WHITE);
    
    for (int x = 0; x < RENDER_X; x++) {
      float ray_angle = ((float)x / RENDER_X - 0.5) * FOV * DEG2RAD + rot;
      Vector2 ray_step = Vector2Rotate((Vector2){STEP_LEN, 0}, ray_angle);
      Vector2 ray_pos = pos;

      for (float f = 0.0f; f < RAY_LEN; f += STEP_LEN) {
        ray_pos = Vector2Add(ray_pos, ray_step);

        Color col_color = get_map_color((Vector2){ray_pos.x, ray_pos.y});

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
