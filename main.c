#include <stdio.h>
#include <sys/stat.h>
#include <raylib.h>
#include <raymath.h>
#include <sys/types.h>

#define GLSL_VERSION 330

#define RAY_STEP 0.01f
#define NUM_STEPS 50
#define FOV 75
#define ROT_SPEED 120
#define MOVE_SPEED 0.1f
#define FLOOR_Z 0.1f
#define CEIL_Z -0.1f

static Image map;
static int map_size;


typedef struct {
  float r;
  float g;
  float b;
  float a;
} GLColor;


GLColor convert_color(Color in) {
  GLColor col = {0};
  col.r = (float)in.r / 255;
  col.g = (float)in.g / 255;
  col.b = (float)in.b / 255;
  col.a = (float)in.a / 255;
  return col;
}


int cmp_color(Color a, Color b) {
  return !(a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
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

  Image blank = GenImageColor(GetScreenWidth(), GetScreenHeight(), BLACK);

  Texture2D bg_tex = LoadTextureFromImage(blank);
  Texture2D map_tex = LoadTextureFromImage(map);

  Shader ray_shader = LoadShader(0, "raycasting.frag");

  // Load initial shader values
  float fov = FOV * DEG2RAD;
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "fov"), &fov, SHADER_UNIFORM_FLOAT);
  float height_ratio = (float)GetScreenHeight() / GetScreenWidth();
  printf("HEIGHT RATIO: %f", height_ratio);
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "heightRatio"), &height_ratio, SHADER_UNIFORM_FLOAT);
  float step_size = RAY_STEP;
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "stepSize"), &step_size, SHADER_UNIFORM_FLOAT);
  int num_steps = NUM_STEPS;
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "numSteps"), &num_steps, SHADER_UNIFORM_INT);
  Vector2 z_bounds = (Vector2){-0.1f, 0.1f};
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "zBounds"), &z_bounds, SHADER_UNIFORM_VEC2);
  GLColor floor_ceil_color = convert_color(BROWN);
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "floorCeilColor"), &floor_ceil_color, SHADER_UNIFORM_VEC4);
  GLColor fog_color = convert_color(BLACK);
  SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "fogColor"), &fog_color, SHADER_UNIFORM_VEC4);


  Vector2 pos = (Vector2){0.05f, 0.05f};
  float rot = 0.0f;

  // Main loop
  while (!WindowShouldClose()) {
    float delta = GetFrameTime();

    // Process input
    rot += (float)(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)) * ROT_SPEED * GetFrameTime() * DEG2RAD;
    Vector2 vel = {0};
    vel.x = (float)(IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    vel.y = (float)(IsKeyDown(KEY_D) - IsKeyDown(KEY_A));

    bool sprinting = IsKeyDown(KEY_W) && IsKeyDown(KEY_LEFT_SHIFT);
    vel = Vector2Rotate(Vector2Scale(Vector2Normalize(vel), MOVE_SPEED * (1.0f + sprinting) * delta), rot);
    Vector2 new_pos = Vector2Add(pos, vel);

    // Check collisions
    if (!cmp_color(get_map_color(new_pos), BLACK)) {
      pos = new_pos;
      if (pos.x < 0) pos.x += 1;
      else if (pos.x > 1) pos.x -= 1;
      if (pos.y < 0) pos.y += 1;
      else if (pos.y > 1) pos.y -= 1;
    }

    // Draw phase
    BeginDrawing();
    BeginShaderMode(ray_shader);
    
    SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "pos"), &pos, SHADER_UNIFORM_VEC2);
    SetShaderValue(ray_shader, GetShaderLocation(ray_shader, "rot"), &rot, SHADER_UNIFORM_FLOAT);
    SetShaderValueTexture(ray_shader, GetShaderLocation(ray_shader, "mapTex"), map_tex);

    DrawTextureEx(bg_tex, (Vector2){0, 0}, 0, 1, BLACK);

    EndShaderMode();
    EndDrawing();
  }
  
  // Unload resources
  UnloadShader(ray_shader);
  UnloadImage(blank);
  UnloadImage(map);
  UnloadTexture(bg_tex);
  UnloadTexture(map_tex);

  CloseWindow();
}
