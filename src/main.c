#include <stdbool.h>
#include <raylib.h>
#include <raymath.h>
#include "render.h"
#include "ui.h"


int main (void) {
  InitWindow(800, 600, "Raycasting");
  SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);

  // Init variables
  RenderContext ctx = {0};
  init_render_ctx(&ctx);
  UIState ui_state = {0};
  init_ui(&ui_state);

  //TODO: move to render code
  ctx.atlas = LoadImage("../textures/leaf-sheet.png");;
  ctx.atlasColors = LoadImageColors(ctx.atlas);

  // Main loop
  while (!WindowShouldClose()) {
    process_input(&ctx);
    render_scene(&ctx);

    BeginDrawing();
    ClearBackground(BLACK);

    draw_scene(&ctx);
    draw_ui(&ui_state, &ctx);

    EndDrawing();
  }
  
  // Unload resources
  UnloadImageColors(ctx.atlasColors);
  UnloadImage(ctx.atlas);

  unload_ui(&ui_state);
  unload_render_ctx(&ctx);

  CloseWindow();
}
