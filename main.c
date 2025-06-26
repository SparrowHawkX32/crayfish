#include <sys/stat.h>
#include <raylib.h>
#include <raymath.h>
#include <sys/types.h>

#define RAY_LEN 0.5f
#define RAY_STEP 0.01f
#define NUM_STEPS 50
#define FOV 90
#define ROT_SPEED 120
#define MOVE_SPEED 0.1f
#define FLOOR_Z 0.1f
#define CEIL_Z -0.1f

static Vector2 pos;
static float rot;
static Image map;
static int map_size;


int cmp_color(Color a, Color b) {
  return !(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
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



Color cast_ray (float angle_h, float angle_v) {
  float ah = rot + angle_h;
  float av = angle_v;
  Vector3 dir = Vector3RotateByAxisAngle((Vector3){1.0f, 0.0f, 0.0f}, (Vector3){0.0f, -1.0f, 0.0f}, av);
  dir = Vector3RotateByAxisAngle(dir, (Vector3){0.0f, 0.0f, 1.0f}, ah);
  Vector3 ray_pos;
  for (float f = 0.0f; f < RAY_LEN; f += RAY_STEP) {
    ray_pos = Vector3Add((Vector3){pos.x, pos.y, 0}, Vector3Scale(dir, f));

    Color result;
    if (ray_pos.z > FLOOR_Z || ray_pos.z < CEIL_Z) result = BROWN;
    else result = get_map_color((Vector2){ray_pos.x, ray_pos.y});

    if (cmp_color(result, BLACK))  {
      return fade_color(result, f / RAY_LEN);
    }
  }
  return BLACK;
}



int main (void) {
  InitWindow(800, 600, "Raycasting");

  // Init variables
  map = LoadImage("maps/map.png");
  map_size = map.width;

  Image out = GenImageColor(160, 120, BLACK);
  Texture2D bg_tex = LoadTextureFromImage(out);

  pos = (Vector2){0.05f, 0.05f};
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
    if (!cmp_color(get_map_color(new_pos), BLACK)) {
      pos = new_pos;
      if (pos.x < 0) pos.x += 1;
      else if (pos.x > 1) pos.x -= 1;
      if (pos.y < 0) pos.y += 1;
      else if (pos.y > 1) pos.y -= 1;
    }
    
    for (int y = 0; y < 120; y++){
      for (int x = 0; x < 160; x++) {
        float ray_angle_h = ((float)x / 160 - 0.5) * FOV * DEG2RAD;
        float ray_angle_v = ((float)y / 120 - 0.5) * ((float)120 / 160 * FOV) * DEG2RAD;
        Color col = cast_ray(ray_angle_h, ray_angle_v);
        ImageDrawPixel(&out, x, y, col);
      }
    }

    UnloadTexture(bg_tex);
    bg_tex = LoadTextureFromImage(out);

    BeginDrawing();

    DrawTextureEx(bg_tex, (Vector2){0, 0}, 0, 5, WHITE);

    EndDrawing();
  }
  
  // Unload resources
  UnloadImage(map);
  UnloadTexture(bg_tex);

  CloseWindow();
}
