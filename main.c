#include <stdio.h>
#include <sys/stat.h>
#include <raylib.h>
#include <raymath.h>
#include <sys/types.h>

#define RAY_LEN 0.5f
#define STEP_LEN 0.01f
#define FOV 90
#define ROT_SPEED 120
#define MOVE_SPEED 0.1f

static Vector2 pos;
static float rot;
static Image map;
static int map_size;


int cmp_color(Color a, Color b) {
  return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}


Color fade_color(Color c, float f) {
  Color result = {0};
  result.r = (u_char)Lerp(c.r, 0, f);
  result.g = (u_char)Lerp(c.g, 0, f);
  result.b = (u_char)Lerp(c.b, 0, f);
  result.a = 255;
  return result;
}


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

  pos = (Vector2){0.15f, 0.05f};
  rot = 0.0f;

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
    if (cmp_color(get_map_color(new_pos), BLACK)) {
      pos = new_pos;
      if (pos.x < 0) pos.x += 1;
      else if (pos.x > 1) pos.x -= 1;
      if (pos.y < 0) pos.y += 1;
      else if (pos.y > 1) pos.y -= 1;
    }

    BeginDrawing();

    ClearBackground(BROWN);
    
    for (int x = 0; x < GetScreenWidth(); x++) {
      float ray_angle = ((float)x / GetScreenWidth() - 0.5) * FOV * DEG2RAD + rot;
      Vector2 ray_step = Vector2Rotate((Vector2){STEP_LEN, 0}, ray_angle);
      Vector2 ray_pos = pos;

      for (float f = 0.0f; f < RAY_LEN; f += STEP_LEN) {
        ray_pos = Vector2Add(ray_pos, ray_step);

        Color col_color = get_map_color((Vector2){ray_pos.x, ray_pos.y});

        if (!cmp_color(col_color, BLACK))  {
          int col_height = GetScreenHeight() * (1 - f / RAY_LEN);
          int col_start = (GetScreenHeight() - col_height) * 0.5;
          col_color = fade_color(col_color, f / RAY_LEN);

          DrawLine(x, col_start, x, col_start + col_height, col_color);
          break;
        }
      }
    }

    EndDrawing();
  }
  
  // Unload resources
  UnloadImage(map);

  CloseWindow();
}
